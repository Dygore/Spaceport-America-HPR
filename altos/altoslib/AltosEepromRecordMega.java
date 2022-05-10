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

public class AltosEepromRecordMega extends AltosEepromRecord {
	public static final int	record_length = 32;

	public static final int max_sat = 12;

	private int log_format;

	/* AO_LOG_FLIGHT elements */
	private int flight() { return data16(0); }
	private int ground_accel() { return data16(2); }
	private int ground_pres() { return data32(4); }
	private int ground_accel_along() { return data16(8); }
	private int ground_accel_across() { return data16(10); }
	private int ground_accel_through() { return data16(12); }
	private int ground_roll() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return data32(16);
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return data16(14);
		default:
			return AltosLib.MISSING;
		}
	}
	private int ground_pitch() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return data32(20);
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return data16(16);
		default:
			return AltosLib.MISSING;
		}
	}
	private int ground_yaw() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return data32(24);
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return data16(18);
		default:
			return AltosLib.MISSING;
		}
	}

	/* AO_LOG_STATE elements */
	private int state() { return data16(0); }
	private int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */
	private int pres() { return data32(0); }
	private int temp() { return data32(4); }
	private int accel_x() { return data16(8); }
	private int accel_y() { return data16(10); }
	private int accel_z() { return data16(12); }
	private int gyro_x() { return data16(14); }
	private int gyro_y() { return data16(16); }
	private int gyro_z() { return data16(18); }
	private int mag_x() { return data16(20); }
	private int mag_z() { return data16(22); }
	private int mag_y() { return data16(24); }

	/* normalized log data */
	private int norm_accel_along() { return data16(8); }
	private int norm_accel_across() { return data16(10); }
	private int norm_accel_through() { return data16(12); }
	private int norm_gyro_roll() { return data16(14); }
	private int norm_gyro_pitch() { return data16(16); }
	private int norm_gyro_yaw() { return data16(18); }
	private int norm_mag_along() { return data16(20); }
	private int norm_mag_across() { return data16(22); }
	private int norm_mag_through() { return data16(24); }

	private int imu_type() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return AltosIMU.imu_type_telemega_v1_v2;
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
			return AltosIMU.imu_type_telemega_v3;
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
			return AltosIMU.imu_type_easymega_v2;
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return AltosIMU.imu_type_telemega_v4;
		default:
			return AltosLib.MISSING;
		}
	}

	private int imu_model() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return AltosLib.model_mpu6000;
		}
		return AltosLib.MISSING;
	}

	private boolean sensor_normalized() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return true;
		}
		return false;
	}

	private int mag_model() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_5:
			return AltosLib.model_mmc5983;
		}
		return AltosLib.MISSING;
	}

	private int accel_across() {
		if (sensor_normalized()) {
			return norm_accel_across();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return accel_x();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return -accel_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int accel_along(){
		if (sensor_normalized()) {
			return norm_accel_along();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return accel_y();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return accel_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int accel_through() {
		if (sensor_normalized()) {
			return norm_accel_through();
		}

		return accel_z();
	}

	private int gyro_pitch() {
		if (sensor_normalized()) {
			return norm_gyro_pitch();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return gyro_x();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
			return -gyro_y();
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return -gyro_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int gyro_roll() {
		if (sensor_normalized()) {
			return norm_gyro_roll();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return gyro_y();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return gyro_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int gyro_yaw() {
		if (sensor_normalized()) {
			return norm_gyro_yaw();
		}

		return gyro_z();
	}

	private int mag_across() {
		if (sensor_normalized()) {
			return norm_mag_across();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return mag_x();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
			return -mag_y();
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return mag_y();
		default:
			return AltosLib.MISSING;
		}
	}

	private int mag_along() {
		if (sensor_normalized()) {
			return norm_mag_along();
		}

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
			return mag_y();
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
			return mag_x();
		default:
			return AltosLib.MISSING;
		}
	}

	private int mag_through() {
		if (sensor_normalized()) {
			return norm_mag_through();
		}

		return mag_z();
	}


	private int accel() { return data16(26); }

	/* AO_LOG_TEMP_VOLT elements */
	private int v_batt() { return data16(0); }
	private int v_pbatt() { return data16(2); }
	private int nsense() { return data16(4); }
	private int sense(int i) { return data16(6 + i * 2); }
	private int pyro() { return data16(26); }

	/* AO_LOG_GPS_TIME elements */
	private int latitude() { return data32(0); }
	private int longitude() { return data32(4); }
	private int altitude_low() { return data16(8); }
	private int hour() { return data8(10); }
	private int minute() { return data8(11); }
	private int second() { return data8(12); }
	private int flags() { return data8(13); }
	private int year() { return data8(14); }
	private int month() { return data8(15); }
	private int day() { return data8(16); }
	private int course() { return data8(17); }
	private int ground_speed() { return data16(18); }
	private int climb_rate() { return data16(20); }
	private int pdop() { return data8(22); }
	private int hdop() { return data8(23); }
	private int vdop() { return data8(24); }
	private int mode() { return data8(25); }
	private int altitude_high() { return data16(26); }

	/* AO_LOG_GPS_SAT elements */
	private int nsat() { return data16(0); }
	private int svid(int n) { return data8(2 + n * 2); }
	private int c_n(int n) { return data8(2 + n * 2 + 1); }

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		super.provide_data(listener, cal_data);

		AltosGPS	gps;

		cal_data.set_imu_type(imu_type());
		cal_data.set_imu_model(imu_model());
		cal_data.set_mag_model(mag_model());

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			cal_data.set_flight(flight());
			cal_data.set_ground_accel(ground_accel());
			cal_data.set_ground_pressure(ground_pres());
			listener.set_accel_ground(cal_data.accel_along(ground_accel_along()),
						  cal_data.accel_across(ground_accel_across()),
						  cal_data.accel_through(ground_accel_through()));
			cal_data.set_gyro_zero(ground_roll() / 512.0,
					       ground_pitch() / 512.0,
					       ground_yaw() / 512.0);
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			AltosConfigData config_data = eeprom.config_data();
			AltosPresTemp pt = config_data.ms5607().pres_temp(pres(), temp());;
			listener.set_pressure(pt.pres);
			listener.set_temperature(pt.temp);

			int	accel_along = accel_along();
			int	accel_across = accel_across();
			int	accel_through = accel_through();
			int	gyro_roll = gyro_roll();
			int	gyro_pitch = gyro_pitch();
			int	gyro_yaw = gyro_yaw();

			int	mag_along = mag_along();
			int	mag_across = mag_across();
			int	mag_through = mag_through();

			if (log_format == AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD)
				cal_data.check_imu_wrap(gyro_roll, gyro_pitch, gyro_yaw);

			listener.set_accel(cal_data.accel_along(accel_along),
					   cal_data.accel_across(accel_across),
					   cal_data.accel_through(accel_through));
			listener.set_gyro(cal_data.gyro_roll(gyro_roll),
					  cal_data.gyro_pitch(gyro_pitch),
					  cal_data.gyro_yaw(gyro_yaw));

			listener.set_mag(cal_data.mag_along(mag_along),
					 cal_data.mag_across(mag_across),
					 cal_data.mag_through(mag_through));

			listener.set_acceleration(cal_data.acceleration(accel()));
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			listener.set_battery_voltage(AltosConvert.mega_battery_voltage(v_batt()));
			listener.set_pyro_voltage(AltosConvert.mega_pyro_voltage(v_pbatt()));

			int nsense = nsense();

			listener.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sense(nsense-2)));
			listener.set_main_voltage(AltosConvert.mega_pyro_voltage(sense(nsense-1)));

			double voltages[] = new double[nsense-2];
			for (int i = 0; i < nsense-2; i++)
				voltages[i] = AltosConvert.mega_pyro_voltage(sense(i));

			listener.set_igniter_voltage(voltages);
			listener.set_pyro_fired(pyro());
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			gps = listener.make_temp_gps(false);
			gps.lat = latitude() / 1e7;
			gps.lon = longitude() / 1e7;

			if (config_data().altitude_32())
				gps.alt = (altitude_low() & 0xffff) | (altitude_high() << 16);
			else
				gps.alt = altitude_low();

			gps.hour = hour();
			gps.minute = minute();
			gps.second = second();

			int flags = flags();

			gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;

			gps.year = 2000 + year();
			gps.month = month();
			gps.day = day();
			gps.ground_speed = ground_speed() * 1.0e-2;
			gps.course = course() * 2;
			gps.climb_rate = climb_rate() * 1.0e-2;
			if (config_data().compare_version("1.4.9") >= 0) {
				gps.pdop = pdop() / 10.0;
				gps.hdop = hdop() / 10.0;
				gps.vdop = vdop() / 10.0;
			} else {
				gps.pdop = pdop() / 100.0;
				if (gps.pdop < 0.8)
					gps.pdop += 2.56;
				gps.hdop = hdop() / 100.0;
				if (gps.hdop < 0.8)
					gps.hdop += 2.56;
				gps.vdop = vdop() / 100.0;
				if (gps.vdop < 0.8)
					gps.vdop += 2.56;
			}
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			gps = listener.make_temp_gps(true);

			int n = nsat();
			if (n > max_sat)
				n = max_sat;
			for (int i = 0; i < n; i++)
				gps.add_sat(svid(i), c_n(i));
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordMega(eeprom, s);
	}

	public AltosEepromRecordMega(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
		log_format = eeprom.config_data().log_format;
	}

	public AltosEepromRecordMega(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
