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

public class AltosEepromRecordMetrum extends AltosEepromRecord {
	public static final int	record_length = 16;

	public int record_length() { return record_length; }

	/* AO_LOG_FLIGHT elements */
	public int flight() { return data16(0); }
	public int ground_accel() { return data16(2); }
	public int ground_pres() { return data32(4); }
	public int ground_temp() { return data32(8); }

	/* AO_LOG_STATE elements */
	public int state() { return data16(0); }
	public int reason() { return data16(2); }

	/* AO_LOG_SENSOR elements */
	public int pres() { return data32(0); }
	public int temp() { return data32(4); }
	public int accel() { return data16(8); }

	/* AO_LOG_TEMP_VOLT elements */
	public int v_batt() { return data16(0); }
	public int sense_a() { return data16(2); }
	public int sense_m() { return data16(4); }

	/* AO_LOG_GPS_POS elements */
	public int latitude() { return data32(0); }
	public int longitude() { return data32(4); }
	public int altitude_low() { return data16(8); }
	public int altitude_high() { return data16(10); }

	/* AO_LOG_GPS_TIME elements */
	public int hour() { return data8(0); }
	public int minute() { return data8(1); }
	public int second() { return data8(2); }
	public int flags() { return data8(3); }
	public int year() { return data8(4); }
	public int month() { return data8(5); }
	public int day() { return data8(6); }
	public int pdop() { return data8(7); }

	/* AO_LOG_GPS_SAT elements */
	public int nsat() { return data8(0); }
	public int more() { return data8(1); }
	public int svid(int n) { return data8(2 + n * 2); }
	public int c_n(int n) { return data8(2 + n * 2 + 1); }

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		super.provide_data(listener, cal_data);

		AltosGPS	gps;

		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			cal_data.set_flight(flight());
			cal_data.set_ground_accel(ground_accel());
			cal_data.set_ground_pressure(ground_pres());
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(state());
			break;
		case AltosLib.AO_LOG_SENSOR:
			AltosPresTemp pt = eeprom.config_data().ms5607().pres_temp(pres(), temp());
			listener.set_pressure(pt.pres);
			listener.set_temperature(pt.temp);
			listener.set_acceleration(cal_data.acceleration(accel()));
			break;
		case AltosLib.AO_LOG_TEMP_VOLT:
			listener.set_battery_voltage(AltosConvert.mega_battery_voltage(v_batt()));
			listener.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sense_a()));
			listener.set_main_voltage(AltosConvert.mega_pyro_voltage(sense_m()));
			break;
		case AltosLib.AO_LOG_GPS_POS:
			gps = listener.make_temp_gps(false);
			gps.lat = latitude() / 1e7;
			gps.lon = longitude() / 1e7;
			if (config_data().altitude_32())
				gps.alt = (altitude_low() & 0xffff) | (altitude_high() << 16);
			else
				gps.alt = altitude_low();
			break;
		case AltosLib.AO_LOG_GPS_TIME:
			gps = listener.make_temp_gps(false);

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
			gps.pdop = pdop() / 10.0;
			break;
		case AltosLib.AO_LOG_GPS_SAT:
			gps = listener.make_temp_gps(true);

			int n = nsat();
			for (int i = 0; i < n; i++)
				gps.add_sat(svid(i), c_n(i));
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordMetrum(eeprom, s);
	}

	public AltosEepromRecordMetrum(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
	}

	public AltosEepromRecordMetrum(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
