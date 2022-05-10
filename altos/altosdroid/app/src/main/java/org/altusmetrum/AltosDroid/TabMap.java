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

import org.altusmetrum.altoslib_14.*;

import android.app.Activity;
import android.os.Bundle;
import android.view.*;
import android.widget.*;
import android.location.Location;

public class TabMap extends AltosDroidTab implements AltosDroidMapSourceListener {

	AltosLatLon	here;

	private TextView mDistanceView;
	private TextView mBearingLabel;
	private TextView mBearingView;
	private TextView mTargetLatitudeView;
	private TextView mTargetLongitudeView;
	private TextView mReceiverLatitudeView;
	private TextView mReceiverLongitudeView;
	private AltosMapOffline map_offline;
	private AltosMapOnline map_online;
	private View view;
	private int map_source;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		view = inflater.inflate(R.layout.tab_map, container, false);
		int map_source = AltosDroidPreferences.map_source();

		mDistanceView  = (TextView)view.findViewById(R.id.distance_value);
		mBearingLabel  = (TextView)view.findViewById(R.id.bearing_label);
		mBearingView   = (TextView)view.findViewById(R.id.bearing_value);
		mTargetLatitudeView  = (TextView)view.findViewById(R.id.target_lat_value);
		mTargetLongitudeView = (TextView)view.findViewById(R.id.target_lon_value);
		mReceiverLatitudeView  = (TextView)view.findViewById(R.id.receiver_lat_value);
		mReceiverLongitudeView = (TextView)view.findViewById(R.id.receiver_lon_value);
		map_offline = (AltosMapOffline)view.findViewById(R.id.map_offline);
		map_offline.onCreateView(altos_droid);
		map_online = new AltosMapOnline(view.getContext());
		map_online.onCreateView(altos_droid);
		map_source_changed(AltosDroidPreferences.map_source());
		AltosDroidPreferences.register_map_source_listener(this);
		return view;
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		if (map_online != null)
			getChildFragmentManager().beginTransaction().add(R.id.map_online, map_online.mMapFragment).commit();
	}

	@Override
	public void onDestroyView() {
		super.onDestroyView();
		map_offline.onDestroyView();
		map_online.onDestroyView();
		AltosDroidPreferences.unregister_map_source_listener(this);
	}

	public String tab_name() { return AltosDroid.tab_map_name; }

	private void center(double lat, double lon, double accuracy) {
		if (map_offline != null)
			map_offline.center(lat, lon, accuracy);
		if (map_online != null)
			map_online.center(lat, lon, accuracy);
	}

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (from_receiver != null) {
			String	direction = AltosDroid.direction(from_receiver, receiver);
			if (direction != null) {
				mBearingLabel.setText("Direction");
				mBearingView.setText(direction);
			} else {
				mBearingLabel.setText("Bearing");
				mBearingView.setText(String.format("%3.0f°", from_receiver.bearing));
			}
			set_value(mDistanceView, AltosConvert.distance, 6, from_receiver.distance);
		} else {
			mBearingLabel.setText("Bearing");
			mBearingView.setText("");
			set_value(mDistanceView, AltosConvert.distance, 6, AltosLib.MISSING);
		}

		if (state != null) {
			if (state.gps != null) {
				mTargetLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
				mTargetLongitudeView.setText(AltosDroid.pos(state.gps.lon, "E", "W"));
			}
		}

		if (receiver != null) {
			double accuracy;

			here = new AltosLatLon(receiver.getLatitude(), receiver.getLongitude());
			if (receiver.hasAccuracy())
				accuracy = receiver.getAccuracy();
			else
				accuracy = 1000;
			mReceiverLatitudeView.setText(AltosDroid.pos(here.lat, "N", "S"));
			mReceiverLongitudeView.setText(AltosDroid.pos(here.lon, "E", "W"));
			center (here.lat, here.lon, accuracy);
		}
		if (map_source == AltosDroidPreferences.MAP_SOURCE_OFFLINE) {
			if (map_offline != null)
				map_offline.show(telem_state, state, from_receiver, receiver);
		} else {
			if (map_online != null)
				map_online.show(telem_state, state, from_receiver, receiver);
		}
	}

	public void map_source_changed(int map_source) {
		this.map_source = map_source;
		if (map_source == AltosDroidPreferences.MAP_SOURCE_OFFLINE) {
			if (map_online != null)
				map_online.set_visible(false);
			if (map_offline != null) {
				map_offline.set_visible(true);
				map_offline.show(last_telem_state, last_state, last_from_receiver, last_receiver);
			}
		} else {
			if (map_offline != null)
				map_offline.set_visible(false);
			if (map_online != null) {
				map_online.set_visible(true);
				map_online.show(last_telem_state, last_state, last_from_receiver, last_receiver);
			}
		}
	}

	public TabMap() {
	}
}
