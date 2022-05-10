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

public class AltosCSV implements AltosWriter {
	File			name;
	PrintStream		out;
	boolean			header_written;
	boolean			seen_boost;
	int			boost_tick;

	boolean			has_call;
	boolean			has_basic;
	boolean			has_accel;
	boolean			has_baro;
	boolean			has_pyro;
	boolean			has_radio;
	boolean			has_battery;
	boolean			has_flight_state;
	boolean			has_3d_accel;
	boolean			has_imu;
	boolean			has_igniter;
	boolean			has_gps;
	boolean			has_gps_sat;
	boolean			has_companion;
	boolean			has_motor_pressure;

	AltosFlightSeries	series;
	int[]			indices;

	static final int ALTOS_CSV_VERSION = 6;

	/* Version 4 format:
	 *
	 * General info
	 *	version number
	 *	serial number
	 *	flight number
	 *	callsign
	 *	time (seconds since boost)
	 *
	 * Radio info (if available)
	 *	rssi
	 *	link quality
	 *
	 * Flight status
	 *	state
	 *	state name
	 *
	 * Basic sensors
	 *	acceleration (m/s²)
	 *	pressure (mBar)
	 *	altitude (m)
	 *	height (m)
	 *	accelerometer speed (m/s)
	 *	barometer speed (m/s)
	 *	temp (°C)
	 *	drogue (V)
	 *	main (V)
	 *
	 * Battery
	 *	battery (V)
	 *
	 * Advanced sensors (if available)
	 *	accel_x (m/s²)
	 *	accel_y (m/s²)
	 *	accel_z (m/s²)
	 *	gyro_x (d/s)
	 *	gyro_y (d/s)
	 *	gyro_z (d/s)
	 *	mag_x (g)
	 *	mag_y (g)
	 *	mag_z (g)
	 *	tilt (d)
	 *
	 * Extra igniter voltages (if available)
	 *	pyro (V)
	 *	igniter_a (V)
	 *	igniter_b (V)
	 *	igniter_c (V)
	 *	igniter_d (V)
	 *
	 * GPS data (if available)
	 *	connected (1/0)
	 *	locked (1/0)
	 *	nsat (used for solution)
	 *	latitude (°)
	 *	longitude (°)
	 *	altitude (m)
	 *	year (e.g. 2010)
	 *	month (1-12)
	 *	day (1-31)
	 *	hour (0-23)
	 *	minute (0-59)
	 *	second (0-59)
	 *	from_pad_dist (m)
	 *	from_pad_azimuth (deg true)
	 *	from_pad_range (m)
	 *	from_pad_elevation (deg from horizon)
	 *	pdop
	 *	hdop
	 *	vdop
	 *
	 * GPS Sat data
	 *	C/N0 data for all 32 valid SDIDs
	 *
	 * Companion data
	 *	companion_id (1-255. 10 is TeleScience)
	 *	time of last companion data (seconds since boost)
	 *	update_period (0.1-2.55 minimum telemetry interval)
	 *	channels (0-12)
	 *	channel data for all 12 possible channels
	 */

	void write_general_header() {
		out.printf("version,serial,flight");
		if (series.cal_data().callsign != null)
			out.printf(",call");
		out.printf(",time");
	}

	double time() {
		return series.time(indices);
	}

	void write_general() {
		out.printf("%s, %d, %d",
			   ALTOS_CSV_VERSION,
			   series.cal_data().serial,
			   series.cal_data().flight);
		if (series.cal_data().callsign != null)
			out.printf(",%s", series.cal_data().callsign);
		out.printf(", %8.2f", time());
	}

	void write_radio_header() {
		out.printf("rssi,lqi");
	}

	int rssi() {
		return (int) series.value(AltosFlightSeries.rssi_name, indices);
	}

	int status() {
		return (int) series.value(AltosFlightSeries.status_name, indices);
	}

	void write_radio() {
		out.printf("%4d, %3d",
			   rssi(), status() & 0x7f);
	}

	void write_flight_header() {
		out.printf("state,state_name");
	}

	int state() {
		return (int) series.value(AltosFlightSeries.state_name, indices);
	}

	void write_flight() {
		int state = state();
		out.printf("%2d,%8s", state, AltosLib.state_name(state));
	}

	void write_basic_header() {
		if (has_accel)
			out.printf("acceleration,");
		if (has_baro)
			out.printf("pressure,altitude,");
		out.printf("height,speed");
		if (has_baro)
			out.printf(",temperature");
		if (has_pyro)
			out.printf(",drogue_voltage,main_voltage");
	}

	double acceleration() { return series.value(AltosFlightSeries.accel_name, indices); }
	double pressure() { return series.value(AltosFlightSeries.pressure_name, indices); }
	double altitude() { return series.value(AltosFlightSeries.altitude_name, indices); }
	double height() { return series.value(AltosFlightSeries.height_name, indices); }
	double speed() { return series.value(AltosFlightSeries.speed_name, indices); }
	double temperature() { return series.value(AltosFlightSeries.temperature_name, indices); }
	double apogee_voltage() { return series.value(AltosFlightSeries.apogee_voltage_name, indices); }
	double main_voltage() { return series.value(AltosFlightSeries.main_voltage_name, indices); }

	void write_basic() {
		if (has_accel)
			out.printf("%8.2f,", acceleration());
		if (has_baro)
			out.printf("%10.2f,%8.2f,",
				   pressure(), altitude());
		out.printf("%8.2f,%8.2f",
			   height(), speed());
		if (has_baro)
			out.printf(",%5.1f", temperature());
		if (has_pyro)
			out.printf(",%5.2f,%5.2f",
				   apogee_voltage(),
				   main_voltage());
	}

	void write_battery_header() {
		out.printf("battery_voltage");
	}

	double battery_voltage() { return series.value(AltosFlightSeries.battery_voltage_name, indices); }

	void write_battery() {
		out.printf("%5.2f", battery_voltage());
	}

	void write_motor_pressure_header() {
		out.printf("motor_pressure");
	}

	double motor_pressure() { return series.value(AltosFlightSeries.motor_pressure_name, indices); }

	void write_motor_pressure() {
		out.printf("%10.1f", motor_pressure());
	}

	void write_3d_accel_header() {
		out.printf("accel_x,accel_y,accel_z");
	}

	double accel_along() { return series.value(AltosFlightSeries.accel_along_name, indices); }
	double accel_across() { return series.value(AltosFlightSeries.accel_across_name, indices); }
	double accel_through() { return series.value(AltosFlightSeries.accel_through_name, indices); }

	void write_3d_accel() {
		out.printf("%7.2f,%7.2f,%7.2f",
			   accel_along(), accel_across(), accel_through());
	}

	void write_imu_header() {
		out.printf("gyro_roll,gyro_pitch,gyro_yaw,mag_x,mag_y,mag_z,tilt");
	}

	double gyro_roll() { return series.value(AltosFlightSeries.gyro_roll_name, indices); }
	double gyro_pitch() { return series.value(AltosFlightSeries.gyro_pitch_name, indices); }
	double gyro_yaw() { return series.value(AltosFlightSeries.gyro_yaw_name, indices); }

	double mag_along() { return series.value(AltosFlightSeries.mag_along_name, indices); }
	double mag_across() { return series.value(AltosFlightSeries.mag_across_name, indices); }
	double mag_through() { return series.value(AltosFlightSeries.mag_through_name, indices); }

	double tilt() { return series.value(AltosFlightSeries.orient_name, indices); }

	void write_imu() {
		out.printf("%7.2f,%7.2f,%7.2f,%7.2f,%7.2f,%7.2f,%7.2f",
			   gyro_roll(), gyro_pitch(), gyro_yaw(),
			   mag_along(), mag_across(), mag_through(),
			   tilt());
	}

	void write_igniter_header() {
		out.printf("pyro");
		for (int i = 0; i < series.igniter_voltage.length; i++)
			out.printf(",%s", AltosLib.igniter_short_name(i));
	}

	double pyro() { return series.value(AltosFlightSeries.pyro_voltage_name, indices); }

	double igniter_value(int channel) { return series.value(series.igniter_voltage_name(channel), indices);	}

	void write_igniter() {
		out.printf("%5.2f", pyro());
		for (int i = 0; i < series.igniter_voltage.length; i++)
			out.printf(",%5.2f", igniter_value(i));
	}

	void write_gps_header() {
		out.printf("connected,locked,nsat,latitude,longitude,altitude,year,month,day,hour,minute,second,pad_dist,pad_range,pad_az,pad_el,pdop,hdop,vdop");
	}

	void write_gps() {
		AltosGPS	gps = series.gps_before(series.time(indices));

		AltosGreatCircle from_pad;

		if (series.cal_data().gps_pad != null && gps != null)
			from_pad = new AltosGreatCircle(series.cal_data().gps_pad, gps);
		else
			from_pad = new AltosGreatCircle();

		if (gps == null)
			gps = new AltosGPS();

		out.printf("%2d,%2d,%3d,%12.7f,%12.7f,%8.1f,%5d,%3d,%3d,%3d,%3d,%3d,%9.0f,%9.0f,%4.0f,%4.0f,%6.1f,%6.1f,%6.1f",
			   gps.connected?1:0,
			   gps.locked?1:0,
			   gps.nsat,
			   gps.lat,
			   gps.lon,
			   gps.alt,
			   gps.year,
			   gps.month,
			   gps.day,
			   gps.hour,
			   gps.minute,
			   gps.second,
			   from_pad.distance,
			   from_pad.range,
			   from_pad.bearing,
			   from_pad.elevation,
			   gps.pdop,
			   gps.hdop,
			   gps.vdop);
	}

	void write_gps_sat_header() {
		for(int i = 1; i <= 32; i++) {
			out.printf("sat%02d", i);
			if (i != 32)
				out.printf(",");
		}
	}

	void write_gps_sat() {
		AltosGPS	gps = series.gps_before(series.time(indices));
		for(int i = 1; i <= 32; i++) {
			int	c_n0 = 0;
			if (gps != null && gps.cc_gps_sat != null) {
				for(int j = 0; j < gps.cc_gps_sat.length; j++)
					if (gps.cc_gps_sat[j].svid == i) {
						c_n0 = gps.cc_gps_sat[j].c_n0;
						break;
					}
			}
			out.printf ("%3d", c_n0);
			if (i != 32)
				out.printf(",");
		}
	}

	void write_companion_header() {
/*
		out.printf("companion_id,companion_time,companion_update,companion_channels");
		for (int i = 0; i < 12; i++)
			out.printf(",companion_%02d", i);
*/
	}

	void write_companion() {
/*
		AltosCompanion companion = state.companion;

		int	channels_written = 0;
		if (companion == null) {
			out.printf("0,0,0,0");
		} else {
			out.printf("%3d,%5.2f,%5.2f,%2d",
				   companion.board_id,
				   (companion.tick - boost_tick) / 100.0,
				   companion.update_period / 100.0,
				   companion.channels);
			for (; channels_written < companion.channels; channels_written++)
				out.printf(",%5d", companion.companion_data[channels_written]);
		}
		for (; channels_written < 12; channels_written++)
			out.printf(",0");
*/
	}

	void write_header() {
		out.printf("#"); write_general_header();
		if (has_radio) {
			out.printf(",");
			write_radio_header();
		}
		if (has_flight_state) {
			out.printf(",");
			write_flight_header();
		}
		if (has_basic) {
			out.printf(",");
			write_basic_header();
		}
		if (has_battery) {
			out.printf(",");
			write_battery_header();
		}
		if (has_motor_pressure) {
			out.printf(",");
			write_motor_pressure_header();
		}
		if (has_3d_accel) {
			out.printf(",");
			write_3d_accel_header();
		}
		if (has_imu) {
			out.printf(",");
			write_imu_header();
		}
		if (has_igniter) {
			out.printf(",");
			write_igniter_header();
		}
		if (has_gps) {
			out.printf(",");
			write_gps_header();
		}
		if (has_gps_sat) {
			out.printf(",");
			write_gps_sat_header();
		}
		if (has_companion) {
			out.printf(",");
			write_companion_header();
		}
		out.printf ("\n");
	}

	void write_one() {
		write_general();
		if (has_radio) {
			out.printf(",");
			write_radio();
		}
		if (has_flight_state) {
			out.printf(",");
			write_flight();
		}
		if (has_basic) {
			out.printf(",");
			write_basic();
		}
		if (has_battery) {
			out.printf(",");
			write_battery();
		}
		if (has_motor_pressure) {
			out.printf(",");
			write_motor_pressure();
		}
		if (has_3d_accel) {
			out.printf(",");
			write_3d_accel();
		}
		if (has_imu) {
			out.printf(",");
			write_imu();
		}
		if (has_igniter) {
			out.printf(",");
			write_igniter();
		}
		if (has_gps) {
			out.printf(",");
			write_gps();
		}
		if (has_gps_sat) {
			out.printf(",");
			write_gps_sat();
		}
		if (has_companion) {
			out.printf(",");
			write_companion();
		}
		out.printf ("\n");
	}

	private void write() {
		if (state() == AltosLib.ao_flight_startup)
			return;
		if (!header_written) {
			write_header();
			header_written = true;
		}
		write_one();
	}

	private PrintStream out() {
		return out;
	}

	public void close() {
		out.close();
	}

	public void write(AltosFlightSeries series) {
//		series.write_comments(out());

		this.series = series;

		series.finish();

		has_radio = false;
		has_flight_state = false;
		has_basic = false;
		has_accel = false;
		has_baro = false;
		has_pyro = false;
		has_battery = false;
		has_motor_pressure = false;
		has_3d_accel = false;
		has_imu = false;
		has_igniter = false;
		has_gps = false;
		has_gps_sat = false;
		has_companion = false;

		if (series.has_series(AltosFlightSeries.rssi_name))
			has_radio = true;
		if (series.has_series(AltosFlightSeries.state_name))
			has_flight_state = true;
		if (series.has_series(AltosFlightSeries.accel_name)) {
			has_basic = true;
			has_accel = true;
		}
		if (series.has_series(AltosFlightSeries.pressure_name)) {
			has_basic = true;
			has_baro = true;
		}
		if (series.has_series(AltosFlightSeries.apogee_voltage_name))
			has_pyro = true;
		if (series.has_series(AltosFlightSeries.battery_voltage_name))
			has_battery = true;
		if (series.has_series(AltosFlightSeries.motor_pressure_name))
			has_motor_pressure = true;
		if (series.has_series(AltosFlightSeries.accel_across_name))
			has_3d_accel = true;
		if (series.has_series(AltosFlightSeries.gyro_roll_name))
			has_imu = true;
		if (series.has_series(AltosFlightSeries.pyro_voltage_name))
			has_igniter = true;

		if (series.gps_series != null)
			has_gps = true;
		if (series.sats_in_view != null)
			has_gps_sat = true;
		/*
		if (state.companion != null)
			has_companion = true;
		*/

		indices = series.indices();

		for (;;) {
			write();
			if (!series.step_indices(indices))
				break;
		}
	}

	public AltosCSV(PrintStream in_out, File in_name) {
		name = in_name;
		out = in_out;
	}

	public AltosCSV(File in_name) throws FileNotFoundException {
		this(new PrintStream(in_name), in_name);
	}

	public AltosCSV(String in_string) throws FileNotFoundException {
		this(new File(in_string));
	}
}
