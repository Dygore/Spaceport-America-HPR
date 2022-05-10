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


public class AltosTelemetryLocation extends AltosTelemetryStandard {
	int	flags() { return uint8(5); }
	int	altitude() {
		if ((mode() & AO_GPS_MODE_ALTITUDE_24) != 0)
			return (int8(31) << 16) | uint16(6);
		else
			return int16(6);
	}
	int	latitude() { return uint32(8); }
	int	longitude() { return uint32(12); }
	int	year() { return uint8(16); }
	int	month() { return uint8(17); }
	int	day() { return uint8(18); }
	int	hour() { return uint8(19); }
	int	minute() { return uint8(20); }
	int	second() { return uint8(21); }
	int	pdop() { return uint8(22); }
	int	hdop() { return uint8(23); }
	int	vdop() { return uint8(24); }
	int	mode() { return uint8(25); }
	int	ground_speed() { return uint16(26); }
	int	climb_rate() { return int16(28); }
	int	course() { return uint8(30); }

	public static final int	AO_GPS_MODE_ALTITUDE_24 = (1 << 0);	/* Reports 24-bits of altitude */

	public AltosTelemetryLocation(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {

		AltosCalData	cal_data = listener.cal_data();

		AltosGPS	gps = listener.make_temp_gps(false);

		int flags = flags();
		gps.nsat = flags & 0xf;
		gps.locked = (flags & (1 << 4)) != 0;
		gps.connected = (flags & (1 << 5)) != 0;
		gps.pdop = pdop() / 10.0;
		gps.hdop = hdop() / 10.0;
		gps.vdop = vdop() / 10.0;
		if (gps.connected)
			super.provide_data(listener);

		if (gps.locked) {
			gps.lat = latitude() * 1.0e-7;
			gps.lon = longitude() * 1.0e-7;
			gps.alt = altitude();
			gps.year = 2000 + year();
			gps.month = month();
			gps.day = day();
			gps.hour = hour();
			gps.minute = minute();
			gps.second = second();
			gps.ground_speed = ground_speed() * 1.0e-2;
			gps.course = course() * 2;
			gps.climb_rate = climb_rate() * 1.0e-2;
		}
		if (gps.connected)
			listener.set_gps(gps, true, false);
	}
}
