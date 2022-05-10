/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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
import java.util.concurrent.*;

/*
 * Open an existing telemetry file and replay it in realtime
 */

class AltosReplay extends AltosDataListener implements Runnable {

	AltosState	state;
	AltosRecordSet	record_set;
	double		last_time = AltosLib.MISSING;
	Semaphore	semaphore = new Semaphore(1);
	boolean		done = false;

	public void set_time(double time) {
		if (time > -2) {
			if (last_time != AltosLib.MISSING) {
				semaphore.release();
				double	delay = Math.min(time - last_time,10);
				if (delay > 0) {
					try {
						Thread.sleep((int) (delay * 1000));
					} catch (InterruptedException ie) {
					}
				}
			}
		}
		last_time = time;
		super.set_time(time);
		state.set_time(time);
	}

	public void set_state(int state) {
		super.set_state(state);
		this.state.set_state(state);
	}

	public void set_rssi(int rssi, int status) { state.set_rssi(rssi, status); }
	public void set_received_time(long received_time) { }

	public void set_acceleration(double accel) { state.set_acceleration(accel); }
	public void set_pressure(double pa) { state.set_pressure(pa); }
	public void set_thrust(double N) { state.set_thrust(N); }

	public void set_kalman(double height, double speed, double accel) { state.set_kalman(height, speed, accel); }

	public void set_temperature(double deg_c) { state.set_temperature(deg_c); }
	public void set_battery_voltage(double volts) { state.set_battery_voltage(volts); }

	public void set_apogee_voltage(double volts) { state.set_apogee_voltage(volts); }
	public void set_main_voltage(double volts) { state.set_main_voltage(volts); }

	public void set_gps(AltosGPS gps, boolean set_location, boolean set_sats) {
		super.set_gps(gps, set_location, set_sats);
		state.set_gps(gps, set_location, set_sats);
	}

	public void set_orient(double orient) { state.set_orient(orient); }
	public void set_gyro(double roll, double pitch, double yaw) { state.set_gyro(roll, pitch, yaw); }
	public void set_accel_ground(double along, double across, double through) { state.set_accel_ground(along, across, through); }
	public void set_accel(double along, double across, double through) { state.set_accel(along, across, through); }
	public void set_mag(double along, double across, double through) { state.set_mag(along, across, through); }
	public void set_pyro_voltage(double volts) { state.set_pyro_voltage(volts); }
	public void set_igniter_voltage(double[] voltage) { state.set_igniter_voltage(voltage); }
	public void set_pyro_fired(int pyro_mask) { state.set_pyro_fired(pyro_mask); }
	public void set_companion(AltosCompanion companion) { state.set_companion(companion); }
	public void set_motor_pressure(double motor_pressure) { state.set_motor_pressure(motor_pressure); }

	public void run () {
		/* Run the flight */
		record_set.capture_series(this);
		/* All done, signal that it's over */
		done = true;
		semaphore.release();
	}

	public AltosReplay(AltosRecordSet record_set) {
		super(record_set.cal_data());
		state = new AltosState(record_set.cal_data());
		this.record_set = record_set;
		try {
			semaphore.acquire();
		} catch (InterruptedException ie) {
		}
	}
}

public class AltosReplayReader extends AltosFlightReader {
	File		file;
	AltosReplay	replay;
	Thread		t;

	public AltosCalData cal_data() {
		return replay.state.cal_data();
	}

	public AltosState read() {
		/* Wait for something to change */
		try {
			replay.semaphore.acquire();
		} catch (InterruptedException ie) {
		}

		/* When done, let the display know */
		if (replay.done)
			return null;

		/* Fake out the received time */
		replay.state.set_received_time(System.currentTimeMillis());
		return replay.state;
	}

	public void close (boolean interrupted) {
	}

	public File backing_file() { return file; }

	public AltosReplayReader(AltosRecordSet record_set, File in_file) {
		file = in_file;
		name = file.getName();
		replay = new AltosReplay(record_set);
		t = new Thread(replay);
		t.start();
	}
}
