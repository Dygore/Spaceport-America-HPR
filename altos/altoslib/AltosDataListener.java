/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

package org.altusmetrum.altoslib_14;

public abstract class AltosDataListener {

	private AltosCalData	cal_data = null;

	public double		time = AltosLib.MISSING;
	public double		frequency = AltosLib.MISSING;

	public int		raw_tick = AltosLib.MISSING;

	public int tick() {
		return raw_tick;
	}

	public void set_tick(int tick) {
		raw_tick = tick;
		cal_data.set_tick(tick);
		set_time(cal_data.time());
	}

	public AltosCalData cal_data() {
		if (cal_data == null)
			cal_data = new AltosCalData();
		return cal_data;
	}

	public void set_time(double time) {
		if (time != AltosLib.MISSING)
			this.time = time;
	}

	public void set_serial(int serial) {
		cal_data().set_serial(serial);
	}

	public void set_device_type(int device_type) {
		cal_data().set_device_type(device_type);
		switch (device_type) {
		case AltosLib.product_telegps:
			set_state(AltosLib.ao_flight_stateless);
			break;
		}
	}

	public void set_log_format(int log_format) {
		cal_data().set_log_format(log_format);
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEGPS:
			set_state(AltosLib.ao_flight_stateless);
			break;
		}
	}

	public double time() {
		return time;
	}

	public String state_name() {
		return cal_data().state_name();
	}

	public void set_state(int state) {
		cal_data().set_state(state);
	}

	public int state() {
		return cal_data().state;
	}

	public void set_flight(int flight) {
		cal_data().set_flight(flight);
	}

	public void set_frequency(double frequency) {
		this.frequency = frequency;
	}

	public void set_avoid_duplicate_files() {
	}

	/* Called after all records are captured */
	public void finish() {
	}

	public void init() {
		set_state(AltosLib.ao_flight_invalid);
		time = AltosLib.MISSING;
		frequency = AltosLib.MISSING;
	}

	public abstract void set_rssi(int rssi, int status);
	public abstract void set_received_time(long received_time);

	public abstract void set_acceleration(double accel);
	public abstract void set_pressure(double pa);
	public abstract void set_thrust(double N);

	public abstract void set_kalman(double height, double speed, double accel);

	public abstract void set_temperature(double deg_c);
	public abstract void set_battery_voltage(double volts);

	public abstract void set_apogee_voltage(double volts);
	public abstract void set_main_voltage(double volts);

	public void set_gps(AltosGPS gps, boolean set_location, boolean set_sats) {
		AltosCalData cal_data = cal_data();
		cal_data.set_cal_gps(gps);
	}

	public AltosGPS make_temp_gps(boolean sats) {
		return cal_data().make_temp_cal_gps(tick(), sats);
	}

	public AltosGPS temp_gps() {
		return cal_data().temp_cal_gps();
	}

	public abstract void set_orient(double orient);
	public abstract void set_gyro(double roll, double pitch, double yaw);
	public abstract void set_accel_ground(double along, double across, double through);
	public abstract void set_accel(double along, double across, double through);
	public abstract void set_mag(double along, double across, double through);
	public abstract void set_pyro_voltage(double volts);
	public abstract void set_igniter_voltage(double[] voltage);
	public abstract void set_pyro_fired(int pyro_mask);
	public abstract void set_companion(AltosCompanion companion);
	public abstract void set_motor_pressure(double motor_pressure);

	public AltosDataListener() {
	}

	public AltosDataListener(AltosCalData cal_data) {
		this.cal_data = cal_data;
	}
}
