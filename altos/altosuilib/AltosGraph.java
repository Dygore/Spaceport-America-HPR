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

package org.altusmetrum.altosuilib_14;

import java.io.*;
import java.util.ArrayList;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;

import org.jfree.ui.*;
import org.jfree.chart.*;
import org.jfree.chart.plot.*;
import org.jfree.chart.axis.*;
import org.jfree.chart.renderer.*;
import org.jfree.chart.renderer.xy.*;
import org.jfree.chart.labels.*;
import org.jfree.data.xy.*;
import org.jfree.data.*;

public class AltosGraph extends AltosUIGraph {

	/* These are in 'priority' order so that earlier ones get simpler line styles,
	 * then they are grouped so that adjacent ones get sequential colors
	 */
	static final private AltosUILineStyle height_color = new AltosUILineStyle();
	static final private AltosUILineStyle speed_color = new AltosUILineStyle();
	static final private AltosUILineStyle accel_color = new AltosUILineStyle();
	static final private AltosUILineStyle vert_accel_color = new AltosUILineStyle();
	static final private AltosUILineStyle orient_color = new AltosUILineStyle();
	static final private AltosUILineStyle azimuth_color = new AltosUILineStyle();
	static final private AltosUILineStyle compass_color = new AltosUILineStyle();

	static final private AltosUILineStyle gps_height_color = new AltosUILineStyle();
	static final private AltosUILineStyle altitude_color = new AltosUILineStyle();

	static final private AltosUILineStyle battery_voltage_color = new AltosUILineStyle();
	static final private AltosUILineStyle pyro_voltage_color = new AltosUILineStyle();
	static final private AltosUILineStyle drogue_voltage_color = new AltosUILineStyle();
	static final private AltosUILineStyle main_voltage_color = new AltosUILineStyle();
	static final private AltosUILineStyle igniter_marker_color = new AltosUILineStyle(1);

	static final private AltosUILineStyle kalman_height_color = new AltosUILineStyle();
	static final private AltosUILineStyle kalman_speed_color = new AltosUILineStyle();
	static final private AltosUILineStyle kalman_accel_color = new AltosUILineStyle();

	static final private AltosUILineStyle gps_nsat_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_nsat_solution_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_nsat_view_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_course_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_ground_speed_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_speed_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_climb_rate_color = new AltosUILineStyle ();
	static final private AltosUILineStyle gps_pdop_color = new AltosUILineStyle();
	static final private AltosUILineStyle gps_hdop_color = new AltosUILineStyle();
	static final private AltosUILineStyle gps_vdop_color = new AltosUILineStyle();

	static final private AltosUILineStyle temperature_color = new AltosUILineStyle ();
	static final private AltosUILineStyle dbm_color = new AltosUILineStyle();
	static final private AltosUILineStyle pressure_color = new AltosUILineStyle ();

	static final private AltosUILineStyle state_color = new AltosUILineStyle(0);
	static final private AltosUILineStyle accel_along_color = new AltosUILineStyle();
	static final private AltosUILineStyle accel_across_color = new AltosUILineStyle();
	static final private AltosUILineStyle accel_through_color = new AltosUILineStyle();
	static final private AltosUILineStyle gyro_roll_color = new AltosUILineStyle();
	static final private AltosUILineStyle gyro_pitch_color = new AltosUILineStyle();
	static final private AltosUILineStyle gyro_yaw_color = new AltosUILineStyle();
	static final private AltosUILineStyle mag_along_color = new AltosUILineStyle();
	static final private AltosUILineStyle mag_across_color = new AltosUILineStyle();
	static final private AltosUILineStyle mag_through_color = new AltosUILineStyle();
	static final private AltosUILineStyle mag_total_color = new AltosUILineStyle();

	static final private AltosUILineStyle motor_pressure_color = new AltosUILineStyle();

	static AltosUnits dop_units = null;
	static AltosUnits tick_units = null;

	AltosUIFlightSeries flight_series;

	AltosUITimeSeries[] setup(AltosFlightStats stats, AltosUIFlightSeries flight_series) {
		AltosCalData	cal_data = flight_series.cal_data();

		AltosUIAxis	height_axis, speed_axis, accel_axis, voltage_axis, temperature_axis, nsat_axis, dbm_axis;
		AltosUIAxis	pressure_axis, thrust_axis;
		AltosUIAxis	gyro_axis, orient_axis, mag_axis;
		AltosUIAxis	course_axis, dop_axis, tick_axis;
		AltosUIAxis	motor_pressure_axis;

		if (stats != null && stats.serial != AltosLib.MISSING && stats.product != null && stats.flight != AltosLib.MISSING)
			setName(String.format("%s %d flight %d\n", stats.product, stats.serial, stats.flight));

		height_axis = newAxis("Height", AltosConvert.height, height_color);
		pressure_axis = newAxis("Pressure", AltosConvert.pressure, pressure_color, 0);
		speed_axis = newAxis("Speed", AltosConvert.speed, speed_color);
		thrust_axis = newAxis("Thrust", AltosConvert.force, accel_color);
		tick_axis = newAxis("Tick", tick_units, accel_color, 0);
		accel_axis = newAxis("Acceleration", AltosConvert.accel, accel_color);
		voltage_axis = newAxis("Voltage", AltosConvert.voltage, battery_voltage_color);
		temperature_axis = newAxis("Temperature", AltosConvert.temperature, temperature_color, 0);
		nsat_axis = newAxis("Satellites", null, gps_nsat_color,
				    AltosUIAxis.axis_include_zero | AltosUIAxis.axis_integer);
		dbm_axis = newAxis("Signal Strength", null, dbm_color, 0);

		gyro_axis = newAxis("Rotation Rate", AltosConvert.rotation_rate, gyro_roll_color, 0);
		orient_axis = newAxis("Angle", AltosConvert.orient, orient_color, 0);
		mag_axis = newAxis("Magnetic Field", AltosConvert.magnetic_field, mag_along_color, 0);
		course_axis = newAxis("Course", AltosConvert.orient, gps_course_color, 0);
		dop_axis = newAxis("Dilution of Precision", dop_units, gps_pdop_color, 0);

		motor_pressure_axis = newAxis("Motor Pressure", AltosConvert.pressure, motor_pressure_color, 0);

		flight_series.register_axis("default",
					    speed_color,
					    false,
					    speed_axis);

		flight_series.register_marker(AltosUIFlightSeries.state_name,
					      state_color,
					      true,
					      plot,
					      true);

		flight_series.register_marker(AltosUIFlightSeries.pyro_fired_name,
					      igniter_marker_color,
					      true,
					      plot,
					      false);

		flight_series.register_axis(AltosUIFlightSeries.tick_name,
					    accel_color,
					    false,
					    tick_axis);

		flight_series.register_axis(AltosUIFlightSeries.accel_name,
					    accel_color,
					    true,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.vert_accel_name,
					    vert_accel_color,
					    true,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.kalman_accel_name,
					    kalman_accel_color,
					    false,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.rssi_name,
					    dbm_color,
					    false,
					    dbm_axis);

		flight_series.register_axis(AltosUIFlightSeries.speed_name,
					    speed_color,
					    true,
					    speed_axis);

		flight_series.register_axis(AltosUIFlightSeries.kalman_speed_name,
					    kalman_speed_color,
					    true,
					    speed_axis);

		flight_series.register_axis(AltosUIFlightSeries.pressure_name,
					    pressure_color,
					    false,
					    pressure_axis);

		flight_series.register_axis(AltosUIFlightSeries.height_name,
					    height_color,
					    true,
					    height_axis);

		flight_series.register_axis(AltosUIFlightSeries.altitude_name,
					    altitude_color,
					    false,
					    height_axis);

		flight_series.register_axis(AltosUIFlightSeries.kalman_height_name,
					    kalman_height_color,
					    false,
					    height_axis);


		flight_series.register_axis(AltosUIFlightSeries.temperature_name,
					    temperature_color,
					    false,
					    temperature_axis);

		flight_series.register_axis(AltosUIFlightSeries.battery_voltage_name,
					    battery_voltage_color,
					    false,
					    voltage_axis);

		flight_series.register_axis(AltosUIFlightSeries.pyro_voltage_name,
					    pyro_voltage_color,
					    false,
					    voltage_axis);

		flight_series.register_axis(AltosUIFlightSeries.apogee_voltage_name,
					    drogue_voltage_color,
					    false,
					    voltage_axis);

		flight_series.register_axis(AltosUIFlightSeries.main_voltage_name,
					    main_voltage_color,
					    false,
					    voltage_axis);

		flight_series.register_axis(AltosUIFlightSeries.sats_in_view_name,
					    gps_nsat_view_color,
					    false,
					    nsat_axis);

		flight_series.register_axis(AltosUIFlightSeries.sats_in_soln_name,
					    gps_nsat_solution_color,
					    false,
					    nsat_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_pdop_name,
					    gps_pdop_color,
					    false,
					    dop_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_hdop_name,
					    gps_hdop_color,
					    false,
					    dop_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_vdop_name,
					    gps_vdop_color,
					    false,
					    dop_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_altitude_name,
					    gps_height_color,
					    false,
					    height_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_height_name,
					    gps_height_color,
					    false,
					    height_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_ground_speed_name,
					    gps_ground_speed_color,
					    false,
					    speed_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_ascent_rate_name,
					    gps_climb_rate_color,
					    false,
					    speed_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_course_name,
					    gps_course_color,
					    false,
					    course_axis);

		flight_series.register_axis(AltosUIFlightSeries.gps_speed_name,
					    gps_speed_color,
					    false,
					    speed_axis);

		flight_series.register_axis(AltosUIFlightSeries.accel_along_name,
					    accel_along_color,
					    false,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.accel_across_name,
					    accel_across_color,
					    false,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.accel_through_name,
					    accel_through_color,
					    false,
					    accel_axis);

		flight_series.register_axis(AltosUIFlightSeries.gyro_roll_name,
					    gyro_roll_color,
					    false,
					    gyro_axis);

		flight_series.register_axis(AltosUIFlightSeries.gyro_pitch_name,
					    gyro_pitch_color,
					    false,
					    gyro_axis);

		flight_series.register_axis(AltosUIFlightSeries.gyro_yaw_name,
					    gyro_yaw_color,
					    false,
					    gyro_axis);

		flight_series.register_axis(AltosUIFlightSeries.mag_along_name,
					    mag_along_color,
					    false,
					    mag_axis);

		flight_series.register_axis(AltosUIFlightSeries.mag_across_name,
					    mag_across_color,
					    false,
					    mag_axis);

		flight_series.register_axis(AltosUIFlightSeries.mag_through_name,
					    mag_through_color,
					    false,
					    mag_axis);

		flight_series.register_axis(AltosUIFlightSeries.mag_total_name,
					    mag_total_color,
					    false,
					    mag_axis);

		flight_series.register_axis(AltosUIFlightSeries.orient_name,
					    orient_color,
					    false,
					    orient_axis);

		flight_series.register_axis(AltosUIFlightSeries.azimuth_name,
					    azimuth_color,
					    false,
					    orient_axis);

		flight_series.register_axis(AltosUIFlightSeries.compass_name,
					    compass_color,
					    false,
					    orient_axis);

		flight_series.register_axis(AltosUIFlightSeries.thrust_name,
					    accel_color,
					    true,
					    thrust_axis);

		for (int channel = 0; channel < 8; channel++) {
			flight_series.register_axis(flight_series.igniter_voltage_name(channel),
						    new AltosUILineStyle(),
						    false,
						    voltage_axis);
		}

		flight_series.register_axis(AltosUIFlightSeries.motor_pressure_name,
					    motor_pressure_color,
					    true,
					    motor_pressure_axis);

		flight_series.check_axes();

		return flight_series.series(cal_data);
	}

	public void set_data(AltosFlightStats stats, AltosUIFlightSeries flight_series) {
		set_series(setup(stats, flight_series));
	}

	public AltosGraph(AltosUIEnable enable) {
		super(enable, "Flight");
	}

	public AltosGraph(AltosUIEnable enable, AltosFlightStats stats, AltosUIFlightSeries flight_series) {
		this(enable);
		this.flight_series = flight_series;
		set_series(setup(stats, flight_series));
	}
}
