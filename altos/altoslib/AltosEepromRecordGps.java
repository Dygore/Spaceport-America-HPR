/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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
import java.text.*;

public class AltosEepromRecordGps extends AltosEepromRecord {
	public static final int	record_length = 32;

	/* AO_LOG_FLIGHT elements */
	public int flight() { return data16(0); }
	public int start_altitude() { return data16(2); }
	public int start_latitude() { return data32(4); }
	public int start_longitude() { return data32(8); }

	/* AO_LOG_GPS_TIME elements */
	public int latitude() { return data32(0); }
	public int longitude() { return data32(4); }
	public int altitude_low() { return data16(8); }
	public int hour() { return data8(10); }
	public int minute() { return data8(11); }
	public int second() { return data8(12); }
	public int flags() { return data8(13); }
	public int year() { return data8(14); }
	public int month() { return data8(15); }
	public int day() { return data8(16); }
	public int course() { return data8(17); }
	public int ground_speed() { return data16(18); }
	public int climb_rate() { return data16(20); }
	public int pdop() { return data8(22); }
	public int hdop() { return data8(23); }
	public int vdop() { return data8(24); }
	public int mode() { return data8(25); }
	public int altitude_high() { return data16(26); }

	private int seconds() {
		switch (cmd()) {
		case AltosLib.AO_LOG_GPS_TIME:
			return second() + 60 * (minute() + 60 * (hour() + 24 * (day() + 31 * month())));
		default:
			return 0;
		}
	}

	public int compareTo(AltosEepromRecord o) {
		AltosEepromRecordGps og = (AltosEepromRecordGps) o;

		int	seconds_diff = seconds() - og.seconds();

		if (seconds_diff != 0)
			return seconds_diff;

		return start - o.start;
	}

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		super.provide_data(listener, cal_data);

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			if (cal_data.flight == AltosLib.MISSING) {
				cal_data.set_boost_tick();
				cal_data.set_flight(flight());
			}
			/* no place to log start lat/lon yet */
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			AltosGPS gps = new AltosGPS();
			gps.lat = latitude() / 1e7;
			gps.lon = longitude() / 1e7;
			if (eeprom.config_data().altitude_32 == 1)
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
			if (eeprom.config_data().compare_version("1.4.9") >= 0) {
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
			listener.set_gps(gps, true, true);
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordGps(eeprom, s);
	}

	public AltosEepromRecordGps(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
	}

	public AltosEepromRecordGps(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
