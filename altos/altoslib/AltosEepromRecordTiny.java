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

public class AltosEepromRecordTiny extends AltosEepromRecord implements AltosDataProvider {
	public static final int	record_length = 2;

	private int value() {
		return eeprom.data16(start);
	}

	public boolean valid(int s) {
		return eeprom.data16(s) != 0xffff;
	}

	public int cmd() {
		if (start == 0)
			return AltosLib.AO_LOG_FLIGHT;
		if ((value() & 0x8000) != 0)
			return AltosLib.AO_LOG_STATE;
		return AltosLib.AO_LOG_SENSOR;
	}

	public int tick() {
		int	tick = 0;
		int	step = 10;
		for (int i = 2; i < start; i += 2)
		{
			int v = eeprom.data16(i);

			if ((v & 0x8000) != 0) {
				if ((v & 0x7fff) >= AltosLib.ao_flight_drogue)
					step = 100;
			} else {
				tick += step;
			}
		}
		return tick;
	}

	public void provide_data(AltosDataListener listener) {
		int value = data16(-header_length);

		listener.set_tick(tick());
		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			listener.set_state(AltosLib.ao_flight_pad);
			listener.cal_data().set_flight(value);
			listener.cal_data().set_boost_tick();
			break;
		case AltosLib.AO_LOG_STATE:
			listener.set_state(value & 0x7fff);
			break;
		case AltosLib.AO_LOG_SENSOR:
			listener.set_pressure(AltosConvert.barometer_to_pressure(value));
			break;
		}
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordTiny(eeprom, s);
	}

	public AltosEepromRecordTiny(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
	}

	public AltosEepromRecordTiny(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
