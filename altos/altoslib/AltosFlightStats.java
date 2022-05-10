/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

public class AltosFlightStats {
	public double		max_height;
	public double		max_gps_height;
	public double		max_speed;
	public double		max_acceleration;
	public double[]		state_speed = new double[AltosLib.ao_flight_invalid + 1];
	public double[]		state_enter_speed = new double[AltosLib.ao_flight_invalid + 1];
	public double[]		state_enter_height = new double[AltosLib.ao_flight_invalid + 1];
	public double[]		state_enter_gps_height = new double[AltosLib.ao_flight_invalid + 1];
	public double[]		state_accel = new double[AltosLib.ao_flight_invalid + 1];
	public double[]		state_time = new double[AltosLib.ao_flight_invalid + 1];
	public String		product;
	public String		firmware_version;
	public int		serial;
	public int		flight;
	public int		year, month, day;
	public int		hour, minute, second;
	public double		boost_time;
	public double		landed_time;
	public double		lat, lon;
	public double		pad_lat, pad_lon;
	public boolean		has_flight_data;
	public boolean		has_gps;
	public boolean		has_gps_sats;
	public boolean		has_gps_detail;
	public boolean		has_flight_adc;
	public boolean		has_battery;
	public boolean		has_rssi;
	public boolean		has_imu;
	public boolean		has_mag;
	public boolean		has_orient;
	public int		num_igniter;

	double landed_time(AltosFlightSeries series) {
		double	landed_state_time = AltosLib.MISSING;

		double	prev_state_time = AltosLib.MISSING;
		if (series.state_series != null) {
			for (AltosTimeValue state : series.state_series) {
				if (state.value == AltosLib.ao_flight_landed) {
					landed_state_time = state.time;
					break;
				} else {
					prev_state_time = state.time;
				}
			}
		}

		if (landed_state_time == AltosLib.MISSING && series.height_series != null)
			landed_state_time = series.height_series.get(series.height_series.size()-1).time;

		double landed_height = AltosLib.MISSING;

		if (series.height_series != null) {
			for (AltosTimeValue height : series.height_series) {
				landed_height = height.value;
				if (height.time >= landed_state_time)
					break;
			}
		}

		if (landed_height == AltosLib.MISSING)
			return AltosLib.MISSING;

		boolean	above = true;

		double	landed_time = AltosLib.MISSING;

		if (series.height_series != null) {
			for (AltosTimeValue height : series.height_series) {
				if (height.value > landed_height + 10) {
					above = true;
				} else {
					if (above && Math.abs(height.value - landed_height) < 2) {
						above = false;
						landed_time = height.time;
					}
				}
			}
		}

		if (landed_time == AltosLib.MISSING || (prev_state_time != AltosLib.MISSING && landed_time < prev_state_time))
			landed_time = landed_state_time;
		return landed_time;
	}

	double boost_time(AltosFlightSeries series) {
		double 		boost_time = AltosLib.MISSING;
		double		boost_state_time = AltosLib.MISSING;

		if (series.state_series != null) {
			for (AltosTimeValue state : series.state_series) {
				if (state.value >= AltosLib.ao_flight_boost && state.value <= AltosLib.ao_flight_landed) {
					boost_state_time = state.time;
					break;
				}
			}
		}
		if (series.accel_series != null) {
			for (AltosTimeValue accel : series.accel_series) {
				if (accel.value < 1)
					boost_time = accel.time;
				if (boost_state_time != AltosLib.MISSING && accel.time >= boost_state_time)
					break;
			}
		}
		if (boost_time == AltosLib.MISSING)
			boost_time = boost_state_time;
		return boost_time;
	}

	private void add_times(AltosFlightSeries series, int state, double start_time, double end_time) {
		double delta_time = end_time - start_time;
		if (0 <= state && state <= AltosLib.ao_flight_invalid && delta_time > 0) {
			if (state_enter_speed[state] == AltosLib.MISSING)
				state_enter_speed[state] = series.speed_series.value(start_time);
			if (state_enter_height[state] == AltosLib.MISSING)
				state_enter_height[state] = series.height_series.value(start_time);
			if (state_enter_gps_height[state] == AltosLib.MISSING)
				if (series.gps_height != null)
					state_enter_gps_height[state] = series.gps_height.value(start_time);
			speeds[state].value += series.speed_series.average(start_time, end_time) * delta_time;
			speeds[state].time += delta_time;
			accels[state].value += series.accel_series.average(start_time, end_time) * delta_time;
			accels[state].time += delta_time;
			state_time[state] += delta_time;

			if (state == AltosLib.ao_flight_boost) {
				AltosTimeValue tv_speed = series.speed_series.max(start_time, end_time);
				if (tv_speed != null && (max_speed == AltosLib.MISSING || tv_speed.value > max_speed))
					max_speed = tv_speed.value;
				AltosTimeValue tv_accel = series.accel_series.max(start_time, end_time);
				if (tv_accel != null && (max_acceleration == AltosLib.MISSING || tv_accel.value > max_acceleration))
					max_acceleration = tv_accel.value;
			}
		}
	}

	AltosTimeValue[]	speeds = new AltosTimeValue[AltosLib.ao_flight_invalid + 1];
	AltosTimeValue[]	accels = new AltosTimeValue[AltosLib.ao_flight_invalid + 1];

	public AltosFlightStats(AltosFlightSeries series) {
		AltosCalData	cal_data = series.cal_data();

		series.finish();

		boost_time = boost_time(series);
		landed_time = landed_time(series);

		if (series.state_series != null){
			boolean fixed_boost = false;
			boolean fixed_landed = false;
			for (AltosTimeValue state : series.state_series) {
				if ((int) state.value == AltosLib.ao_flight_boost)
					if (boost_time != AltosLib.MISSING && !fixed_boost) {
						state.time = boost_time;
						fixed_boost = true;
					}
				if ((int) state.value == AltosLib.ao_flight_landed)
					if (landed_time != AltosLib.MISSING && !fixed_landed) {
						state.time = landed_time;
						fixed_landed = true;
					}
			}
		}

		year = month = day = AltosLib.MISSING;
		hour = minute = second = AltosLib.MISSING;
		serial = flight = AltosLib.MISSING;
		lat = lon = AltosLib.MISSING;
		has_flight_data = false;
		has_gps = false;
		has_gps_sats = false;
		has_flight_adc = false;
		has_battery = false;
		has_rssi = false;
		has_imu = false;
		has_mag = false;
		has_orient = false;

		for (int s = 0; s < AltosLib.ao_flight_invalid + 1; s++) {
			state_speed[s] = AltosLib.MISSING;
			state_enter_speed[s] = AltosLib.MISSING;
			state_accel[s] = AltosLib.MISSING;
			state_time[s] = 0;
			speeds[s] = new AltosTimeValue(0, 0);
			accels[s] = new AltosTimeValue(0, 0);
		}

		max_speed = AltosLib.MISSING;
		max_acceleration = AltosLib.MISSING;

		if (series.state_series != null) {
			AltosTimeValue prev = null;
			for (AltosTimeValue state : series.state_series) {
				if (prev != null)
					add_times(series, (int) prev.value, prev.time, state.time);
				prev = state;
			}
			if (prev != null) {
				AltosTimeValue last_accel = series.accel_series.last();
				if (last_accel != null)
					add_times(series, (int) prev.value, prev.time, last_accel.time);
			}
		}

		for (int s = 0; s <= AltosLib.ao_flight_invalid; s++) {
			if (speeds[s].time > 0)
				state_speed[s] = speeds[s].value / speeds[s].time;
			if (accels[s].time > 0)
				state_accel[s] = accels[s].value / accels[s].time;
		}

		product = cal_data.product;
		firmware_version = cal_data.firmware_version;
		serial = cal_data.serial;
		flight = cal_data.flight;

		has_battery = series.battery_voltage_series != null;
		has_flight_adc = series.main_voltage_series != null;
		has_rssi = series.rssi_series != null;
		has_flight_data = series.pressure_series != null;

		AltosGPS gps = series.cal_data().gps_pad;

		if (gps != null) {
			year = gps.year;
			month = gps.month;
			day = gps.day;
			hour = gps.hour;
			minute = gps.minute;
			second = gps.second;
			has_gps = true;
			lat = pad_lat = gps.lat;
			lon = pad_lon = gps.lon;
			if (series.gps_series != null) {
				for (AltosGPSTimeValue gtv : series.gps_series) {
					gps = gtv.gps;
					if (gps.locked && gps.nsat >= 4) {
						lat = gps.lat;
						lon = gps.lon;
					}
				}
			}
		}

		max_height = series.max_height;
		if (max_height == AltosLib.MISSING && series.height_series != null)
			max_height = series.height_series.max().value;
		max_gps_height = AltosLib.MISSING;
		if (series.gps_height != null) {
			AltosTimeValue tv = series.gps_height.max();
			if (tv != null)
				max_gps_height = tv.value;
		}
	}
}
