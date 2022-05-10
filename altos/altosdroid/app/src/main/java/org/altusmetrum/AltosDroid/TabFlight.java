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
import android.view.*;
import android.widget.*;
import android.location.Location;

public class TabFlight extends AltosDroidTab {
	private TextView speed_view;
	private TextView height_view;
	private TextView altitude_view;
	private View tilt_view;
	private TextView tilt_value;
	private TextView max_speed_view;
	private TextView max_height_view;
	private TextView max_altitude_view;
	private TextView elevation_view;
	private TextView range_view;
	private TextView bearing_view;
	private TextView compass_view;
	private TextView distance_view;
	private TextView latitude_view;
	private TextView longitude_view;
	private View apogee_view;
	private TextView apogee_voltage_view;
	private TextView apogee_voltage_label;
	private GoNoGoLights apogee_lights;
	private View main_view;
	private TextView main_voltage_view;
	private TextView main_voltage_label;
	private GoNoGoLights main_lights;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.tab_flight, container, false);

		speed_view      = (TextView) v.findViewById(R.id.speed_value);
		height_view     = (TextView) v.findViewById(R.id.height_value);
		altitude_view   = (TextView) v.findViewById(R.id.altitude_value);
		tilt_view 	= (View) v.findViewById(R.id.tilt_view);
		tilt_value 	= (TextView) v.findViewById(R.id.tilt_value);
		max_speed_view  = (TextView) v.findViewById(R.id.max_speed_value);
		max_height_view = (TextView) v.findViewById(R.id.max_height_value);
		max_altitude_view= (TextView) v.findViewById(R.id.max_altitude_value);
		elevation_view = (TextView) v.findViewById(R.id.elevation_value);
		range_view     = (TextView) v.findViewById(R.id.range_value);
		bearing_view   = (TextView) v.findViewById(R.id.bearing_value);
		compass_view   = (TextView) v.findViewById(R.id.compass_value);
		distance_view  = (TextView) v.findViewById(R.id.distance_value);
		latitude_view  = (TextView) v.findViewById(R.id.lat_value);
		longitude_view = (TextView) v.findViewById(R.id.lon_value);

		apogee_view = v.findViewById(R.id.apogee_view);
		apogee_voltage_view = (TextView) v.findViewById(R.id.apogee_voltage_value);
		apogee_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.apogee_redled),
		                                 (ImageView) v.findViewById(R.id.apogee_greenled),
		                                 getResources());
		apogee_voltage_label = (TextView) v.findViewById(R.id.apogee_voltage_label);

		main_view = v.findViewById(R.id.main_view);
		main_voltage_view = (TextView) v.findViewById(R.id.main_voltage_value);
		main_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.main_redled),
		                               (ImageView) v.findViewById(R.id.main_greenled),
		                               getResources());
		main_voltage_label = (TextView) v.findViewById(R.id.main_voltage_label);

		return v;
	}

	public String tab_name() { return AltosDroid.tab_flight_name; }

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		if (state != null) {
			set_value(speed_view, AltosConvert.speed, 1, state.speed());
			set_value(height_view, AltosConvert.height, 1, state.height());
			set_value(altitude_view, AltosConvert.height, 1, state.altitude());
			double orient = state.orient();
			if (orient == AltosLib.MISSING) {
				tilt_view.setVisibility(View.GONE);
			} else {
				tilt_value.setText(AltosDroid.number("%1.0f°", orient));
				tilt_view.setVisibility(View.VISIBLE);
			}
			set_value(max_speed_view, AltosConvert.speed, 1, state.max_speed());
			set_value(max_height_view, AltosConvert.height, 1, state.max_height());
			set_value(max_altitude_view, AltosConvert.height, 1, state.max_altitude());
			if (from_receiver != null) {
				elevation_view.setText(AltosDroid.number("%1.0f°", from_receiver.elevation));
				set_value(range_view, AltosConvert.distance, 1, from_receiver.range);
				bearing_view.setText(AltosDroid.number("%1.0f°", from_receiver.bearing));
				compass_view.setText(from_receiver.bearing_words(AltosGreatCircle.BEARING_LONG));
				set_value(distance_view, AltosConvert.distance, 1, from_receiver.distance);
			} else { 
				elevation_view.setText("<unknown>");
				range_view.setText("<unknown>");
				bearing_view.setText("<unknown>");
				compass_view.setText("<unknown>");
				distance_view.setText("<unknown>");
			}
			if (state.gps != null) {
				latitude_view.setText(AltosDroid.pos(state.gps.lat, "N", "S"));
				longitude_view.setText(AltosDroid.pos(state.gps.lon, "E", "W"));
			}

			if (state.apogee_voltage == AltosLib.MISSING) {
				apogee_view.setVisibility(View.GONE);
			} else {
				apogee_voltage_view.setText(AltosDroid.number("%1.2f V", state.apogee_voltage));
				apogee_lights.set(state.apogee_voltage > 3.2, state.apogee_voltage == AltosLib.MISSING);
				apogee_view.setVisibility(View.VISIBLE);
			}

			if (state.main_voltage == AltosLib.MISSING) {
				main_view.setVisibility(View.GONE);
			} else {
				main_voltage_view.setText(AltosDroid.number("%1.2f V", state.main_voltage));
				main_lights.set(state.main_voltage > 3.2, state.main_voltage == AltosLib.MISSING);
				main_view.setVisibility(View.VISIBLE);
			}
		}
	}

}
