/*
 * Copyright © 2010 Anthony Towns <aj@erisian.com.au>
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

public class AltosMapTile implements AltosFontListener, AltosMapStoreListener {
	LinkedList<AltosMapTileListener>	listeners = new LinkedList<AltosMapTileListener>();
	public AltosLatLon	upper_left, center;
	public int		px_size;
	int		zoom;
	int		maptype;
	int		scale;
	private AltosMapCache	cache;
	public AltosMapStore	store;
	public int	status;

	static public final int	loaded = 0;	/* loaded from file */
	static public final int	fetched = 1;	/* downloaded to file */
	static public final int	fetching = 2;	/* downloading from net */
	static public final int	failed = 3;	/* loading from file failed */
	static public final int	bad_request = 4;/* downloading failed */
	static public final int	forbidden = 5;	/* downloading failed */

	static public String status_name(int status) {
		switch (status) {
		case loaded:
			return "loaded";
		case fetched:
			return "fetched";
		case fetching:
			return "fetching";
		case failed:
			return "failed";
		case bad_request:
			return "bad_request";
		case forbidden:
			return "forbidden";
		default:
			return "unknown";
		}
	}

	public void font_size_changed(int font_size) {
	}

	private synchronized void notify_listeners(int status) {
		this.status = status;
		for (AltosMapTileListener listener : listeners)
			listener.notify_tile(this, status);
	}

	public void notify_store(AltosMapStore store, int status) {
		notify_listeners(status);
	}

	public void notify_image(AltosImage image) {
		if (image == null)
			status = failed;
		else
			status = loaded;
		notify_listeners(status);
	}

	public void paint(AltosMapTransform t) {
	}

	public AltosImage get_image() {
		if (cache == null)
			return null;
		return cache.get(this);
	}

	public synchronized void add_listener(AltosMapTileListener listener) {
		if (!listeners.contains(listener))
			listeners.add(listener);
		listener.notify_tile(this, status);
	}

	public synchronized void remove_listener(AltosMapTileListener listener) {
		listeners.remove(listener);
	}

	public AltosMapTile(AltosMapCache cache, AltosLatLon upper_left, AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		this.cache = cache;
		this.upper_left = upper_left;

		while (center.lon < -180.0)
			center.lon += 360.0;
		while (center.lon > 180.0)
			center.lon -= 360.0;

		this.center = center;
		this.zoom = zoom;
		this.maptype = maptype;
		this.px_size = px_size;
		this.scale = scale;

		store = AltosMapStore.get(center, zoom, maptype, px_size, scale);
		store.add_listener(this);
	}
}
