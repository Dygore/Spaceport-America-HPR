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


public class AltosTelemetryMini2 extends AltosTelemetryStandard {

	int	state() { return uint8(5); }

	int	v_batt() { return int16(6); }
	int	sense_a() { return int16(8); }
	int	sense_m() { return int16(10); }

	int	pres() { return int32(12); }
	int	temp() { return int16(16); }

	int	acceleration() { return int16(18); }
	int	speed() { return int16(20); }
	int	height() { return int16(22); }

	int	ground_pres() { return int32(24); }

	public AltosTelemetryMini2(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		listener.set_state(state());

		listener.set_battery_voltage(AltosConvert.tele_mini_2_voltage(v_batt()));
		listener.set_apogee_voltage(AltosConvert.tele_mini_2_voltage(sense_a()));
		listener.set_main_voltage(AltosConvert.tele_mini_2_voltage(sense_m()));

		AltosCalData	cal_data = listener.cal_data();

		cal_data.set_ground_pressure(ground_pres());

		listener.set_pressure(pres());
		listener.set_temperature(temp()/100.0);

		listener.set_kalman(height(), speed()/16.0, acceleration()/16.0);
	}
}
