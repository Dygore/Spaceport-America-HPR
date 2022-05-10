/*
 * Copyright © 2013 Mike Beattie <mike@ethernal.org>
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

import org.altusmetrum.altoslib_14.*;

import com.google.android.gms.maps.*;
import com.google.android.gms.maps.model.*;

import android.graphics.Color;
import android.graphics.*;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.location.Location;
import android.content.*;

class RocketOnline implements Comparable {
	Marker		marker;
	int		serial;
	long		last_packet;
	int		size;

	void set_position(AltosLatLon position, long last_packet) {
		marker.setPosition(new LatLng(position.lat, position.lon));
		this.last_packet = last_packet;
	}

	private Bitmap rocket_bitmap(Context context, String text) {

		/* From: http://mapicons.nicolasmollet.com/markers/industry/military/missile-2/
		 */
		Bitmap orig_bitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.rocket);
		Bitmap bitmap = orig_bitmap.copy(Bitmap.Config.ARGB_8888, true);

		Canvas canvas = new Canvas(bitmap);
		Paint paint = new Paint();
		paint.setTextSize(40);
		paint.setColor(0xff000000);

		Rect	bounds = new Rect();
		paint.getTextBounds(text, 0, text.length(), bounds);

		int	width = bounds.right - bounds.left;
		int	height = bounds.bottom - bounds.top;

		float x = bitmap.getWidth() / 2.0f - width / 2.0f;
		float y = bitmap.getHeight() / 2.0f - height / 2.0f;

		size = bitmap.getWidth();

		canvas.drawText(text, 0, text.length(), x, y, paint);
		return bitmap;
	}

	public void remove() {
		marker.remove();
	}

	public int compareTo(Object o) {
		RocketOnline other = (RocketOnline) o;

		long	diff = last_packet - other.last_packet;

		if (diff > 0)
			return 1;
		if (diff < 0)
			return -1;
		return 0;
	}

	RocketOnline(Context context, int serial, GoogleMap map, double lat, double lon, long last_packet) {
		this.serial = serial;
		String name = String.format("%d", serial);
		this.marker = map.addMarker(new MarkerOptions()
					    .icon(BitmapDescriptorFactory.fromBitmap(rocket_bitmap(context, name)))
					    .position(new LatLng(lat, lon))
					    .visible(true));
		this.last_packet = last_packet;
	}
}

public class AltosMapOnline implements AltosDroidMapInterface, GoogleMap.OnMarkerClickListener, GoogleMap.OnMapClickListener, OnMapReadyCallback, AltosMapTypeListener {
	public AltosOnlineMapFragment mMapFragment;
	private GoogleMap mMap;
	private boolean mapLoaded = false;
	Context context;

	private HashMap<Integer,RocketOnline> rockets = new HashMap<Integer,RocketOnline>();
	private Marker mPadMarker;
	private boolean pad_set;
	private Polyline mPolyline;

	private double mapAccuracy = -1;

	private AltosLatLon my_position = null;
	private AltosLatLon target_position = null;

	private AltosDroid altos_droid;

	public static class AltosOnlineMapFragment extends SupportMapFragment {
		AltosMapOnline c;
		View map_view;

		public AltosOnlineMapFragment(AltosMapOnline c) {
			this.c = c;
		}

		public AltosOnlineMapFragment() {
		}

		@Override
		public void onActivityCreated(Bundle savedInstanceState) {
			super.onActivityCreated(savedInstanceState);
			if (c != null)
				getMapAsync(c);
		}
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			map_view = super.onCreateView(inflater, container, savedInstanceState);
			return map_view;
		}
		@Override
		public void onDestroyView() {
			super.onDestroyView();
			map_view = null;
		}
		public void set_visible(boolean visible) {
			if (map_view == null)
				return;
			if (visible)
				map_view.setVisibility(View.VISIBLE);
			else
				map_view.setVisibility(View.GONE);
		}
	}

	public void onCreateView(AltosDroid altos_droid) {
		this.altos_droid = altos_droid;
		AltosPreferences.register_map_type_listener(this);
		mMapFragment = new AltosOnlineMapFragment(this);
	}

	public void onDestroyView() {
		AltosPreferences.unregister_map_type_listener(this);
	}

	private double pixel_distance(LatLng a, LatLng b) {
		Projection projection = mMap.getProjection();

		Point	a_pt = projection.toScreenLocation(a);
		Point	b_pt = projection.toScreenLocation(b);

		return Math.hypot((double) (a_pt.x - b_pt.x), (double) (a_pt.y - b_pt.y));
	}

	private RocketOnline[] sorted_rockets() {
		synchronized(rockets) {
			RocketOnline[]	rocket_array = rockets.values().toArray(new RocketOnline[0]);

			Arrays.sort(rocket_array);
			return rocket_array;
		}
	}

	public void onMapClick(LatLng lat_lng) {
		ArrayList<Integer>	near = new ArrayList<Integer>();

		for (RocketOnline rocket : sorted_rockets()) {
			LatLng	pos = rocket.marker.getPosition();

			if (pos == null)
				continue;

			double distance = pixel_distance(lat_lng, pos);
			if (distance < rocket.size * 2)
				near.add(rocket.serial);
		}

		if (near.size() != 0)
			altos_droid.touch_trackers(near.toArray(new Integer[0]));
	}

	public boolean onMarkerClick(Marker marker) {
		onMapClick(marker.getPosition());
		return true;
	}

	void
	position_permission() {
		if (mMap != null)
			mMap.setMyLocationEnabled(true);
	}

	@Override
	public void onMapReady(GoogleMap googleMap) {
		final int map_type = AltosPreferences.map_type();
		mMap = googleMap;
		if (mMap != null) {
			map_type_changed(map_type);
			if (altos_droid.have_location_permission)
				mMap.setMyLocationEnabled(true);
			else
				altos_droid.tell_map_permission(this);
			mMap.getUiSettings().setTiltGesturesEnabled(false);
			mMap.getUiSettings().setZoomControlsEnabled(false);
			mMap.setOnMarkerClickListener(this);
			mMap.setOnMapClickListener(this);

			mPadMarker = mMap.addMarker(
					new MarkerOptions().icon(BitmapDescriptorFactory.fromResource(R.drawable.pad))
					                   .position(new LatLng(0,0))
					                   .visible(false)
					);

			mPolyline = mMap.addPolyline(
					new PolylineOptions().add(new LatLng(0,0), new LatLng(0,0))
					                     .width(20)
					                     .color(Color.BLUE)
					                     .visible(false)
					);

			mapLoaded = true;
		}
	}

	public void center(double lat, double lon, double accuracy) {
		if (mMap == null)
			return;

		if (mapAccuracy < 0 || accuracy < mapAccuracy/10) {
			mMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(lat, lon),14));
			mapAccuracy = accuracy;
		}
	}

	private void set_rocket(int serial, AltosState state) {
		RocketOnline	rocket;

		if (state.gps == null || state.gps.lat == AltosLib.MISSING)
			return;

		if (mMap == null)
			return;

		synchronized(rockets) {
			if (rockets.containsKey(serial)) {
				rocket = rockets.get(serial);
				rocket.set_position(new AltosLatLon(state.gps.lat, state.gps.lon), state.received_time);
			} else {
				rocket = new RocketOnline(context,
							  serial,
							  mMap, state.gps.lat, state.gps.lon,
							  state.received_time);
				rockets.put(serial, rocket);
			}
		}
	}

	private void remove_rocket(int serial) {
		synchronized(rockets) {
			RocketOnline rocket = rockets.get(serial);
			rocket.remove();
			rockets.remove(serial);
		}
	}

	public void set_visible(boolean visible) {
		if (mMapFragment != null)
			mMapFragment.set_visible(visible);
	}

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {

		if (telem_state != null) {
			synchronized(rockets) {
				for (int serial : rockets.keySet()) {
					if (!telem_state.containsKey(serial))
						remove_rocket(serial);
				}

				for (int serial : telem_state.keySet()) {
					set_rocket(serial, telem_state.get(serial));
				}
			}
		}

		if (state != null) {
			if (mapLoaded) {
				if (!pad_set && state.pad_lat != AltosLib.MISSING) {
					pad_set = true;
					mPadMarker.setPosition(new LatLng(state.pad_lat, state.pad_lon));
					mPadMarker.setVisible(true);
				}
			}
			if (state.gps != null && state.gps.lat != AltosLib.MISSING) {

				target_position = new AltosLatLon(state.gps.lat, state.gps.lon);
				if (state.gps.locked && state.gps.nsat >= 4)
					center (state.gps.lat, state.gps.lon, 10);
			}
		}

		if (receiver != null) {
			double accuracy;

			if (receiver.hasAccuracy())
				accuracy = receiver.getAccuracy();
			else
				accuracy = 1000;

			my_position = new AltosLatLon(receiver.getLatitude(), receiver.getLongitude());
			center (my_position.lat, my_position.lon, accuracy);
		}

		if (my_position != null && target_position != null && mPolyline != null) {
			mPolyline.setPoints(Arrays.asList(new LatLng(my_position.lat, my_position.lon), new LatLng(target_position.lat, target_position.lon)));
			mPolyline.setVisible(true);
		}

	}

	public void map_type_changed(int map_type) {
		if (mMap != null) {
			if (map_type == AltosMap.maptype_hybrid)
				mMap.setMapType(GoogleMap.MAP_TYPE_HYBRID);
			else if (map_type == AltosMap.maptype_satellite)
				mMap.setMapType(GoogleMap.MAP_TYPE_SATELLITE);
			else if (map_type == AltosMap.maptype_terrain)
				mMap.setMapType(GoogleMap.MAP_TYPE_TERRAIN);
			else
				mMap.setMapType(GoogleMap.MAP_TYPE_NORMAL);
		}
	}

	public AltosMapOnline(Context context) {
		this.context = context;
	}
}
