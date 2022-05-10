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

/*
 * Track flight state from telemetry or eeprom data stream
 */

package org.altusmetrum.altoslib_14;

public class AltosState extends AltosDataListener {

	public static final int set_position = 1;
	public static final int set_gps = 2;
	public static final int set_data = 4;

	public int set;

	static final double filter_len = 2.0;
	static final double ascent_filter_len = 0.5;
	static final double descent_filter_len = 5.0;

	/* derived data */

	public long	received_time;

	public int	rssi;
	public int	status;

	class AltosValue {
		double	value;
		double	prev_value;
		private double	max_value;
		private double	set_time;
		private double	prev_set_time;

		boolean can_max() { return true; }

		void set(double new_value, double time) {
			if (new_value != AltosLib.MISSING) {
				value = new_value;
				if (can_max() && (max_value == AltosLib.MISSING || value > max_value))
					max_value = value;
				set_time = time;
			}
		}

		void set_filtered(double new_value, double time) {
			if (prev_value != AltosLib.MISSING) {
				double f = 1/Math.exp((time - prev_set_time) / filter_len);
				new_value = f * new_value + (1-f) * prev_value;
			}
			set(new_value, time);
		}

		double value() {
			return value;
		}

		double max() {
			return max_value;
		}

		double prev() {
			return prev_value;
		}

		double change() {
			if (value != AltosLib.MISSING && prev_value != AltosLib.MISSING)
				return value - prev_value;
			return AltosLib.MISSING;
		}

		double rate() {
			double c = change();
			double t = set_time - prev_set_time;

			if (c != AltosLib.MISSING && t != 0)
				return c / t;
			return AltosLib.MISSING;
		}

		double integrate() {
			if (value == AltosLib.MISSING)
				return AltosLib.MISSING;
			if (prev_value == AltosLib.MISSING)
				return AltosLib.MISSING;

			return (value + prev_value) / 2 * (set_time - prev_set_time);
		}

		double time() {
			return set_time;
		}

		void set_derivative(AltosValue in) {
			double	n = in.rate();

			if (n == AltosLib.MISSING)
				return;

			double	p = prev_value;
			double	pt = prev_set_time;

			if (p == AltosLib.MISSING) {
				p = 0;
				pt = in.time() - 0.01;
			}

			/* Clip changes to reduce noise */
			double	ddt = in.time() - pt;
			double	ddv = (n - p) / ddt;

			final double max = 100000;

			/* 100gs */
			if (Math.abs(ddv) > max) {
				if (n > p)
					n = p + ddt * max;
				else
					n = p - ddt * max;
			}

			double filter_len;

			if (ascent)
				filter_len = ascent_filter_len;
			else
				filter_len = descent_filter_len;

			double f = 1/Math.exp(ddt/ filter_len);
			n = p * f + n * (1-f);

			set(n, in.time());
		}

		void set_integral(AltosValue in) {
			double	change = in.integrate();

			if (change != AltosLib.MISSING) {
				double	prev = prev_value;
				if (prev == AltosLib.MISSING)
					prev = 0;
				set(prev + change, in.time());
			}
		}

		void copy(AltosValue old) {
			value = old.value;
			set_time = old.set_time;
			prev_value = old.value;
			prev_set_time = old.set_time;
			max_value = old.max_value;
		}

		void finish_update() {
			prev_value = value;
			prev_set_time = set_time;
		}

		AltosValue() {
			value = AltosLib.MISSING;
			prev_value = AltosLib.MISSING;
			max_value = AltosLib.MISSING;
		}

	}

	class AltosCValue {

		class AltosIValue extends AltosValue {
			boolean can_max() {
				return c_can_max();
			}

			AltosIValue() {
				super();
			}
		};

		public AltosIValue	measured;
		public AltosIValue	computed;

		boolean can_max() { return true; }

		boolean c_can_max() { return can_max(); }

		double value() {
			double v = measured.value();
			if (v != AltosLib.MISSING)
				return v;
			return computed.value();
		}

		boolean is_measured() {
			return measured.value() != AltosLib.MISSING;
		}

		double max() {
			double m = measured.max();

			if (m != AltosLib.MISSING)
				return m;
			return computed.max();
		}

		double prev_value() {
			if (measured.value != AltosLib.MISSING && measured.prev_value != AltosLib.MISSING)
				return measured.prev_value;
			return computed.prev_value;
		}

		AltosValue altos_value() {
			if (measured.value() != AltosLib.MISSING)
				return measured;
			return computed;
		}

		double change() {
			double c = measured.change();
			if (c == AltosLib.MISSING)
				c = computed.change();
			return c;
		}

		double rate() {
			double r = measured.rate();
			if (r == AltosLib.MISSING)
				r = computed.rate();
			return r;
		}

		void set_measured(double new_value, double time) {
			measured.set(new_value, time);
		}

		void set_computed(double new_value, double time) {
			computed.set(new_value, time);
		}

		void set_derivative(AltosValue in) {
			computed.set_derivative(in);
		}

		void set_derivative(AltosCValue in) {
			set_derivative(in.altos_value());
		}

		void set_integral(AltosValue in) {
			computed.set_integral(in);
		}

		void set_integral(AltosCValue in) {
			set_integral(in.altos_value());
		}

		void copy(AltosCValue old) {
			measured.copy(old.measured);
			computed.copy(old.computed);
		}

		void finish_update() {
			measured.finish_update();
			computed.finish_update();
		}

		public AltosCValue() {
			measured = new AltosIValue();
			computed = new AltosIValue();
		}
	}

	public boolean	landed;
	public boolean	ascent;	/* going up? */
	public boolean	boost;	/* under power */

	private double pressure_to_altitude(double p) {
		if (p == AltosLib.MISSING)
			return AltosLib.MISSING;
		return AltosConvert.pressure_to_altitude(p);
	}

	private AltosCValue ground_altitude;

	public double ground_altitude() {
		return ground_altitude.value();
	}

	public void set_ground_altitude(double a) {
		ground_altitude.set_measured(a, time);
	}

	class AltosGpsGroundAltitude extends AltosValue {
		void set(double a, double t) {
			super.set(a, t);

			gps_altitude.set_gps_height();
		}

		void set_filtered(double a, double t) {
			super.set_filtered(a, t);
			gps_altitude.set_gps_height();
		}

		AltosGpsGroundAltitude() {
			super();
		}
	}

	private AltosGpsGroundAltitude gps_ground_altitude;

	public double gps_ground_altitude() {
		return gps_ground_altitude.value();
	}

	public void set_gps_ground_altitude(double a) {
		gps_ground_altitude.set(a, time);
	}

	class AltosGroundPressure extends AltosCValue {
		void set_filtered(double p, double time) {
			computed.set_filtered(p, time);
			if (!is_measured())
				ground_altitude.set_computed(pressure_to_altitude(computed.value()), time);
		}

		void set_measured(double p, double time) {
			super.set_measured(p, time);
			ground_altitude.set_computed(pressure_to_altitude(p), time);
		}

		AltosGroundPressure () {
			super();
		}
	}

	private AltosGroundPressure ground_pressure;

	public double ground_pressure() {
		return ground_pressure.value();
	}

	public void set_ground_pressure (double pressure) {
		ground_pressure.set_measured(pressure, time);
	}

	class AltosAltitude extends AltosCValue {

		private void set_speed(AltosValue v) {
			if (!acceleration.is_measured() || !ascent)
				speed.set_derivative(this);
		}

		void set_computed(double a, double time) {
			super.set_computed(a,time);
			set_speed(computed);
			set |= set_position;
		}

		void set_measured(double a, double time) {
			super.set_measured(a,time);
			set_speed(measured);
			set |= set_position;
		}

		AltosAltitude() {
			super();
		}
	}

	private AltosAltitude	altitude;

	class AltosGpsAltitude extends AltosValue {

		private void set_gps_height() {
			double	a = value();
			double	g = gps_ground_altitude.value();

			if (a != AltosLib.MISSING && g != AltosLib.MISSING)
				gps_height = a - g;
			else
				gps_height = AltosLib.MISSING;
		}

		void set(double a, double t) {
			super.set(a, t);
			set_gps_height();
		}

		AltosGpsAltitude() {
			super();
		}
	}

	private AltosGpsAltitude	gps_altitude;

	private AltosValue		gps_ground_speed;
	private AltosValue		gps_ascent_rate;
	private AltosValue		gps_course;
	private AltosValue		gps_speed;

	public double altitude() {
		double a = altitude.value();
		if (a != AltosLib.MISSING)
			return a;
		return gps_altitude.value();
	}

	public double max_altitude() {
		double a = altitude.max();
		if (a != AltosLib.MISSING)
			return a;
		return gps_altitude.max();
	}

	public void set_altitude(double new_altitude) {
		double old_altitude = altitude.value();
		if (old_altitude != AltosLib.MISSING) {
			while (old_altitude - new_altitude > 32000)
				new_altitude += 65536.0;
		}
		altitude.set_measured(new_altitude, time);
	}

	public double gps_altitude() {
		return gps_altitude.value();
	}

	public double max_gps_altitude() {
		return gps_altitude.max();
	}

	public void set_gps_altitude(double new_gps_altitude) {
		gps_altitude.set(new_gps_altitude, time);
	}

	public double gps_ground_speed() {
		return gps_ground_speed.value();
	}

	public double max_gps_ground_speed() {
		return gps_ground_speed.max();
	}

	public double gps_ascent_rate() {
		return gps_ascent_rate.value();
	}

	public double max_gps_ascent_rate() {
		return gps_ascent_rate.max();
	}

	public double gps_course() {
		return gps_course.value();
	}

	public double gps_speed() {
		return gps_speed.value();
	}

	public double max_gps_speed() {
		return gps_speed.max();
	}

	class AltosPressure extends AltosValue {
		void set(double p, double time) {
			super.set(p, time);
			if (state() == AltosLib.ao_flight_pad)
				ground_pressure.set_filtered(p, time);
			double a = pressure_to_altitude(p);
			altitude.set_computed(a, time);
		}

		AltosPressure() {
			super();
		}
	}

	private AltosPressure	pressure;

	public double pressure() {
		return pressure.value();
	}

	public void set_pressure(double p) {
		pressure.set(p, time);
	}

	public void set_thrust(double N) {
	}

	public double baro_height() {
		double a = altitude();
		double g = ground_altitude();
		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return AltosLib.MISSING;
	}

	public double height() {
		double b = baro_height();
		if (b != AltosLib.MISSING)
			return b;

		double k = kalman_height.value();
		if (k != AltosLib.MISSING)
			return k;

		return gps_height();
	}

	public double max_height() {
		double a = altitude.max();
		double g = ground_altitude();
		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;

		double	k = kalman_height.max();
		if (k != AltosLib.MISSING)
			return k;

		return max_gps_height();
	}

	public double gps_height() {
		double a = gps_altitude();
		double g = gps_ground_altitude();

		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return AltosLib.MISSING;
	}

	public double max_gps_height() {
		double a = gps_altitude.max();
		double g = gps_ground_altitude();

		if (a != AltosLib.MISSING && g != AltosLib.MISSING)
			return a - g;
		return AltosLib.MISSING;
	}

	class AltosSpeed extends AltosCValue {

		boolean can_max() {
			return state() < AltosLib.ao_flight_fast || state() == AltosLib.ao_flight_stateless;
		}

		void set_accel() {
			acceleration.set_derivative(this);
		}

		void set_derivative(AltosCValue in) {
			super.set_derivative(in);
			set_accel();
		}

		void set_computed(double new_value, double time) {
			super.set_computed(new_value, time);
			set_accel();
		}

		void set_measured(double new_value, double time) {
			super.set_measured(new_value, time);
			set_accel();
		}

		AltosSpeed() {
			super();
		}
	}

	private AltosSpeed speed;

	public double speed() {
		double v = kalman_speed.value();
		if (v != AltosLib.MISSING)
			return v;
		v = speed.value();
		if (v != AltosLib.MISSING)
			return v;
		v = gps_speed();
		if (v != AltosLib.MISSING)
			return v;
		return AltosLib.MISSING;
	}

	public double max_speed() {
		double v = kalman_speed.max();
		if (v != AltosLib.MISSING)
			return v;
		v = speed.max();
		if (v != AltosLib.MISSING)
			return v;
		v = max_gps_speed();
		if (v != AltosLib.MISSING)
			return v;
		return AltosLib.MISSING;
	}

	class AltosAccel extends AltosCValue {

		boolean can_max() {
			return state() < AltosLib.ao_flight_fast || state() == AltosLib.ao_flight_stateless;
		}

		void set_measured(double a, double time) {
			super.set_measured(a, time);
			if (ascent)
				speed.set_integral(this.measured);
		}

		AltosAccel() {
			super();
		}
	}

	AltosAccel acceleration;

	public double acceleration() {
		return acceleration.value();
	}

	public double max_acceleration() {
		return acceleration.max();
	}

	public AltosCValue	orient;

	public void set_orient(double new_orient) {
		orient.set_measured(new_orient, time);
	}

	public double orient() {
		return orient.value();
	}

	public double max_orient() {
		return orient.max();
	}

	public AltosValue	kalman_height, kalman_speed, kalman_acceleration;

	public void set_kalman(double height, double speed, double acceleration) {
		double old_height = kalman_height.value();
		if (old_height != AltosLib.MISSING) {
			while (old_height - height > 32000)
				height += 65536;
		}
		kalman_height.set(height, time);
		kalman_speed.set(speed, time);
		kalman_acceleration.set(acceleration, time);
	}

	public double	battery_voltage;
	public double	pyro_voltage;
	public double	temperature;
	public double	apogee_voltage;
	public double	main_voltage;

	public double	igniter_voltage[];

	public AltosGPS	gps;
	public boolean	gps_pending;

	public static final int MIN_PAD_SAMPLES = 10;

	public int	npad;
	public int	gps_waiting;
	public boolean	gps_ready;

	public int	ngps;

	public AltosGreatCircle from_pad;
	public double	elevation;	/* from pad */
	public double	distance;	/* distance along ground */
	public double	range;		/* total distance */

	public double	gps_height;

	public double pad_lat, pad_lon;

	public int	speak_tick;
	public double	speak_altitude;

	public double	ground_accel;

	public AltosCompanion	companion;

	public int	pyro_fired;

	public double	motor_pressure;

	public void set_npad(int npad) {
		this.npad = npad;
		gps_waiting = MIN_PAD_SAMPLES - npad;
		if (this.gps_waiting < 0)
			gps_waiting = 0;
		gps_ready = gps_waiting == 0;
	}

	public void init() {
		super.init();

		set = 0;

		received_time = System.currentTimeMillis();
		landed = false;
		boost = false;
		rssi = AltosLib.MISSING;
		status = 0;

		ground_altitude = new AltosCValue();
		ground_pressure = new AltosGroundPressure();
		altitude = new AltosAltitude();
		pressure = new AltosPressure();
		speed = new AltosSpeed();
		acceleration = new AltosAccel();
		orient = new AltosCValue();

		temperature = AltosLib.MISSING;
		battery_voltage = AltosLib.MISSING;
		pyro_voltage = AltosLib.MISSING;
		apogee_voltage = AltosLib.MISSING;
		main_voltage = AltosLib.MISSING;
		igniter_voltage = null;

		kalman_height = new AltosValue();
		kalman_speed = new AltosValue();
		kalman_acceleration = new AltosValue();

		gps = null;
		gps_pending = false;

		last_imu_time = AltosLib.MISSING;
		rotation = null;

		accel_ground_along = AltosLib.MISSING;
		accel_ground_across = AltosLib.MISSING;
		accel_ground_through = AltosLib.MISSING;

		accel_along = AltosLib.MISSING;
		accel_across = AltosLib.MISSING;
		accel_through = AltosLib.MISSING;

		gyro_roll = AltosLib.MISSING;
		gyro_pitch = AltosLib.MISSING;
		gyro_yaw = AltosLib.MISSING;

		mag_along = AltosLib.MISSING;
		mag_across = AltosLib.MISSING;
		mag_through = AltosLib.MISSING;

		set_npad(0);
		ngps = 0;

		from_pad = null;
		elevation = AltosLib.MISSING;
		distance = AltosLib.MISSING;
		range = AltosLib.MISSING;
		gps_height = AltosLib.MISSING;

		pad_lat = AltosLib.MISSING;
		pad_lon = AltosLib.MISSING;

		gps_altitude = new AltosGpsAltitude();
		gps_ground_altitude = new AltosGpsGroundAltitude();
		gps_ground_speed = new AltosValue();
		gps_speed = new AltosValue();
		gps_ascent_rate = new AltosValue();
		gps_course = new AltosValue();

		speak_tick = AltosLib.MISSING;
		speak_altitude = AltosLib.MISSING;

		ground_accel = AltosLib.MISSING;

		companion = null;

		pyro_fired = 0;
	}

	void finish_update() {
		ground_altitude.finish_update();
		altitude.finish_update();
		pressure.finish_update();
		speed.finish_update();
		acceleration.finish_update();
		orient.finish_update();

		kalman_height.finish_update();
		kalman_speed.finish_update();
		kalman_acceleration.finish_update();
	}

	void update_time() {
	}

	void update_gps() {
		elevation = AltosLib.MISSING;
		distance = AltosLib.MISSING;
		range = AltosLib.MISSING;

		if (gps == null)
			return;

		if (gps.locked && gps.nsat >= 4) {
			/* Track consecutive 'good' gps reports, waiting for 10 of them */
			if (state() == AltosLib.ao_flight_pad || state() == AltosLib.ao_flight_stateless) {
				set_npad(npad+1);
				if (pad_lat != AltosLib.MISSING && (npad < 10 || state() == AltosLib.ao_flight_pad)) {
					pad_lat = (pad_lat * 31 + gps.lat) / 32;
					pad_lon = (pad_lon * 31 + gps.lon) / 32;
					gps_ground_altitude.set_filtered(gps.alt, time);
				}
			}
			if (pad_lat == AltosLib.MISSING) {
				pad_lat = gps.lat;
				pad_lon = gps.lon;
				gps_ground_altitude.set(gps.alt, time);
			}
			gps_altitude.set(gps.alt, time);
			if (gps.climb_rate != AltosLib.MISSING)
				gps_ascent_rate.set(gps.climb_rate, time);
			if (gps.ground_speed != AltosLib.MISSING)
				gps_ground_speed.set(gps.ground_speed, time);
			if (gps.climb_rate != AltosLib.MISSING && gps.ground_speed != AltosLib.MISSING)
				gps_speed.set(Math.sqrt(gps.ground_speed * gps.ground_speed +
							gps.climb_rate * gps.climb_rate), time);
			if (gps.course != AltosLib.MISSING)
				gps_course.set(gps.course, time);
		} else if (state() == AltosLib.ao_flight_pad || state() == AltosLib.ao_flight_stateless) {
			set_npad(0);
		}
		if (gps.lat != 0 && gps.lon != 0 &&
		    pad_lat != AltosLib.MISSING &&
		    pad_lon != AltosLib.MISSING)
		{
			double h = height();

			if (h == AltosLib.MISSING)
				h = 0;
			from_pad = new AltosGreatCircle(pad_lat, pad_lon, 0, gps.lat, gps.lon, h);
			elevation = from_pad.elevation;
			distance = from_pad.distance;
			range = from_pad.range;
		}
	}

	public void set_state(int state) {
		super.set_state(state);
		ascent = (AltosLib.ao_flight_boost <= state() &&
			  state() <= AltosLib.ao_flight_coast);
		boost = (AltosLib.ao_flight_boost == state());
	}

	public int rssi() {
		if (rssi == AltosLib.MISSING)
			return 0;
		return rssi;
	}

	public void set_rssi(int rssi, int status) {
		if (rssi != AltosLib.MISSING) {
			this.rssi = rssi;
			this.status = status;
		}
	}

	public void set_received_time(long ms) {
		received_time = ms;
	}

	public void set_gps(AltosGPS gps, boolean set_location, boolean set_sats) {
		super.set_gps(gps, set_location, set_sats);
		if (gps != null) {
			this.gps = gps;
			update_gps();
			set |= set_gps;
		}
	}

	public AltosRotation	rotation;

	public double	accel_ground_along, accel_ground_across, accel_ground_through;

	void update_pad_rotation() {
		if (cal_data().pad_orientation != AltosLib.MISSING && accel_ground_along != AltosLib.MISSING) {
			rotation = new AltosRotation(accel_ground_across,
						     accel_ground_through,
						     accel_ground_along,
						     cal_data().pad_orientation);
			orient.set_computed(rotation.tilt(), time);
		}
	}

	public void set_accel_ground(double ground_along, double ground_across, double ground_through) {
		accel_ground_along = ground_along;
		accel_ground_across = ground_across;
		accel_ground_through = ground_through;
		update_pad_rotation();
	}

	public double	last_imu_time;

	private void update_orient() {
		if (last_imu_time != AltosLib.MISSING) {
			double	t = time - last_imu_time;

			if (t > 0 && gyro_pitch != AltosLib.MISSING && rotation != null) {
				double	pitch = AltosConvert.degrees_to_radians(gyro_pitch) * t;
				double	yaw = AltosConvert.degrees_to_radians(gyro_yaw) * t;
				double	roll = AltosConvert.degrees_to_radians(gyro_roll) * t;

				rotation.rotate(pitch, yaw, roll);
				orient.set_computed(rotation.tilt(), time);
			}
		}
		last_imu_time = time;
	}

	private double	gyro_roll, gyro_pitch, gyro_yaw;

	public void set_gyro(double roll, double pitch, double yaw) {
		gyro_roll = roll;
		gyro_pitch = pitch;
		gyro_yaw = yaw;
		update_orient();
	}

	private double accel_along, accel_across, accel_through;

	public void set_accel(double along, double across, double through) {
		accel_along = along;
		accel_across = across;
		accel_through = through;
		update_orient();
	}

	public double accel_along() {
		return accel_along;
	}

	public double accel_across() {
		return accel_across;
	}

	public double accel_through() {
		return accel_through;
	}

	public double gyro_roll() {
		return gyro_roll;
	}

	public double gyro_pitch() {
		return gyro_pitch;
	}

	public double gyro_yaw() {
		return gyro_yaw;
	}

	private double mag_along, mag_across, mag_through;

	public void set_mag(double along, double across, double through) {
		mag_along = along;
		mag_across = across;
		mag_through = through;
	}

	public double mag_along() {
		return mag_along;
	}

	public double mag_across() {
		return mag_across;
	}

	public double mag_through() {
		return mag_through;
	}

	public void set_companion(AltosCompanion companion) {
		this.companion = companion;
	}

	public void set_acceleration(double acceleration) {
		if (acceleration != AltosLib.MISSING) {
			this.acceleration.set_measured(acceleration, time);
			set |= set_data;
		}
	}

	public void set_temperature(double temperature) {
		if (temperature != AltosLib.MISSING) {
			this.temperature = temperature;
			set |= set_data;
		}
	}

	public void set_battery_voltage(double battery_voltage) {
		if (battery_voltage != AltosLib.MISSING) {
			this.battery_voltage = battery_voltage;
			set |= set_data;
		}
	}

	public void set_pyro_voltage(double pyro_voltage) {
		if (pyro_voltage != AltosLib.MISSING) {
			this.pyro_voltage = pyro_voltage;
			set |= set_data;
		}
	}

	public void set_apogee_voltage(double apogee_voltage) {
		if (apogee_voltage != AltosLib.MISSING) {
			this.apogee_voltage = apogee_voltage;
			set |= set_data;
		}
	}

	public void set_main_voltage(double main_voltage) {
		if (main_voltage != AltosLib.MISSING) {
			this.main_voltage = main_voltage;
			set |= set_data;
		}
	}

	public void set_igniter_voltage(double[] voltage) {
		this.igniter_voltage = voltage;
	}

	public void set_pyro_fired(int fired) {
		this.pyro_fired = fired;
	}

	public void set_motor_pressure(double motor_pressure) {
		this.motor_pressure = motor_pressure;
	}

	public AltosState() {
		init();
	}

	public AltosState (AltosCalData cal_data) {
		super(cal_data);
		init();
	}
}
