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

public class TabPad extends AltosDroidTab {
	private TextView battery_voltage_view;
	private GoNoGoLights battery_lights;

	private TableRow receiver_row;
	private TextView receiver_voltage_view;
	private TextView receiver_voltage_label;
	private GoNoGoLights receiver_voltage_lights;

	private TableRow apogee_row;
	private TextView apogee_voltage_view;
	private TextView apogee_voltage_label;
	private GoNoGoLights apogee_lights;

	private TableRow main_row;
	private TextView main_voltage_view;
	private TextView main_voltage_label;
	private GoNoGoLights main_lights;

	private TextView data_logging_view;
	private GoNoGoLights data_logging_lights;

	private TextView gps_locked_view;
	private GoNoGoLights gps_locked_lights;

	private TextView gps_ready_view;
	private GoNoGoLights gps_ready_lights;

	private TextView receiver_latitude_view;
	private TextView receiver_longitude_view;
	private TextView receiver_altitude_view;

	private TableRow[] ignite_row = new TableRow[4];
	private TextView[] ignite_voltage_view = new TextView[4];
	private TextView[] ignite_voltage_label = new TextView[4];
	private GoNoGoLights[] ignite_lights = new GoNoGoLights[4];

	private View tilt_view;
	private TextView tilt_value;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		AltosDebug.debug("TabPad onCreateView\n");
		View v = inflater.inflate(R.layout.tab_pad, container, false);
		battery_voltage_view = (TextView) v.findViewById(R.id.battery_voltage_value);
		battery_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.battery_redled),
		                                  (ImageView) v.findViewById(R.id.battery_greenled),
		                                  getResources());

		receiver_row = (TableRow) v.findViewById(R.id.receiver_row);
		receiver_voltage_view = (TextView) v.findViewById(R.id.receiver_voltage_value);
		receiver_voltage_label = (TextView) v.findViewById(R.id.receiver_voltage_label);
		receiver_voltage_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.receiver_redled),
							   (ImageView) v.findViewById(R.id.receiver_greenled),
							   getResources());

		apogee_row = (TableRow) v.findViewById(R.id.apogee_row);
		apogee_voltage_view = (TextView) v.findViewById(R.id.apogee_voltage_value);
		apogee_voltage_label = (TextView) v.findViewById(R.id.apogee_voltage_label);
		apogee_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.apogee_redled),
		                                 (ImageView) v.findViewById(R.id.apogee_greenled),
		                                 getResources());

		main_row = (TableRow) v.findViewById(R.id.main_row);
		main_voltage_view = (TextView) v.findViewById(R.id.main_voltage_value);
		main_voltage_label = (TextView) v.findViewById(R.id.main_voltage_label);
		main_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.main_redled),
		                               (ImageView) v.findViewById(R.id.main_greenled),
		                               getResources());

		data_logging_view = (TextView) v.findViewById(R.id.logging_value);
		data_logging_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.logging_redled),
		                                      (ImageView) v.findViewById(R.id.logging_greenled),
		                                      getResources());

		gps_locked_view = (TextView) v.findViewById(R.id.gps_locked_value);
		gps_locked_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.gps_locked_redled),
		                                    (ImageView) v.findViewById(R.id.gps_locked_greenled),
		                                    getResources());

		gps_ready_view = (TextView) v.findViewById(R.id.gps_ready_value);
		gps_ready_lights = new GoNoGoLights((ImageView) v.findViewById(R.id.gps_ready_redled),
		                                   (ImageView) v.findViewById(R.id.gps_ready_greenled),
		                                   getResources());

		tilt_view 	= (View) v.findViewById(R.id.tilt_view);
		tilt_value 	= (TextView) v.findViewById(R.id.tilt_value);

		for (int i = 0; i < 4; i++) {
			int row_id, view_id, label_id, lights_id;
			int red_id, green_id;
			switch (i) {
			case 0:
			default:
				row_id = R.id.ignite_a_row;
				view_id = R.id.ignite_a_voltage_value;
				label_id = R.id.ignite_a_voltage_label;
				red_id = R.id.ignite_a_redled;
				green_id = R.id.ignite_a_greenled;
				break;
			case 1:
				row_id = R.id.ignite_b_row;
				view_id = R.id.ignite_b_voltage_value;
				label_id = R.id.ignite_b_voltage_label;
				red_id = R.id.ignite_b_redled;
				green_id = R.id.ignite_b_greenled;
				break;
			case 2:
				row_id = R.id.ignite_c_row;
				view_id = R.id.ignite_c_voltage_value;
				label_id = R.id.ignite_c_voltage_label;
				red_id = R.id.ignite_c_redled;
				green_id = R.id.ignite_c_greenled;
				break;
			case 3:
				row_id = R.id.ignite_d_row;
				view_id = R.id.ignite_d_voltage_value;
				label_id = R.id.ignite_d_voltage_label;
				red_id = R.id.ignite_d_redled;
				green_id = R.id.ignite_d_greenled;
				break;
			}
			ignite_row[i] = (TableRow) v.findViewById(row_id);
			ignite_voltage_view[i] = (TextView) v.findViewById(view_id);
			ignite_voltage_label[i] = (TextView) v.findViewById(label_id);
			ignite_lights[i] = new GoNoGoLights((ImageView) v.findViewById(red_id),
							     (ImageView) v.findViewById(green_id),
							     getResources());
		}

		receiver_latitude_view = (TextView) v.findViewById(R.id.receiver_lat_value);
		receiver_longitude_view = (TextView) v.findViewById(R.id.receiver_lon_value);
		receiver_altitude_view = (TextView) v.findViewById(R.id.receiver_alt_value);

		AltosDebug.debug("TabPad onCreateView done battery_voltage_view %s\n", battery_voltage_view);
		return v;
	}

	public String tab_name() { return AltosDroid.tab_pad_name; }

	public void show(TelemetryState telem_state, AltosState state, AltosGreatCircle from_receiver, Location receiver) {
		AltosDebug.debug("pad show state %b bvv %s\n", state != null, battery_voltage_view);
		if (state != null) {
			battery_voltage_view.setText(AltosDroid.number("%1.2f V", state.battery_voltage));
			battery_lights.set(state.battery_voltage >= AltosLib.ao_battery_good, state.battery_voltage == AltosLib.MISSING);
			if (state.apogee_voltage == AltosLib.MISSING) {
				apogee_row.setVisibility(View.GONE);
			} else {
				apogee_voltage_view.setText(AltosDroid.number("%1.2f V", state.apogee_voltage));
				apogee_row.setVisibility(View.VISIBLE);
			}
			apogee_lights.set(state.apogee_voltage >= AltosLib.ao_igniter_good, state.apogee_voltage == AltosLib.MISSING);
			if (state.main_voltage == AltosLib.MISSING) {
				main_row.setVisibility(View.GONE);
			} else {
				main_voltage_view.setText(AltosDroid.number("%1.2f V", state.main_voltage));
				main_row.setVisibility(View.VISIBLE);
			}
			main_lights.set(state.main_voltage >= AltosLib.ao_igniter_good, state.main_voltage == AltosLib.MISSING);

			int num_igniter = state.igniter_voltage == null ? 0 : state.igniter_voltage.length;

			for (int i = 0; i < 4; i++) {
				double voltage = i >= num_igniter ? AltosLib.MISSING : state.igniter_voltage[i];
				if (voltage == AltosLib.MISSING) {
					ignite_row[i].setVisibility(View.GONE);
				} else {
					ignite_voltage_view[i].setText(AltosDroid.number("%1.2f V", voltage));
					ignite_row[i].setVisibility(View.VISIBLE);
				}
				ignite_lights[i].set(voltage >= AltosLib.ao_igniter_good, voltage == AltosLib.MISSING);
			}

			if (state.cal_data().flight != 0) {
				if (state.state() <= AltosLib.ao_flight_pad)
					data_logging_view.setText("Ready to record");
				else if (state.state() < AltosLib.ao_flight_landed)
					data_logging_view.setText("Recording data");
				else
					data_logging_view.setText("Recorded data");
			} else {
				data_logging_view.setText("Storage full");
			}
			data_logging_lights.set(state.cal_data().flight != 0, state.cal_data().flight == AltosLib.MISSING);

			if (state.gps != null) {
				int soln = state.gps.nsat;
				int nsat = state.gps.cc_gps_sat != null ? state.gps.cc_gps_sat.length : 0;
				gps_locked_view.setText(String.format("%d in soln, %d in view", soln, nsat));
				gps_locked_lights.set(state.gps.locked && state.gps.nsat >= 4, false);
				if (state.gps_ready)
					gps_ready_view.setText("Ready");
				else
					gps_ready_view.setText(AltosDroid.integer("Waiting %d", state.gps_waiting));
			} else
				gps_locked_lights.set(false, true);
			gps_ready_lights.set(state.gps_ready, state.gps == null);

			double orient = state.orient();

			if (orient == AltosLib.MISSING) {
				tilt_view.setVisibility(View.GONE);
			} else {
				tilt_value.setText(AltosDroid.number("%1.0f°", orient));
				tilt_view.setVisibility(View.VISIBLE);
			}
		}


		if (telem_state != null) {
			if (telem_state.receiver_battery == AltosLib.MISSING) {
				receiver_row.setVisibility(View.GONE);
			} else {
				receiver_voltage_view.setText(AltosDroid.number("%1.2f V", telem_state.receiver_battery));
				receiver_row.setVisibility(View.VISIBLE);
			}
			receiver_voltage_lights.set(telem_state.receiver_battery >= AltosLib.ao_battery_good, telem_state.receiver_battery == AltosLib.MISSING);
		}

		if (receiver != null) {
			double altitude = AltosLib.MISSING;
			if (receiver.hasAltitude())
				altitude = receiver.getAltitude();
			String lat_text = AltosDroid.pos(receiver.getLatitude(), "N", "S");
			String lon_text = AltosDroid.pos(receiver.getLongitude(), "E", "W");
			AltosDebug.debug("lat %s lon %s\n", lat_text, lon_text);
			receiver_latitude_view.setText(lat_text);
			receiver_longitude_view.setText(lon_text);
			set_value(receiver_altitude_view, AltosConvert.height, 1, altitude);
		}
	}
}
