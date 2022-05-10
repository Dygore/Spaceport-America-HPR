/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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

public class AltosEepromRecordMotor extends AltosEepromRecord {
	public static final int	record_length = 16;

	private int log_format;

	/* AO_LOG_FLIGHT elements */
	private int flight() { return data16(0); }
	private int ground_accel() { return data16(2); }
	private int ground_accel_along() { return data16(4); }
	private int ground_accel_across() { return data16(6); }
	private int ground_accel_through() { return data16(8); }
	private int ground_motor_pressure() { return data16(10); }

	/* AO_LOG_STATE elements */
	private int state() { return data16(0); }
	private int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */
	private int motor_pres() { return data16(0); }
	private int v_batt() { return data16(2); }
	private int accel() { return data16(4); }
	private int accel_across() { return data16(6); }
	private int accel_along() { return -data16(8); }
	private int accel_through() { return data16(10); }

	private int imu_type() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_EASYMOTOR:
			return AltosIMU.imu_type_easymotor_v2;
		default:
			return AltosLib.MISSING;
		}
	}

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		super.provide_data(listener, cal_data);

		cal_data.set_imu_type(imu_type());

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			cal_data.set_flight(flight());
			cal_data.set_ground_accel(ground_accel());
			listener.set_accel_ground(cal_data.accel_along(ground_accel_along()),
						  cal_data.accel_across(ground_accel_across()),
						  cal_data.accel_through(ground_accel_through()));
			cal_data.set_ground_motor_pressure(ground_motor_pressure());
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			AltosConfigData config_data = eeprom.config_data();

			listener.set_battery_voltage(AltosConvert.easy_mini_2_voltage(v_batt()));
			double pa = AltosConvert.easy_motor_2_motor_pressure(motor_pres(), cal_data.ground_motor_pressure);
			listener.set_motor_pressure(pa);

			int	accel_along = accel_along();
			int	accel_across = accel_across();
			int	accel_through = accel_through();

			listener.set_accel(cal_data.accel_along(accel_along),
					   cal_data.accel_across(accel_across),
					   cal_data.accel_through(accel_through));

			listener.set_acceleration(cal_data.acceleration(accel()));
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordMotor(eeprom, s);
	}

	public AltosEepromRecordMotor(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
		log_format = eeprom.config_data().log_format;
	}

	public AltosEepromRecordMotor(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
