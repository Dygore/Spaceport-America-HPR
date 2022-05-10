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

package org.altusmetrum.AltosDroid;

import java.util.*;
import java.io.*;

import org.altusmetrum.altoslib_14.*;

import android.graphics.*;
import android.view.*;
import android.location.Location;
import android.content.*;
import android.util.*;

class Rocket implements Comparable {
	AltosLatLon	position;
	String		name;
	int		serial;
	long		last_packet;
	boolean		active;
	AltosMapOffline	map_offline;

	void paint() {
		map_offline.draw_bitmap(position, map_offline.rocket_bitmap, map_offline.rocket_off_x, map_offline.rocket_off_y);
		map_offline.draw_text(position, name, 0, 3*map_offline.rocket_bitmap.getHeight()/4);
	}

	void set_position(AltosLatLon position, long last_packet) {
		this.position = position;
		this.last_packet = last_packet;
	}

	void set_active(boolean active) {
		this.active = active;
	}

	public int compareTo(Object o) {
		Rocket other = (Rocket) o;

		if (active && !other.active)
			return 1;
		if (other.active && !active)
			return -1;

		long	diff = last_packet - other.last_packet;

		if (diff > 0)
			return 1;
		if (diff < 0)
			return -1;
		return 0;
	}

	Rocket(int serial, AltosMapOffline map_offline) {
		this.serial = serial;
		this.name = String.format("%d", serial);
		this.map_offline = map_offline;
	}
}

public class AltosMapOffline extends View implements ScaleGestureDetector.OnScaleGestureListener, AltosMapInterface, AltosDroidMapInterface, AltosMapTypeListener {
	ScaleGestureDetector	scale_detector;
	boolean			scaling;
	AltosMap		map;
	AltosDroid		altos_droid;

	static int scale = 1;

	AltosLatLon	here;
	AltosLatLon	there;
	AltosLatLon	pad;

	Canvas	canvas;
	Paint	paint;

	Bitmap	pad_bitmap;
	int	pad_off_x, pad_off_y;
	Bitmap	rocket_bitmap;
	int	rocket_off_x, rocket_off_y;
	Bitmap	here_bitmap;
	int	here_off_x, here_off_y;

	static  final int	WHITE = 0xffffffff;
	static  final int	RED   = 0xffff0000;
	static  final int	PINK  = 0xffff8080;
	static  final int	YELLOW= 0xffffff00;
	static  final int	CYAN  = 0xff00ffff;
	static  final int	BLUE  = 0xff0000ff;
	static  final int	BLACK = 0xff000000;

	public static final int stateColors[] = {
		WHITE,  // startup
		WHITE,  // idle
		WHITE,  // pad
		RED,    // boost
		PINK,   // fast
		YELLOW, // coast
		CYAN,   // drogue
		BLUE,   // main
		BLACK,  // landed
		BLACK,  // invalid
		CYAN,   // stateless
	};

	/* AltosMapInterface */
	public void debug(String format, Object ... arguments) {
		AltosDebug.debug(format, arguments);
	}

	class MapTile extends AltosMapTile {
		public void paint(AltosMapTransform t) {
			AltosPointInt		pt = new AltosPointInt(t.screen(upper_left));

			if (canvas.quickReject(pt.x, pt.y, pt.x + px_size, pt.y + px_size, Canvas.EdgeType.AA))
				return;

			AltosImage		altos_image = this.get_image();

			MapImage 		map_image = (MapImage) altos_image;

			Bitmap			bitmap = null;

			if (map_image != null)
				bitmap = map_image.bitmap;

			if (bitmap != null) {
				canvas.drawBitmap(bitmap, pt.x, pt.y, paint);
			} else {
				paint.setColor(0xff808080);
				canvas.drawRect(pt.x, pt.y, pt.x + px_size, pt.y + px_size, paint);
				if (t.has_location()) {
					String	message = null;
					switch (status) {
					case AltosMapTile.fetching:
						message = "Fetching...";
						break;
					case AltosMapTile.bad_request:
						message = "Internal error";
						break;
					case AltosMapTile.failed:
						message = "Network error";
						break;
					case AltosMapTile.forbidden:
						message = "Outside of known launch areas";
						break;
					}
					if (message != null) {
						Rect	bounds = new Rect();
						paint.getTextBounds(message, 0, message.length(), bounds);

						int	width = bounds.right - bounds.left;
						int	height = bounds.bottom - bounds.top;

						float x = pt.x + px_size / 2.0f;
						float y = pt.y + px_size / 2.0f;
						x = x - width / 2.0f;
						y = y + height / 2.0f;
						paint.setColor(0xff000000);
						canvas.drawText(message, 0, message.length(), x, y, paint);
					}
				}
			}
		}

		public MapTile(AltosMapCache cache, AltosLatLon upper_left, AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
			super(cache, upper_left, center, zoom, maptype, px_size, scale);
		}

	}

	public AltosMapTile new_tile(AltosMapCache cache, AltosLatLon upper_left, AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		return new MapTile(cache, upper_left, center, zoom, maptype, px_size, scale);
	}

	public AltosMapPath new_path() {
		return null;
	}

	public AltosMapLine new_line() {
		return null;
	}

	class MapImage implements AltosImage {
		public Bitmap	bitmap;

		public void flush() {
			if (bitmap != null) {
				bitmap.recycle();
				bitmap = null;
			}
		}

		public MapImage(File file) {
			bitmap = BitmapFactory.decodeFile(file.getPath());
		}
	}

	public AltosImage load_image(File file) throws Exception {
		return new MapImage(file);
	}

	class MapMark extends AltosMapMark {
		public void paint(AltosMapTransform t) {
		}

		MapMark(double lat, double lon, int state) {
			super(lat, lon, state);
		}

		MapMark(double lat, double lon, int state, String label) {
			super(lat, lon, state, label);
		}
	}

	public AltosMapMark new_mark(double lat, double lon, int state) {
		return new MapMark(lat, lon, state);
	}

	public AltosMapMark new_mark(double lat, double lon, int state, String label) {
		return new MapMark(lat, lon, state, label);
	}

	public int width() {
		return getWidth();
	}

	public int height() {
		return getHeight();
	}

	public void repaint() {
		postInvalidate();
	}

	public void repaint(AltosRectangle damage) {
		postInvalidate(damage.x, damage.y, damage.x + damage.width, damage.y + damage.height);
	}

	public void set_zoom_label(String label) {
	}

	public void select_object(AltosLatLon latlon) {
		if (map.transform == null)
			return;
		ArrayList<Integer>	near = new ArrayList<Integer>();

		for (Rocket rocket : sorted_rockets()) {
			if (rocket.position == null) {
				debug("rocket %d has no position\n", rocket.serial);
				continue;
			}
			double distance = map.transform.hypot(latlon, rocket.position);
			debug("check select %d distance %g width %d\n", rocket.serial, distance, rocket_bitmap.getWidth());
			if (distance < rocket_bitmap.getWidth() * 2.0) {
				debug("selecting %d\n", rocket.serial);
				near.add(rocket.serial);
			}
		}
		if (near.size() != 0)
			altos_droid.touch_trackers(near.toArray(new Integer[0]));
	}

	class Line {
		AltosLatLon	a, b;

		void paint() {
			if (a != null && b != null) {
				AltosPointDouble	a_screen = map.transform.screen(a);
				AltosPointDouble	b_screen = map.transform.screen(b);
				paint.setColor(0xff8080ff);
				canvas.drawLine((float) a_screen.x, (float) a_screen.y,
						    (float) b_screen.x, (float) b_screen.y,
						    paint);
			}
		}

		void set_a(AltosLatLon a) {
			this.a = a;
		}

		void set_b(AltosLatLon b) {
			this.b = b;
		}

		Line() {
		}
	}

	Line line = new Line();

	int	stroke_width = 20;

	void draw_text(AltosLatLon lat_lon, String text, int off_x, int off_y) {
		if (lat_lon != null && map != null && map.transform != null) {
			AltosPointInt pt = new AltosPointInt(map.transform.screen(lat_lon));

			Rect	bounds = new Rect();
			paint.getTextBounds(text, 0, text.length(), bounds);

			int	width = bounds.right - bounds.left;
			int	height = bounds.bottom - bounds.top;

			float x = pt.x;
			float y = pt.y;
			x = x - width / 2.0f - off_x;
			y = y + height / 2.0f - off_y;
			paint.setColor(0xff000000);
			canvas.drawText(text, 0, text.length(), x, y, paint);
		}
	}

	HashMap<Integer,Rocket> rockets = new HashMap<Integer,Rocket>();

	void draw_bitmap(AltosLatLon lat_lon, Bitmap bitmap, int off_x, int off_y) {
		if (lat_lon != null && map != null && map.transform != null) {
			AltosPointInt pt = new AltosPointInt(map.transform.screen(lat_lon));

			canvas.drawBitmap(bitmap, pt.x - off_x, pt.y - off_y, paint);
		}
	}

	private Rocket[] sorted_rockets() {
		Rocket[]	rocket_array = rockets.values().toArray(new Rocket[0]);

		Arrays.sort(rocket_array);
		return rocket_array;
	}

	private void draw_positions() {
		line.set_a(there);
		line.set_b(here);
		line.paint();
		draw_bitmap(pad, pad_bitmap, pad_off_x, pad_off_y);

		for (Rocket rocket : sorted_rockets())
			rocket.paint();
		draw_bitmap(here, here_bitmap, here_off_x, here_off_y);
	}

	@Override
	protected void onDraw(Canvas view_canvas) {
		if (map == null) {
			debug("MapView draw without map\n");
			return;
		}
		if (map.transform == null) {
			debug("MapView draw without transform\n");
			return;
		}
		canvas = view_canvas;
		paint = new Paint(Paint.ANTI_ALIAS_FLAG);
		paint.setStrokeWidth(stroke_width);
		paint.setStrokeCap(Paint.Cap.ROUND);
		paint.setStrokeJoin(Paint.Join.ROUND);
		paint.setTextSize(40);
		map.paint();
		draw_positions();
		canvas = null;
	}

	public boolean onScale(ScaleGestureDetector detector) {
		float	f = detector.getScaleFactor();

		if (f <= 0.8) {
			map.set_zoom_centre(map.get_zoom() - 1, new AltosPointInt((int) detector.getFocusX(), (int) detector.getFocusY()));
			return true;
		}
		if (f >= 1.2) {
			map.set_zoom_centre(map.get_zoom() + 1, new AltosPointInt((int) detector.getFocusX(), (int) detector.getFocusY()));
			return true;
		}
		return false;
	}

	public boolean onScaleBegin(ScaleGestureDetector detector) {
		return true;
	}

	public void onScaleEnd(ScaleGestureDetector detector) {
	}

	@Override
	public boolean dispatchTouchEvent(MotionEvent event) {
		scale_detector.onTouchEvent(event);

		if (scale_detector.isInProgress()) {
			scaling = true;
		}

		if (scaling) {
			if (event.getAction() == MotionEvent.ACTION_UP) {
				scaling = false;
			}
			return true;
		}

		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			map.touch_start((int) event.getX(), (int) event.getY(), true);
		} else if (event.getAction() == MotionEvent.ACTION_MOVE) {
			map.touch_continue((int) event.getX(), (int) event.getY(), true);
		} else if (event.getAction() == MotionEvent.ACTION_UP) {
			map.touch_stop((int) event.getX(), (int) event.getY(), true);
		}
		return true;
	}

	double	mapAccuracy;

	public void center(double lat, double lon, double accuracy) {
		if (mapAccuracy <= 0 || accuracy < mapAccuracy/10 || (map != null && !map.has_centre())) {
			if (map != null)
				map.maybe_centre(lat, lon);
			mapAccuracy = accuracy;
		}
	}

	public void set_visible(boolean visible) {
		if (visible)
			setVisibility(VISIBLE);
		else
			setVisibility(GONE);
	}

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		boolean	changed = false;

		if (state != null) {
			map.show(state, null);
			if (state.pad_lat != AltosLib.MISSING && pad == null)
				pad = new AltosLatLon(state.pad_lat, state.pad_lon);
		}

		if (telem_state != null) {
			Integer[] old_serial = rockets.keySet().toArray(new Integer[0]);
			Integer[] new_serial = telem_state.keySet().toArray(new Integer[0]);

			/* remove deleted keys */
			for (int serial : old_serial) {
				if (!telem_state.containsKey(serial))
					rockets.remove(serial);
			}

			/* set remaining keys */

			for (int serial : new_serial) {
				Rocket 		rocket;
				AltosState	t_state = telem_state.get(serial);
				if (rockets.containsKey(serial))
					rocket = rockets.get(serial);
				else {
					rocket = new Rocket(serial, this);
					rockets.put(serial, rocket);
				}
				if (t_state.gps != null) {
					AltosLatLon	latlon = new AltosLatLon(t_state.gps.lat, t_state.gps.lon);
					rocket.set_position(latlon, t_state.received_time);
					if (state != null && state.cal_data().serial == serial)
						there = latlon;
				}
				if (state != null)
					rocket.set_active(state.cal_data().serial == serial);
			}
		}
		if (receiver != null) {
			AltosLatLon new_here = new AltosLatLon(receiver.getLatitude(), receiver.getLongitude());
			if (!new_here.equals(here)) {
				here = new_here;
				AltosDebug.debug("Location changed, redraw");
				repaint();
			}
		}
	}

	public void onCreateView(AltosDroid altos_droid) {
		this.altos_droid = altos_droid;
		map = new AltosMap(this, scale);
		AltosPreferences.register_map_type_listener(this);
		map.set_maptype(AltosPreferences.map_type());

		pad_bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.pad);
		/* arrow at the bottom of the launchpad image */
		pad_off_x = pad_bitmap.getWidth() / 2;
		pad_off_y = pad_bitmap.getHeight();

		rocket_bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.rocket);
		/* arrow at the bottom of the rocket image */
		rocket_off_x = rocket_bitmap.getWidth() / 2;
		rocket_off_y = rocket_bitmap.getHeight();

		here_bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_maps_indicator_current_position);
		/* Center of the dot */
		here_off_x = here_bitmap.getWidth() / 2;
		here_off_y = here_bitmap.getHeight() / 2;
	}

	public void onDestroyView() {
		AltosPreferences.unregister_map_type_listener(this);
	}

	public void map_type_changed(int map_type) {
		if (map != null)
			map.set_maptype(map_type);
	}

	public AltosMapOffline(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.altos_droid = altos_droid;
		scale_detector = new ScaleGestureDetector(context, this);
	}
}
