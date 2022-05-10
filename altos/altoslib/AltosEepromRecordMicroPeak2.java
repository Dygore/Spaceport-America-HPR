/*
 * Copyright © 2019 Keith Packard <keithp@keithp.com>
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

public class AltosEepromRecordMicroPeak2 extends AltosEepromRecord {
	public static final int	record_length = 2;

	private static final int PA_GROUND_OFFSET = 0;
	private static final int PA_MIN_OFFSET = 4;
	private static final int N_SAMPLES_OFFSET = 8;
	private static final int STARTING_LOG_OFFSET = 10;

	private static final int LOG_ID_MICROPEAK = 0;
	private static final int LOG_ID_MICROKITE = 1;
	private static final int LOG_ID_MICROPEAK2 = 2;

	private int value16(int o) {
		return eeprom.data16(o);
	}

	private int value32(int o) {
		return eeprom.data32(o);
	}

	public int cmd() {
		if (start == 0)
			return AltosLib.AO_LOG_FLIGHT;
		return AltosLib.AO_LOG_SENSOR;
	}

	private int pa_ground() {
		return value32(PA_GROUND_OFFSET);
	}

	private int pa_min() {
		return value32(PA_MIN_OFFSET);
	}

	private int log_id() {
		return value16(N_SAMPLES_OFFSET) >> 12;
	}

	private int n_samples() {
		return value16(N_SAMPLES_OFFSET) & 0xfff;
	}

	private int ticks_per_sample() {
		int log_id = log_id();

		if (log_id == LOG_ID_MICROPEAK)
			return 2;
		if (log_id == LOG_ID_MICROKITE)
			return 200;
		if (log_id == LOG_ID_MICROPEAK2)
			return 10;
		return 1;
	}

	public int tick() {
		if (start <= STARTING_LOG_OFFSET)
			return 0;
		return ((start - STARTING_LOG_OFFSET) / 2) * ticks_per_sample();
	}

	public double ticks_per_sec() {
		int log_id = log_id();

		if (log_id == LOG_ID_MICROPEAK)
			return 1000.0/96.0;
		if (log_id == LOG_ID_MICROKITE)
			return 1000 / 96.0;
		if (log_id == LOG_ID_MICROPEAK2)
			return 100.0;
		return 100.0;
	}

	int mix_in (int high, int low) {
		return  high - (high & 0xffff) + low;
	}

	boolean closer (int target, int a, int b) {
		return Math.abs (target - a) < Math.abs(target - b);
	}

	private int pressure() {
		int cur = value32(PA_GROUND_OFFSET);
		for (int s = STARTING_LOG_OFFSET; s <= start; s += 2) {
			int 	k = value16(s);
			int	same = mix_in(cur, k);
			int	up = mix_in(cur + 0x10000, k);
			int	down = mix_in(cur - 0x10000, k);

			if (closer (cur, same, up)) {
				if (closer (cur, same, down))
					cur = same;
				else
					cur = down;
			} else {
				if (closer (cur, up, down))
					cur = up;
				else
					cur = down;
			}
		}
		return cur;
	}

	public void provide_data(AltosDataListener listener, AltosCalData cal_data) {
		listener.set_tick(tick());
		switch (cmd()) {
		case AltosLib.AO_LOG_FLIGHT:
			int pa_ground = pa_ground();
			int pa_min = pa_min();
			int n_samples = n_samples();
			int log_id = log_id();
			listener.set_state(AltosLib.ao_flight_pad);
			listener.cal_data().set_ground_pressure(pa_ground);
			listener.cal_data().set_ticks_per_sec(ticks_per_sec());
			listener.cal_data().set_boost_tick();
			listener.set_avoid_duplicate_files();
			break;
		case AltosLib.AO_LOG_SENSOR:
			listener.set_state(AltosLib.ao_flight_boost);
			listener.set_pressure(pressure());
			break;
		}
	}

	public int next_start() {
		if (start == 0)
			return STARTING_LOG_OFFSET;
		if (start + 2 >= STARTING_LOG_OFFSET + 2 * n_samples())
			return -1;
		return start + 2;
	}

	public AltosEepromRecord next() {
		int	s = next_start();
		if (s < 0)
			return null;
		return new AltosEepromRecordMicroPeak2(eeprom, s);
	}

	public AltosEepromRecordMicroPeak2(AltosEeprom eeprom, int start) {
		super(eeprom, start, record_length);
	}

	public AltosEepromRecordMicroPeak2(AltosEeprom eeprom) {
		this(eeprom, 0);
	}
}
