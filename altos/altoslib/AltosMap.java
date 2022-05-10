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
import java.lang.*;
import java.util.*;
import java.util.concurrent.*;

public class AltosMap implements AltosMapTileListener, AltosMapStoreListener {

	public static final int px_size = 512;

	public static final int maptype_hybrid = 0;
	public static final int maptype_roadmap = 1;
	public static final int maptype_satellite = 2;
	public static final int maptype_terrain = 3;
	public static final int maptype_default = maptype_hybrid;

	public static final int default_zoom = 15;
	public static final int min_zoom = 3;
	public static final int max_zoom = 21;

	public static final String[] maptype_names = {
		"hybrid",
		"roadmap",
		"satellite",
		"terrain"
	};

	public static final String[] maptype_labels = {
		"Hybrid",
		"Roadmap",
		"Satellite",
		"Terrain"
	};

	AltosMapInterface	map_interface;
	int			scale;

	AltosMapCache		cache;

	public AltosMapCache cache() { return cache; }

	LinkedList<AltosMapMark> marks = new LinkedList<AltosMapMark>();

	AltosMapPath		path;
	AltosMapLine		line;
	public AltosLatLon	last_position;

	boolean		have_boost = false;
	boolean		have_landed = false;

	ConcurrentHashMap<AltosPointInt,AltosMapTile> tiles = new ConcurrentHashMap<AltosPointInt,AltosMapTile>();
	int		load_radius;
	AltosLatLon	load_centre = null;
	AltosMapTileListener	load_listener;

	int 		zoom = AltosMap.default_zoom;
	int		maptype = AltosMap.maptype_default;

	long		user_input_time;

	/* Milliseconds to wait after user action before auto-scrolling
	 */
	static final long auto_scroll_delay = 20 * 1000;

	public AltosMapTransform	transform;
	public AltosLatLon		centre;

	public void reset() {
		// nothing
	}

	/* MapInterface wrapping functions */

	public void repaint(int x, int y, int w, int h) {
		map_interface.repaint(new AltosRectangle(x, y, w, h));
	}

	public void repaint(AltosMapRectangle damage, int pad) {
		AltosRectangle r = transform.screen(damage);
		repaint(r.x - pad, r.y - pad, r.width + pad * 2, r.height + pad * 2);
	}

	public void repaint() {
		map_interface.repaint();
	}

	public int width() {
		return map_interface.width();
	}

	public int height() {
		return map_interface.height();
	}

	public void debug(String format, Object ... arguments) {
		map_interface.debug(format, arguments);
	}

	static public AltosPointInt floor(AltosPointDouble point) {
		return new AltosPointInt ((int) Math.floor(point.x / AltosMap.px_size) * AltosMap.px_size,
					      (int) Math.floor(point.y / AltosMap.px_size) * AltosMap.px_size);
	}

	static public AltosPointInt ceil(AltosPointDouble point) {
		return new AltosPointInt ((int) Math.ceil(point.x / AltosMap.px_size) * AltosMap.px_size,
					      (int) Math.ceil(point.y / AltosMap.px_size) * AltosMap.px_size);
	}

	public void notice_user_input() {
		user_input_time = System.currentTimeMillis();
	}

	public boolean recent_user_input() {
		return (System.currentTimeMillis() - user_input_time) < auto_scroll_delay;
	}

	public boolean has_centre() {
		return centre != null;
	}

	public boolean far_from_centre(AltosLatLon lat_lon) {

		if (centre == null || transform == null)
			return true;

		AltosPointDouble	screen = transform.screen(lat_lon);

		int		width = width();
		int		dx = Math.abs ((int) (double) screen.x - width/2);

		if (dx > width / 4)
			return true;

		int		height = height();
		int		dy = Math.abs ((int) (double) screen.y - height/2);

		if (dy > height / 4)
			return true;

		return false;
	}

	public void set_transform() {
		if (centre != null) {
			transform = new AltosMapTransform(width(), height(), zoom, centre);
			repaint();
		}
	}

	private void set_zoom_label() {
		map_interface.set_zoom_label(String.format("Zoom %d", get_zoom() - default_zoom));
	}


	public boolean set_zoom(int zoom) {
		notice_user_input();
		if (AltosMap.min_zoom <= zoom && zoom <= AltosMap.max_zoom && zoom != this.zoom) {
			this.zoom = zoom;
			tiles.clear();
			set_transform();
			set_zoom_label();
			return true;
		}
		return false;
	}

	public boolean set_zoom_centre(int zoom, AltosPointInt centre) {
		AltosLatLon	mouse_lat_lon = null;
		boolean		ret;

		if (transform != null)
			mouse_lat_lon = transform.screen_lat_lon(centre);

		ret = set_zoom(zoom);

		if (ret && mouse_lat_lon != null) {
			AltosPointDouble	new_mouse = transform.screen(mouse_lat_lon);

			double	dx = width()/2.0 - centre.x;
			double	dy = height()/2.0 - centre.y;

			AltosLatLon	new_centre = transform.screen_lat_lon(new AltosPointDouble(new_mouse.x + dx, new_mouse.y + dy));

			centre(new_centre);
		}

		return ret;
	}

	public int get_zoom() {
		return zoom;
	}

	public boolean set_maptype(int maptype) {
/*
		if (maptype != this.maptype) {
			this.maptype = maptype;
			tiles.clear();
			repaint();
			return true;
		}
*/
		return false;
	}

	public void show(AltosGPS gps, double time, int state, double gps_height) {
		/*
		 * If insufficient gps data, nothing to update
		 */

		if (gps == null)
			return;

		if (!gps.locked && gps.nsat < 4)
			return;

		switch (state) {
		case AltosLib.ao_flight_boost:
			if (!have_boost) {
				add_mark(gps.lat, gps.lon, state);
				have_boost = true;
			}
			break;
		case AltosLib.ao_flight_landed:
			if (!have_landed) {
				add_mark(gps.lat, gps.lon, state);
				have_landed = true;
			}
			break;
		}

		if (path != null) {
			AltosMapRectangle	damage = path.add(gps, time, state, gps_height);

			if (damage != null)
				repaint(damage, AltosMapPath.stroke_width);
		}

		last_position = new AltosLatLon(gps.lat, gps.lon);

		maybe_centre(gps.lat, gps.lon);
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		show(state.gps, state.time, state.state(), state.gps_height());
	}

	public void centre(AltosLatLon lat_lon) {
		centre = lat_lon;
		set_transform();
	}

	public void centre(double lat, double lon) {
		centre(new AltosLatLon(lat, lon));
	}

	public void centre(AltosGPS gps) {
		if (!gps.locked && gps.nsat < 4)
			return;
		centre(gps.lat, gps.lon);
	}

	public void centre(AltosState state) {
		centre(state.gps);
	}

	public void maybe_centre(double lat, double lon) {
		AltosLatLon	lat_lon = new AltosLatLon(lat, lon);
		if (centre == null || (!recent_user_input() && far_from_centre(lat_lon)))
			centre(lat_lon);
	}

	public AltosMapMark add_mark(double lat, double lon, int state, String label) {
		AltosMapMark mark;
		synchronized(marks) {
			mark = map_interface.new_mark(lat, lon, state, label);
			if (mark != null)
				marks.add(mark);
		}
		repaint();
		return mark;
	}

	public AltosMapMark add_mark(double lat, double lon, int state) {
		return add_mark(lat, lon, state, null);
	}

	public void del_mark(AltosMapMark mark) {
		marks.remove(mark);
	}

	public void clear_marks() {
		synchronized(marks) {
			marks.clear();
		}
	}

	private void make_tiles() {
		AltosPointInt	upper_left;
		AltosPointInt	lower_right;

		if (load_centre != null) {
			AltosPointInt centre = floor(transform.point(load_centre));

			upper_left = new AltosPointInt(centre.x - load_radius * AltosMap.px_size,
							       centre.y - load_radius * AltosMap.px_size);
			lower_right = new AltosPointInt(centre.x + load_radius * AltosMap.px_size,
								centre.y + load_radius * AltosMap.px_size);
		} else {
			upper_left = floor(transform.screen_point(new AltosPointInt(0, 0)));
			lower_right = floor(transform.screen_point(new AltosPointInt(width(), height())));
		}

		Enumeration<AltosPointInt> keyEnumeration = tiles.keys();

		while (keyEnumeration.hasMoreElements()) {
			AltosPointInt point = keyEnumeration.nextElement();
			if (point.x < upper_left.x || lower_right.x < point.x ||
			    point.y < upper_left.y || lower_right.y < point.y) {
				tiles.remove(point);
			}
		}

		cache.set_cache_size((width() / AltosMap.px_size + 2) * (height() / AltosMap.px_size + 2));

		for (int y = (int) upper_left.y; y <= lower_right.y; y += AltosMap.px_size) {
			for (int x = (int) upper_left.x; x <= lower_right.x; x += AltosMap.px_size) {
				AltosPointInt	point = new AltosPointInt(x, y);

				if (!tiles.containsKey(point)) {
					AltosLatLon	ul = transform.lat_lon(point);
					AltosLatLon	center = transform.lat_lon(new AltosPointDouble(x + AltosMap.px_size/2, y + AltosMap.px_size/2));
					AltosMapTile tile = map_interface.new_tile(cache, ul, center, zoom, maptype, px_size, scale);
					int status = tile.store.status();
					if (status == AltosMapTile.fetching)
						debug("Fetching %.6f %.6f %d\n", center.lat, center.lon, zoom);
					tile.add_listener(this);
					tiles.put(point, tile);
				}
			}
		}
	}

	public void set_load_params(int new_zoom, int new_type, double lat, double lon, int radius, AltosMapTileListener listener) {
		if (AltosMap.min_zoom <= new_zoom && new_zoom <= AltosMap.max_zoom)
			zoom = new_zoom;
/*		maptype = new_type; */
		load_centre = new AltosLatLon(lat, lon);
		load_radius = radius;
		load_listener = listener;
		centre(lat, lon);
		tiles.clear();
		make_tiles();
		repaint();
	}

	public String getName() {
		return "Map";
	}

	public void paint() {
		if (centre != null)
			make_tiles();

		if (transform == null)
			return;

		for (AltosMapTile tile : tiles.values())
			tile.paint(transform);

		synchronized(marks) {
			for (AltosMapMark mark : marks)
				mark.paint(transform);
		}

		if (path != null)
			path.paint(transform);

		if (line != null)
			line.paint(transform);
	}

	/* AltosMapTileListener methods */
	public synchronized void notify_tile(AltosMapTile tile, int status) {
		Enumeration<AltosPointInt> keyEnumeration = tiles.keys();

		while (keyEnumeration.hasMoreElements()) {
			AltosPointInt point = keyEnumeration.nextElement();
			if (tile == tiles.get(point)) {
				AltosPointInt	screen = transform.screen(point);
				repaint(screen.x, screen.y, AltosMap.px_size, AltosMap.px_size);
			}
		}
	}

	/* AltosMapStoreListener methods */
	public synchronized void notify_store(AltosMapStore store, int status) {
		if (load_listener != null) {
			for (AltosMapTile tile : tiles.values())
				if (store.equals(tile.store))
					load_listener.notify_tile(tile, status);
		}
	}

	/* UI elements */

	AltosPointInt	drag_start;

	boolean		dragged;

	static final double drag_far = 20;

	private void drag(int x, int y) {
		if (drag_start == null)
			return;

		int dx = x - drag_start.x;
		int dy = y - drag_start.y;

		double distance = Math.hypot(dx, dy);

		if (distance > drag_far)
			dragged = true;

		if (transform == null)
			return;

		AltosLatLon new_centre = transform.screen_lat_lon(new AltosPointInt(width() / 2 - dx, height() / 2 - dy));
		centre(new_centre);
		drag_start = new AltosPointInt(x, y);
	}

	private void drag_start(int x, int y) {
		drag_start = new AltosPointInt(x, y);
		dragged = false;
	}

	private void drag_stop(int x, int y) {
		if (!dragged) {
			if (transform == null) {
				return;
			}
			map_interface.select_object (transform.screen_lat_lon(new AltosPointInt(x,y)));
		}
	}

	private void line_start(int x, int y) {
		if (line != null && transform != null) {
			line.pressed(new AltosPointInt(x, y), transform);
			repaint();
		}
	}

	private void line(int x, int y) {
		if (line != null && transform != null) {
			line.dragged(new AltosPointInt(x, y), transform);
			repaint();
		}
	}

	public void touch_start(int x, int y, boolean is_drag) {
		notice_user_input();
		if (is_drag)
			drag_start(x, y);
		else
			line_start(x, y);
	}

	public void touch_continue(int x, int y, boolean is_drag) {
		notice_user_input();
		if (is_drag)
			drag(x, y);
		else
			line(x, y);
	}

	public void touch_stop(int x, int y, boolean is_drag) {
		notice_user_input();
		if (is_drag)
			drag_stop(x, y);
	}

	public AltosMapPathPoint nearest(int x, int y) {
		notice_user_input();
		if (path == null)
			return null;
		if (transform == null)
			return null;
		AltosLatLon	at = transform.screen_lat_lon(new  AltosPointInt(x, y));
		return path.nearest(at);
	}

	public AltosMap(AltosMapInterface map_interface, int scale) {
		this.map_interface = map_interface;
		this.scale = scale;
		cache = new AltosMapCache(map_interface);
		line = map_interface.new_line();
		path = map_interface.new_path();
		set_zoom_label();
	}

	public AltosMap(AltosMapInterface map_interface) {
		this(map_interface, 1);
	}
}
