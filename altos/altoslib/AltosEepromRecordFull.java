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

public class AltosEepromRecordFull extends AltosEepromRecord {
	public static final int	record_length = 8;

	public static final int max_sat = 12;

	public static final int two_g_default = 16294 - 15758;

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {

		super.provide_data(listener, cal_data);
		AltosGPS	gps;

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			listener.set_state(AltosLib.ao_flight_pad);
			cal_data.set_ground_accel(data16(0));
			cal_data.set_flight(data16(2));
			if (cal_data.accel_plus_g == AltosLib.MISSING)
				cal_data.set_accel_plus_minus(data16(0), data16(0) + two_g_default);
			break;
		case AltosLib.AO_LOG_SENSOR:
			listener.set_acceleration(cal_data.acceleration(data16(0)));
			listener.set_pressure(AltosConvert.barometer_to_pressure(data16(2)));
			break;
		case AltosLib.AO_LOG_PRESSURE:
			listener.set_pressure(AltosConvert.barometer_to_pressure(data16(2)));
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			listener.set_temperature(AltosConvert.thermometer_to_temperature(data16(0)));
			listener.set_battery_voltage(AltosConvert.cc_battery_to_voltage(data16(2)));
			break;
		case AltosLib.AO_LOG_DEPLOY:
			listener.set_apogee_voltage(AltosConvert.cc_igniter_to_voltage(data16(0)));
			listener.set_main_voltage(AltosConvert.cc_igniter_to_voltage(data16(2)));
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(data16(0));
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			gps = listener.make_temp_gps(false);

			gps.hour = data8(0);
			gps.minute = data8(1);
			gps.second = data8(2);

			int flags = data8(3);

			gps.connected = (flags & AltosLib.AO_GPS_RUNNING) != 0;
			gps.locked = (flags & AltosLib.AO_GPS_VALID) != 0;
			gps.nsat = (flags & AltosLib.AO_GPS_NUM_SAT_MASK) >>
				AltosLib.AO_GPS_NUM_SAT_SHIFT;
			break;
		case AltosLib.AO_LOG_GPS_LAT:
			gps = listener.make_temp_gps(false);

			int lat32 = data32(0);
			gps.lat = (double) lat32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_LON:
			gps = listener.make_temp_gps(false);

			int lon32 = data32(0);
			gps.lon = (double) lon32 / 1e7;
			break;
		case AltosLib.AO_LOG_GPS_ALT:
			gps = listener.make_temp_gps(false);
			gps.alt = data16(0);
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			gps = listener.make_temp_gps(true);
			int svid = data16(0);
			int c_n0 = data16(2);
			gps.add_sat(svid, c_n0);
			break;
		case AltosLib.AO_LOG_GPS_DATE:
			gps = listener.make_temp_gps(false);
			gps.year = data8(0) + 2000;
			gps.month = data8(1);
			gps.day = data8(2);
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordFull(eeprom, s);
	}

	public AltosEepromRecordFull(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
	}

	public AltosEepromRecordFull(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
