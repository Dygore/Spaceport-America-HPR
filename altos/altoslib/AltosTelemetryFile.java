/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
import java.text.*;

class AltosTelemetryNullListener extends AltosDataListener {
	public void set_rssi(int rssi, int status) { }
	public void set_received_time(long received_time) { }

	public void set_acceleration(double accel) { }
	public void set_pressure(double pa) { }
	public void set_thrust(double N) { }

	public void set_kalman(double height, double speed, double accel) { }

	public void set_temperature(double deg_c) { }
	public void set_battery_voltage(double volts) { }

	public void set_apogee_voltage(double volts) { }
	public void set_main_voltage(double volts) { }

	public void set_gps(AltosGPS gps) { }

	public void set_orient(double orient) { }
	public void set_gyro(double roll, double pitch, double yaw) { }
	public void set_accel_ground(double along, double across, double through) { }
	public void set_accel(double along, double across, double through) { }
	public void set_mag(double along, double across, double through) { }
	public void set_pyro_voltage(double volts) { }
	public void set_igniter_voltage(double[] voltage) { }
	public void set_pyro_fired(int pyro_mask) { }
	public void set_companion(AltosCompanion companion) { }
	public void set_motor_pressure(double motor_pressure) { }

	public boolean cal_data_complete() {
		/* All telemetry packets */
		AltosCalData cal_data = cal_data();

		if (cal_data.serial == AltosLib.MISSING)
			return false;

		if (cal_data.boost_tick == AltosLib.MISSING)
			return false;

		/*
		 * TelemetryConfiguration:
		 *
		 * device_type, flight, config version, log max,
		 * flight params, callsign and version
		 */
		if (cal_data.device_type == AltosLib.MISSING)
			return false;

		/*
		 * TelemetrySensor or TelemetryMegaData:
		 *
		 * ground_accel, accel+/-, ground pressure
		 */
		if (cal_data.ground_pressure == AltosLib.MISSING)
			return false;

		/*
		 * TelemetryLocation
		 */
		if (AltosLib.has_gps(cal_data.device_type) && cal_data.gps_pad == null)
			return false;

		return true;
	}

	public AltosTelemetryNullListener(AltosCalData cal_data) {
		super(cal_data);
	}
}

public class AltosTelemetryFile implements AltosRecordSet {

	AltosTelemetryIterable	telems;
	AltosCalData		cal_data;
	int			first_state;

	public void write_comments(PrintStream out) {
	}

	public void write(PrintStream out) {
	}

	/* Construct cal data by walking through the telemetry data until we've found everything available */
	public AltosCalData cal_data() {
		if (cal_data == null) {
			cal_data = new AltosCalData();
			AltosTelemetryNullListener l = new AltosTelemetryNullListener(cal_data);

			first_state = AltosLib.ao_flight_startup;
			for (AltosTelemetry telem : telems) {
				telem.provide_data(l);
				if (cal_data.state == AltosLib.ao_flight_pad)
					first_state = cal_data.state;
				if (l.cal_data_complete())
					break;
			}
		}
		return cal_data;
	}

	public boolean valid() {
		return true;
	}

	public void capture_series(AltosDataListener listener) {
		cal_data();
		cal_data.reset();
		cal_data.state = first_state;
		for (AltosTelemetry telem : telems) {
			telem.provide_data(listener);
			if (listener.state() == AltosLib.ao_flight_landed)
				break;
		}
		listener.finish();
	}

	public AltosTelemetryFile(FileInputStream input) throws IOException {
		telems = new AltosTelemetryIterable(input);
	}
}
