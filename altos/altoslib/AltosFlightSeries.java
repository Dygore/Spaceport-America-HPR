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

import java.util.*;

public class AltosFlightSeries extends AltosDataListener {

	public ArrayList<AltosTimeSeries> series = new ArrayList<AltosTimeSeries>();

	public double	speed_filter_width = 4.0;
	public double	accel_filter_width = 1.0;

	public int[] indices() {
		int[] indices = new int[series.size()];
		for (int i = 0; i < indices.length; i++)
			indices[i] = -1;
		step_indices(indices);
		return indices;
	}

	private double time(int id, int index) {
		AltosTimeSeries		s = series.get(id);

		if (index < 0)
			return Double.NEGATIVE_INFINITY;

		if (index < s.values.size())
			return s.values.get(index).time;
		return Double.POSITIVE_INFINITY;
	}

	public boolean step_indices(int[] indices) {
		double	min_next = time(0, indices[0]+1);

		for (int i = 1; i < indices.length; i++) {
			double next = time(i, indices[i]+1);
			if (next < min_next)
				min_next = next;
		}

		if (min_next == Double.POSITIVE_INFINITY)
			return false;

		for (int i = 0; i < indices.length; i++) {
			double	t = time(i, indices[i] + 1);

			if (t <= min_next)
				indices[i]++;
		}
		return true;
	}

	public double time(int[] indices) {
		double max = time(0, indices[0]);

		for (int i = 1; i < indices.length; i++) {
			double t = time(i, indices[i]);
			if (t >= max)
				max = t;
		}
		return max;
	}

	public double value(String name, int[] indices) {
		for (int i = 0; i < indices.length; i++) {
			AltosTimeSeries	s = series.get(i);
			if (s.label.equals(name)) {
				int index = indices[i];
				if (index < 0)
					index = 0;
				if (index >= s.values.size())
					index = s.values.size() - 1;
				return s.values.get(index).value;
			}
		}
		return AltosLib.MISSING;
	}

	public double value(String name, double time) {
		for (AltosTimeSeries s : series) {
			if (s.label.equals(name))
				return s.value(time);
		}
		return AltosLib.MISSING;
	}

	public double value_before(String name, double time) {
		for (AltosTimeSeries s : series) {
			if (s.label.equals(name))
				return s.value_before(time);
		}
		return AltosLib.MISSING;
	}

	public double value_after(String name, double time) {
		for (AltosTimeSeries s : series) {
			if (s.label.equals(name))
				return s.value_after(time);
		}
		return AltosLib.MISSING;
	}

	public AltosTimeSeries make_series(String label, AltosUnits units) {
		return new AltosTimeSeries(label, units);
	}

	public void add_series(AltosTimeSeries s) {
		for (int e = 0; e < series.size(); e++) {
			if (s.compareTo(series.get(e)) < 0){
				series.add(e, s);
				return;
			}
		}
		series.add(s);
	}

	public AltosTimeSeries add_series(String label, AltosUnits units) {
		AltosTimeSeries s = make_series(label, units);
		add_series(s);
		return s;
	}

	public void remove_series(AltosTimeSeries s) {
		series.remove(s);
	}

	public boolean has_series(String label) {
		for (AltosTimeSeries s : series)
			if (s.label.equals(label))
				return true;
		return false;
	}

	public AltosTimeSeries state_series;

	public static final String state_name = "State";

	public void set_state(int state) {

		if (state != AltosLib.ao_flight_pad && state != AltosLib.MISSING && state != AltosLib.ao_flight_stateless) {
			if (state_series == null)
				state_series = add_series(state_name, AltosConvert.state_name);
			if (this.state() != state)
				state_series.add(time(), state);
		}
		super.set_state(state);
	}

	public AltosTimeSeries	accel_series;
	public boolean		accel_computed;

	public static final String accel_name = "Accel";

	public AltosTimeSeries	vert_accel_series;

	public static final String vert_accel_name = "Vertical Accel";

	public void set_acceleration(double acceleration) {
		if (acceleration == AltosLib.MISSING)
			return;
		if (accel_series == null)
			accel_series = add_series(accel_name, AltosConvert.accel);

		accel_series.add(time(), acceleration);
		accel_computed = false;
	}

	private AltosTimeSeries compute_accel() {
		AltosTimeSeries	new_accel_series = null;

		if (speed_series != null) {
			AltosTimeSeries temp_series;
			if (accel_filter_width > 0) {
				temp_series = make_series(speed_name, AltosConvert.speed);
				speed_series.filter(temp_series, accel_filter_width);
			} else
				temp_series = speed_series;

			new_accel_series = make_series(accel_name, AltosConvert.accel);
			temp_series.differentiate(new_accel_series);
		}
		return new_accel_series;
	}

	public void set_filter(double speed_filter, double accel_filter) {
		this.speed_filter_width = speed_filter;
		this.accel_filter_width = accel_filter;

		AltosTimeSeries new_speed_series = compute_speed();

		if (new_speed_series != null) {
			speed_series.erase_values();
			for (AltosTimeValue tv : new_speed_series)
				speed_series.add(tv);
		}
		if (accel_computed) {
			AltosTimeSeries new_accel_series = compute_accel();
			if (new_accel_series != null) {
				accel_series.erase_values();
				for (AltosTimeValue tv : new_accel_series)
					accel_series.add(tv);
			}
		}
	}

	public void set_received_time(long received_time) {
	}

	public AltosTimeSeries tick_series;

	public static final String tick_name = "Tick";

	public void set_tick(int tick) {
		super.set_tick(tick);
		if (tick_series == null)
			tick_series = add_series(tick_name, null);
		tick_series.add(time(), tick);
	}

	public AltosTimeSeries rssi_series;

	public static final String rssi_name = "RSSI";

	public AltosTimeSeries status_series;

	public static final String status_name = "Radio Status";

	public void set_rssi(int rssi, int status) {
		if (rssi_series == null) {
			rssi_series = add_series(rssi_name, null);
			status_series = add_series(status_name, null);
		}
		rssi_series.add(time(), rssi);
		status_series.add(time(), status);
	}

	public AltosTimeSeries pressure_series;

	public static final String pressure_name = "Pressure";

	public AltosTimeSeries altitude_series;

	public static final String altitude_name = "Altitude";

	public AltosTimeSeries height_series;

	public double max_height = AltosLib.MISSING;

	public	void set_min_pressure(double pa) {
		double ground_altitude = cal_data().ground_altitude;
		if (ground_altitude != AltosLib.MISSING)
			max_height = AltosConvert.pressure_to_altitude(pa) -
				ground_altitude;
	}

	public static final String height_name = "Height";

	public  void set_pressure(double pa) {
		if (pa == AltosLib.MISSING)
			return;

		if (pressure_series == null)
			pressure_series = add_series(pressure_name, AltosConvert.pressure);
		pressure_series.add(time(), pa);
		if (altitude_series == null)
			altitude_series = add_series(altitude_name, AltosConvert.height);

		if (cal_data().ground_pressure == AltosLib.MISSING)
			cal_data().set_ground_pressure(pa);

		double altitude = AltosConvert.pressure_to_altitude(pa);
		altitude_series.add(time(), altitude);
	}

	private void compute_height() {
		if (height_series == null) {
			double ground_altitude = cal_data().ground_altitude;
			if (ground_altitude != AltosLib.MISSING && altitude_series != null) {
				height_series = add_series(height_name, AltosConvert.height);
				for (AltosTimeValue alt : altitude_series)
					height_series.add(alt.time, alt.value - ground_altitude);
			} else if (speed_series != null) {
				height_series = add_series(height_name, AltosConvert.height);
				speed_series.integrate(height_series);
			}
		}

		if (gps_height == null && cal_data().gps_pad != null && cal_data().gps_pad.alt != AltosLib.MISSING && gps_altitude != null) {
			double gps_ground_altitude = cal_data().gps_pad.alt;
			gps_height = add_series(gps_height_name, AltosConvert.height);
			for (AltosTimeValue gps_alt : gps_altitude)
				gps_height.add(gps_alt.time, gps_alt.value - gps_ground_altitude);
		}
	}

	public AltosTimeSeries speed_series;

	public static final String speed_name = "Speed";

	private AltosTimeSeries compute_speed() {
		AltosTimeSeries new_speed_series = null;
		AltosTimeSeries	alt_speed_series = null;
		AltosTimeSeries accel_speed_series = null;

		if (altitude_series != null) {
			AltosTimeSeries temp_series;

			if (speed_filter_width > 0) {
				temp_series = make_series(speed_name, AltosConvert.height);
				altitude_series.filter(temp_series, speed_filter_width);
			} else
				temp_series = altitude_series;

			alt_speed_series = make_series(speed_name, AltosConvert.speed);
			temp_series.differentiate(alt_speed_series);
		}
		if (accel_series != null && !accel_computed) {

			if (orient_series != null) {
				vert_accel_series = add_series(vert_accel_name, AltosConvert.accel);

				for (AltosTimeValue a : accel_series) {
					double	orient = orient_series.value(a.time);
					double	a_abs = a.value + AltosConvert.gravity;
					double	v_a = a_abs * Math.cos(AltosConvert.degrees_to_radians(orient)) - AltosConvert.gravity;

					vert_accel_series.add(a.time, v_a);
				}
			}

			AltosTimeSeries temp_series = make_series(speed_name, AltosConvert.speed);

			if (vert_accel_series != null)
				vert_accel_series.integrate(temp_series);
			else
				accel_series.integrate(temp_series);

			AltosTimeSeries clip_series = make_series(speed_name, AltosConvert.speed);

			temp_series.clip(clip_series, 0, Double.POSITIVE_INFINITY);

			accel_speed_series = make_series(speed_name, AltosConvert.speed);
			clip_series.filter(accel_speed_series, 0.1);
		}

		if (alt_speed_series != null && accel_speed_series != null) {
			double	apogee_time = AltosLib.MISSING;
			if (state_series != null) {
				for (AltosTimeValue d : state_series) {
					if (d.value >= AltosLib.ao_flight_drogue){
						apogee_time = d.time;
						break;
					}
				}
			}
			if (apogee_time == AltosLib.MISSING) {
				new_speed_series = alt_speed_series;
			} else {
				new_speed_series = make_series(speed_name, AltosConvert.speed);
				for (AltosTimeValue d : accel_speed_series) {
					if (d.time <= apogee_time)
						new_speed_series.add(d);
				}
				for (AltosTimeValue d : alt_speed_series) {
					if (d.time > apogee_time)
						new_speed_series.add(d);
				}

			}
		} else if (alt_speed_series != null) {
			new_speed_series = alt_speed_series;
		} else if (accel_speed_series != null) {
			new_speed_series = accel_speed_series;
		}
		return new_speed_series;
	}

	public AltosTimeSeries orient_series;
	public AltosTimeSeries azimuth_series;

	public static final String orient_name = "Tilt Angle";
	public static final String azimuth_name = "Azimuth Angle";

	private void compute_orient() {

		if (orient_series != null)
			return;

		if (accel_ground_across == AltosLib.MISSING)
			return;

		AltosCalData cal_data = cal_data();

		if (cal_data.pad_orientation == AltosLib.MISSING)
			return;

		if (cal_data.accel_zero_across == AltosLib.MISSING)
			return;

		if (cal_data.gyro_zero_roll == AltosLib.MISSING)
			return;

		AltosRotation rotation = new AltosRotation(accel_ground_across,
							   accel_ground_through,
							   accel_ground_along,
							   cal_data.pad_orientation);
		double prev_time = ground_time;

		orient_series = add_series(orient_name, AltosConvert.orient);
		orient_series.add(ground_time, rotation.tilt());

		azimuth_series = add_series(azimuth_name, AltosConvert.orient);
		azimuth_series.add(ground_time, rotation.azimuth());

		for (AltosTimeValue roll_v : gyro_roll) {
			double	time = roll_v.time;
			double	dt = time - prev_time;

			if (dt > 0) {
				double	roll = AltosConvert.degrees_to_radians(roll_v.value) * dt;
				double	pitch = AltosConvert.degrees_to_radians(gyro_pitch.value(time)) * dt;
				double	yaw = AltosConvert.degrees_to_radians(gyro_yaw.value(time)) * dt;

				rotation.rotate(pitch, yaw, roll);
				orient_series.add(time, rotation.tilt());
				azimuth_series.add(time, rotation.azimuth());
			}
			prev_time = time;
		}
	}

	public AltosTimeSeries	kalman_height_series, kalman_speed_series, kalman_accel_series;

	public static final String kalman_height_name = "Kalman Height";
	public static final String kalman_speed_name = "Kalman Speed";
	public static final String kalman_accel_name = "Kalman Accel";

	public void set_kalman(double height, double speed, double acceleration) {
		if (kalman_height_series == null) {
			kalman_height_series = add_series(kalman_height_name, AltosConvert.height);
			kalman_speed_series = add_series(kalman_speed_name, AltosConvert.speed);
			kalman_accel_series = add_series(kalman_accel_name, AltosConvert.accel);
		}
		kalman_height_series.add(time(), height);
		kalman_speed_series.add(time(), speed);
		kalman_accel_series.add(time(), acceleration);
	}

	public AltosTimeSeries thrust_series;

	public static final String thrust_name = "Thrust";

	public	void set_thrust(double N) {
		if (thrust_series == null)
			thrust_series = add_series(thrust_name, AltosConvert.force);
		thrust_series.add(time(), N);
	}

	public AltosTimeSeries temperature_series;

	public static final String temperature_name = "Temperature";

	public  void set_temperature(double deg_c) {
		if (temperature_series == null)
			temperature_series = add_series(temperature_name, AltosConvert.temperature);
		temperature_series.add(time(), deg_c);
	}

	public AltosTimeSeries battery_voltage_series;

	public static final String battery_voltage_name = "Battery Voltage";

	public void set_battery_voltage(double volts) {
		if (volts == AltosLib.MISSING)
			return;
		if (battery_voltage_series == null)
			battery_voltage_series = add_series(battery_voltage_name, AltosConvert.voltage);
		battery_voltage_series.add(time(), volts);
	}

	public AltosTimeSeries apogee_voltage_series;

	public static final String apogee_voltage_name = "Apogee Voltage";

	public void set_apogee_voltage(double volts) {
		if (volts == AltosLib.MISSING)
			return;
		if (apogee_voltage_series == null)
			apogee_voltage_series = add_series(apogee_voltage_name, AltosConvert.voltage);
		apogee_voltage_series.add(time(), volts);
	}

	public AltosTimeSeries main_voltage_series;

	public static final String main_voltage_name = "Main Voltage";

	public void set_main_voltage(double volts) {
		if (volts == AltosLib.MISSING)
			return;
		if (main_voltage_series == null)
			main_voltage_series = add_series(main_voltage_name, AltosConvert.voltage);
		main_voltage_series.add(time(), volts);
	}

	public ArrayList<AltosGPSTimeValue> gps_series;

	public AltosGPS gps_before(double time) {
		AltosGPSTimeValue nearest = null;
		for (AltosGPSTimeValue gtv : gps_series) {
			if (nearest == null)
				nearest = gtv;
			else {
				if (gtv.time <= time) {
					if (nearest.time <= time && gtv.time > nearest.time)
						nearest = gtv;
				} else {
					if (nearest.time > time && gtv.time < nearest.time)
						nearest = gtv;
				}
			}
		}
		if (nearest != null)
			return nearest.gps;
		else
			return null;
	}

	public AltosTimeSeries	sats_in_view;
	public AltosTimeSeries sats_in_soln;
	public AltosTimeSeries gps_altitude;
	public AltosTimeSeries gps_height;
	public AltosTimeSeries gps_ground_speed;
	public AltosTimeSeries gps_ascent_rate;
	public AltosTimeSeries gps_course;
	public AltosTimeSeries gps_speed;
	public AltosTimeSeries gps_pdop, gps_vdop, gps_hdop;

	public static final String sats_in_view_name = "Satellites in view";
	public static final String sats_in_soln_name = "Satellites in solution";
	public static final String gps_altitude_name = "GPS Altitude";
	public static final String gps_height_name = "GPS Height";
	public static final String gps_ground_speed_name = "GPS Ground Speed";
	public static final String gps_ascent_rate_name = "GPS Ascent Rate";
	public static final String gps_course_name = "GPS Course";
	public static final String gps_speed_name = "GPS Speed";
	public static final String gps_pdop_name = "GPS Dilution of Precision";
	public static final String gps_vdop_name = "GPS Vertical Dilution of Precision";
	public static final String gps_hdop_name = "GPS Horizontal Dilution of Precision";

	public void set_gps(AltosGPS gps, boolean new_location, boolean new_sats) {
		super.set_gps(gps, new_location, new_sats);
		AltosCalData cal_data = cal_data();
		if (gps_series == null)
			gps_series = new ArrayList<AltosGPSTimeValue>();
		gps_series.add(new AltosGPSTimeValue(time(), gps));

		if (new_location) {
			if (sats_in_soln == null) {
				sats_in_soln = add_series(sats_in_soln_name, null);
			}
			sats_in_soln.add(time(), gps.nsat);
			if (gps.pdop != AltosLib.MISSING) {
				if (gps_pdop == null)
					gps_pdop = add_series(gps_pdop_name, null);
				gps_pdop.add(time(), gps.pdop);
			}
			if (gps.hdop != AltosLib.MISSING) {
				if (gps_hdop == null)
					gps_hdop = add_series(gps_hdop_name, null);
				gps_hdop.add(time(), gps.hdop);
			}
			if (gps.vdop != AltosLib.MISSING) {
				if (gps_vdop == null)
					gps_vdop = add_series(gps_vdop_name, null);
				gps_vdop.add(time(), gps.vdop);
			}
			if (gps.locked) {
				if (gps.alt != AltosLib.MISSING) {
					if (gps_altitude == null)
						gps_altitude = add_series(gps_altitude_name, AltosConvert.height);
					gps_altitude.add(time(), gps.alt);
				}
				if (gps.ground_speed != AltosLib.MISSING) {
					if (gps_ground_speed == null)
						gps_ground_speed = add_series(gps_ground_speed_name, AltosConvert.speed);
					gps_ground_speed.add(time(), gps.ground_speed);
				}
				if (gps.climb_rate != AltosLib.MISSING) {
					if (gps_ascent_rate == null)
						gps_ascent_rate = add_series(gps_ascent_rate_name, AltosConvert.speed);
					gps_ascent_rate.add(time(), gps.climb_rate);
				}
				if (gps.course != AltosLib.MISSING) {
					if (gps_course == null)
						gps_course = add_series(gps_course_name, null);
					gps_course.add(time(), gps.course);
				}
				if (gps.ground_speed != AltosLib.MISSING && gps.climb_rate != AltosLib.MISSING) {
					if (gps_speed == null)
						gps_speed = add_series(gps_speed_name, null);
					gps_speed.add(time(), Math.sqrt(gps.ground_speed * gps.ground_speed +
									gps.climb_rate * gps.climb_rate));
				}
			}
		}
		if (new_sats) {
			if (gps.cc_gps_sat != null) {
				if (sats_in_view == null)
					sats_in_view = add_series(sats_in_view_name, null);
				sats_in_view.add(time(), gps.cc_gps_sat.length);
			}
		}
	}

	public static final String accel_along_name = "Accel Along";
	public static final String accel_across_name = "Accel Across";
	public static final String accel_through_name = "Accel Through";

	public AltosTimeSeries accel_along, accel_across, accel_through;

	public static final String gyro_roll_name = "Roll Rate";
	public static final String gyro_pitch_name = "Pitch Rate";
	public static final String gyro_yaw_name = "Yaw Rate";

	public AltosTimeSeries gyro_roll, gyro_pitch, gyro_yaw;

	public static final String mag_along_name = "Magnetic Field Along";
	public static final String mag_across_name = "Magnetic Field Across";
	public static final String mag_through_name = "Magnetic Field Through";
	public static final String mag_total_name = "Magnetic Field Strength";
	public static final String compass_name = "Compass";

	public AltosTimeSeries mag_along, mag_across, mag_through, mag_total, compass;

	public  void set_accel(double along, double across, double through) {
		if (accel_along == null) {
			accel_along = add_series(accel_along_name, AltosConvert.accel);
			accel_across = add_series(accel_across_name, AltosConvert.accel);
			accel_through = add_series(accel_through_name, AltosConvert.accel);
		}
		accel_along.add(time(), along);
		accel_across.add(time(), across);
		accel_through.add(time(), through);
	}

	private double	accel_ground_along = AltosLib.MISSING;
	private double	accel_ground_across = AltosLib.MISSING;
	private double	accel_ground_through = AltosLib.MISSING;

	private double		ground_time;

	public  void set_accel_ground(double along, double across, double through) {
		accel_ground_along = along;
		accel_ground_across = across;
		accel_ground_through = through;
		ground_time = time();
	}

	public  void set_gyro(double roll, double pitch, double yaw) {
		if (gyro_roll == null) {
			gyro_roll = add_series(gyro_roll_name, AltosConvert.rotation_rate);
			gyro_pitch = add_series(gyro_pitch_name, AltosConvert.rotation_rate);
			gyro_yaw = add_series(gyro_yaw_name, AltosConvert.rotation_rate);
		}
		gyro_roll.add(time(), roll);
		gyro_pitch.add(time(), pitch);
		gyro_yaw.add(time(), yaw);
	}

	public  void set_mag(double along, double across, double through) {
		if (mag_along == null) {
			mag_along = add_series(mag_along_name, AltosConvert.magnetic_field);
			mag_across = add_series(mag_across_name, AltosConvert.magnetic_field);
			mag_through = add_series(mag_through_name, AltosConvert.magnetic_field);
			mag_total = add_series(mag_total_name, AltosConvert.magnetic_field);
			compass = add_series(compass_name, AltosConvert.orient);
		}
		mag_along.add(time(), along);
		mag_across.add(time(), across);
		mag_through.add(time(), through);
		mag_total.add(time(), Math.sqrt(along * along + across * across + through *through));
		compass.add(time(), Math.atan2(across, through) * 180 / Math.PI);
	}

	public void set_orient(double orient) {
		if (orient_series == null)
			orient_series = add_series(orient_name, AltosConvert.orient);
		orient_series.add(time(), orient);
	}

	public static final String pyro_voltage_name = "Pyro Voltage";

	public AltosTimeSeries pyro_voltage;

	public  void set_pyro_voltage(double volts) {
		if (pyro_voltage == null)
			pyro_voltage = add_series(pyro_voltage_name, AltosConvert.voltage);
		pyro_voltage.add(time(), volts);
	}

	private static String[] igniter_voltage_names;

	public String igniter_voltage_name(int channel) {
		if (igniter_voltage_names == null || igniter_voltage_names.length <= channel) {
			String[] new_igniter_voltage_names = new String[channel + 1];
			int	i = 0;

			if (igniter_voltage_names != null) {
				for (; i < igniter_voltage_names.length; i++)
					new_igniter_voltage_names[i] = igniter_voltage_names[i];
			}
			for (; i < channel+1; i++)
				new_igniter_voltage_names[i] = AltosLib.igniter_name(i);
			igniter_voltage_names = new_igniter_voltage_names;
		}
		return igniter_voltage_names[channel];
	}

	public AltosTimeSeries[] igniter_voltage;

	public  void set_igniter_voltage(double[] voltage) {
		int channels = voltage.length;
		if (igniter_voltage == null || igniter_voltage.length <= channels) {
			AltosTimeSeries[]	new_igniter_voltage = new AltosTimeSeries[channels];
			int			i = 0;

			if (igniter_voltage != null) {
				for (; i < igniter_voltage.length; i++)
					new_igniter_voltage[i] = igniter_voltage[i];
			}
			for (; i < channels; i++)
				new_igniter_voltage[i] = add_series(igniter_voltage_name(i), AltosConvert.voltage);
			igniter_voltage = new_igniter_voltage;
		}
		for (int channel = 0; channel < voltage.length; channel++)
			igniter_voltage[channel].add(time(), voltage[channel]);
	}

	public static final String pyro_fired_name = "Pyro Channel State";

	public AltosTimeSeries pyro_fired_series;

	int	last_pyro_mask;

	public  void set_pyro_fired(int pyro_mask) {
		if (pyro_fired_series == null)
			pyro_fired_series = add_series(pyro_fired_name, AltosConvert.pyro_name);
		for (int channel = 0; channel < 32; channel++) {
			if ((last_pyro_mask & (1 << channel)) == 0 &&
			    (pyro_mask & (1 << channel)) != 0) {
				pyro_fired_series.add(time(), channel);
			}
		}
		last_pyro_mask = pyro_mask;
	}

	public void set_companion(AltosCompanion companion) {
	}

	public static final String motor_pressure_name = "Motor Pressure";

	public AltosTimeSeries motor_pressure_series;

	public void set_motor_pressure(double motor_pressure) {
		if (motor_pressure_series == null)
			motor_pressure_series = add_series(motor_pressure_name, AltosConvert.pressure);
		motor_pressure_series.add(time(), motor_pressure);
	}

	public void finish() {
		compute_orient();
		if (speed_series == null) {
			speed_series = compute_speed();
			if (speed_series != null)
				add_series(speed_series);
		}
		if (accel_series == null) {
			accel_series = compute_accel();
			if (accel_series != null) {
				add_series(accel_series);
				accel_computed = true;
			}
		}
		compute_height();
	}

	public AltosTimeSeries[] series() {
		finish();
		return series.toArray(new AltosTimeSeries[0]);
	}

	public AltosFlightSeries(AltosCalData cal_data) {
		super(cal_data);
	}
}
