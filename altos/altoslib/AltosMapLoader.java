/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import java.text.*;
import java.lang.Math;
import java.net.URL;
import java.net.URLConnection;

public class AltosMapLoader extends Thread implements AltosMapStoreListener {
	AltosMapLoaderListener	listener;

	double	latitude, longitude;
	int	min_z;
	int	max_z;
	int	cur_z;
	int	all_types;
	int	cur_type;
	double	radius;
	int	scale;

	int	tiles_loaded_layer;
	int	tiles_loaded_total;
	int	tiles_this_layer;
	int	tiles_total;
	int	layers_total;
	int	layers_loaded;

	private static final int	MAX_LOADING = 200;

	private Semaphore	loading = new Semaphore(MAX_LOADING);

	boolean	abort;

	int tile_radius(int zoom) {
		double	delta_lon = AltosMapTransform.lon_from_distance(latitude, radius);

		AltosMapTransform t = new AltosMapTransform(256, 256, zoom + AltosMap.default_zoom, new AltosLatLon(latitude, longitude));

		AltosPointDouble	center = t.point(new AltosLatLon(latitude, longitude));
		AltosPointDouble	edge = t.point(new AltosLatLon(latitude, longitude + delta_lon));

		int tile_radius = (int) Math.ceil(Math.abs(center.x - edge.x) / AltosMap.px_size);

		return tile_radius;
	}

	int tiles_per_layer(int zoom) {
		int	tile_radius = tile_radius(zoom);
		return (tile_radius * 2 + 1) * (tile_radius * 2 + 1);
	}

	private boolean do_load() {
		tiles_this_layer = tiles_per_layer(cur_z);
		tiles_loaded_layer = 0;
		listener.debug("tiles_this_layer %d (zoom %d)\n", tiles_this_layer, cur_z);

		int load_radius = tile_radius(cur_z);
		int zoom = cur_z + AltosMap.default_zoom;
		int maptype = cur_type;
		AltosLatLon load_centre = new AltosLatLon(latitude, longitude);
		AltosMapTransform transform = new AltosMapTransform(256, 256, zoom, load_centre);

		AltosPointInt	upper_left;
		AltosPointInt	lower_right;

		AltosPointInt centre = AltosMap.floor(transform.point(load_centre));

		upper_left = new AltosPointInt(centre.x - load_radius * AltosMap.px_size,
					       centre.y - load_radius * AltosMap.px_size);
		lower_right = new AltosPointInt(centre.x + load_radius * AltosMap.px_size,
						centre.y + load_radius * AltosMap.px_size);


		for (int y = (int) upper_left.y; y <= lower_right.y; y += AltosMap.px_size) {
			for (int x = (int) upper_left.x; x <= lower_right.x; x += AltosMap.px_size) {
				try {
					loading.acquire();
				} catch (InterruptedException ie) {
					return false;
				}
				AltosPointInt	point = new AltosPointInt(x, y);
				AltosLatLon	ul = transform.lat_lon(point);
				AltosLatLon	center = transform.lat_lon(new AltosPointDouble(x + AltosMap.px_size/2, y + AltosMap.px_size/2));
				AltosMapStore	store = AltosMapStore.get(center, zoom, maptype, AltosMap.px_size, scale);
				listener.debug("load state %s url %s\n", AltosMapTile.status_name(store.status()), store.url);
				store.add_listener(this);
				if (abort)
					return false;
			}
		}
		return true;
	}

	private int next_type(int start) {
		int next_type;
		for (next_type = start;
		     next_type <= AltosMap.maptype_terrain && (all_types & (1 << next_type)) == 0;
		     next_type++)
			;
		return next_type;
	}

	private boolean next_load() {
		int next_type = next_type(cur_type + 1);

		if (next_type > AltosMap.maptype_terrain) {
			if (cur_z == max_z) {
				return false;
			} else {
				cur_z++;
			}
			next_type = next_type(0);
		}
		cur_type = next_type;
		return true;
	}

	public void run() {

		cur_z = min_z;
		int ntype = 0;

		for (int t = AltosMap.maptype_hybrid; t <= AltosMap.maptype_terrain; t++)
			if ((all_types & (1 << t)) != 0)
				ntype++;
		if (ntype == 0) {
			all_types = (1 << AltosMap.maptype_hybrid);
			ntype = 1;
		}

		cur_type = next_type(0);

		tiles_total = 0;
		for (int z = min_z; z <= max_z; z++)
			tiles_total += tiles_per_layer(z) * ntype;

		layers_total = (max_z - min_z + 1) * ntype;
		layers_loaded = 0;
		tiles_loaded_total = 0;

		listener.debug("total tiles %d layers %d\n", tiles_total, layers_total);

		listener.loader_start(tiles_total);
		do {
			if (!do_load())
				break;
		} while (next_load());
		if (abort)
			listener.loader_done(tiles_total);
	}

	public synchronized void notify_store(AltosMapStore store, int status) {
		boolean	do_next = false;
		if (status == AltosMapTile.fetching)
			return;

		loading.release();

		store.remove_listener(this);

		if (layers_loaded >= layers_total)
			return;

		++tiles_loaded_total;
		++tiles_loaded_layer;

		listener.debug("AltosMapLoader.notify_store status %d total %d of %d layer %d of %d\n",
			       status, tiles_loaded_total, tiles_total, tiles_loaded_layer, tiles_this_layer);

		if (tiles_loaded_layer == tiles_this_layer) {
			++layers_loaded;
			listener.debug("%d layers loaded\n", layers_loaded);
			do_next = true;
		}

		if (tiles_loaded_total == tiles_total)
			listener.loader_done(tiles_total);
		else
			listener.loader_notify(tiles_loaded_total,
					       tiles_total, store.file.toString());
	}

	public void abort() {
		this.abort = true;
	}

	public AltosMapLoader(AltosMapLoaderListener listener,
			      double latitude, double longitude, int min_z, int max_z, double radius, int all_types, int scale) {
		listener.debug("lat %f lon %f min_z %d max_z %d radius %f all_types %d\n",
			       latitude, longitude, min_z, max_z, radius, all_types);
		this.listener = listener;
		this.latitude = latitude;
		this.longitude = longitude;
		this.min_z = min_z;
		this.max_z = max_z;
		this.radius = radius;
/*
		this.all_types = all_types;
*/
		this.all_types = 1 << AltosMap.maptype_hybrid;
		this.scale = scale;
		this.abort = false;
		start();
	}
}
