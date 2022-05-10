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

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.location.Location;

public class TabRecover extends AltosDroidTab {
	private TextView mBearingView;
	private TextView mDirectionView;
	private TextView mDistanceView;
	private TextView mTargetLatitudeView;
	private TextView mTargetLongitudeView;
	private TextView mReceiverLatitudeView;
	private TextView mReceiverLongitudeView;
	private TextView mMaxHeightView;
	private TextView mMaxSpeedView;
	private TextView mMaxAccelView;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_recover, container, false);

		mBearingView   = (TextView) v.findViewById(R.id.bearing_value);
		mDirectionView = (TextView) v.findViewById(R.id.direction_value);
		mDistanceView  = (TextView) v.findViewById(R.id.distance_value);
		mTargetLatitudeView  = (TextView) v.findViewById(R.id.target_lat_value);
		mTargetLongitudeView = (TextView) v.findViewById(R.id.target_lon_value);
		mReceiverLatitudeView  = (TextView) v.findViewById(R.id.receiver_lat_value);
		mReceiverLongitudeView = (TextView) v.findViewById(R.id.receiver_lon_value);
		mMaxHeightView = (TextView) v.findViewById(R.id.max_height_value);
		mMaxSpeedView  = (TextView) v.findViewById(R.id.max_speed_value);
		mMaxAccelView  = (TextView) v.findViewById(R.id.max_accel_value);

		return v;
	}

	public String tab_name() { return AltosDroid.tab_recover_name; }

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (from_receiver != null) {
			mBearingView.setText(String.format("%1.0f°", from_receiver.bearing));
			set_value(mDistanceView, AltosConvert.distance, 1, from_receiver.distance);
			String direction = AltosDroid.direction(from_receiver, receiver);
			if (direction == null)
				mDirectionView.setText("");
			else
				mDirectionView.setText(direction);
		}
		if (state != null && state.gps != null) {
			mTargetLatitudeView.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
			mTargetLongitudeView.setText(AltosDroid.pos(state.gps.lon, "E", "W"));
		}

		if (receiver != null) {
			mReceiverLatitudeView.setText(AltosDroid.pos(receiver.getLatitude(), "N", "S"));
			mReceiverLongitudeView.setText(AltosDroid.pos(receiver.getLongitude(), "E", "W"));
		}

		if (state != null) {
			set_value(mMaxHeightView, AltosConvert.height, 1, state.max_height());
			set_value(mMaxAccelView, AltosConvert.accel, 1, state.max_acceleration());
			set_value(mMaxSpeedView, AltosConvert.speed, 1, state.max_speed());
		}
	}
}
