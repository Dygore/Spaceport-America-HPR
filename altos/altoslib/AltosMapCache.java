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
import java.net.*;

public class AltosMapCache implements AltosMapCacheListener {

	/* An entry in the MapCache */
	class MapCacheElement implements AltosMapTileListener {

		AltosMapTile		tile;		/* Notify when image has been loaded */
		AltosImage		image;
		long			used;

		class loader implements Runnable {
			public void run() {
				if (image == null) {
					try {
						image = map_interface.load_image(tile.store.file);
					} catch (Exception ex) {
					}
				}
				tile.notify_image(image);
			}
		}

		private void load() {
			loader	l = new loader();
			Thread	lt = new Thread(l);
			lt.start();
		}

		public void flush() {
			if (image != null) {
				image.flush();
				image = null;
			}
		}

		public boolean has_map() {
			return tile.status == AltosMapTile.loaded;
		}

		public synchronized void notify_tile(AltosMapTile tile, int status) {
			if (status == AltosMapTile.fetched) {
				load();
			}
		}

		public MapCacheElement(AltosMapTile tile) {
			this.tile = tile;
			this.image = null;
			this.used = 0;
			tile.add_listener(this);
		}
	}

	int			min_cache_size;		/* configured minimum cache size */
	int			cache_size;		/* current cache size */
	int			requested_cache_size;	/* cache size computed by application */

	private Object 		fetch_lock = new Object();
	private Object 		cache_lock = new Object();

	AltosMapInterface	map_interface;

	MapCacheElement[]	elements = new MapCacheElement[cache_size];

	long			used;

	public void set_cache_size(int new_size) {

		requested_cache_size = new_size;

		if (new_size < min_cache_size)
			new_size = min_cache_size;

		if (new_size == cache_size)
			return;

		synchronized(cache_lock) {
			MapCacheElement[]	new_elements = new MapCacheElement[new_size];

			for (int i = 0; i < cache_size; i++) {
				if (i < new_size)
					new_elements[i] = elements[i];
				else if (elements[i] != null)
					elements[i].flush();
			}
			elements = new_elements;
			cache_size = new_size;
		}
	}

	public AltosImage get(AltosMapTile tile) {
		int		oldest = -1;
		long		age = used;

		synchronized(cache_lock) {
			MapCacheElement	element = null;
			for (int i = 0; i < cache_size; i++) {
				element = elements[i];

				if (element == null) {
					oldest = i;
					break;
				}
				if (tile.store.equals(element.tile.store)) {
					element.used = used++;
					return element.image;
				}
				if (element.used < age) {
					oldest = i;
					age = element.used;
				}
			}

			element = new MapCacheElement(tile);
			element.used = used++;
			if (elements[oldest] != null)
				elements[oldest].flush();

			elements[oldest] = element;
			return element.image;
		}
	}

	public void map_cache_changed(int map_cache) {
		min_cache_size = map_cache;

		set_cache_size(requested_cache_size);
	}

	public void dispose() {
		AltosPreferences.unregister_map_cache_listener(this);

		for (int i = 0; i < cache_size; i++) {
			MapCacheElement element = elements[i];

			if (element != null)
			    element.flush();
		}
	}

	public AltosMapCache(AltosMapInterface map_interface) {
		this.map_interface = map_interface;
		min_cache_size = AltosPreferences.map_cache();

		set_cache_size(0);

		AltosPreferences.register_map_cache_listener(this);
	}
}
