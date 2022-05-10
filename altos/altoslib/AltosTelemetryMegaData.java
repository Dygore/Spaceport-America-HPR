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

public class AltosTelemetryMegaData extends AltosTelemetryStandard {

	int	state() { return uint8(5); }

	int	v_batt() { return int16(6); }
	int	v_pyro() { return int16(8); }

	/* pyro sense values are sent in 8 bits, expand to 12 bits */
	int	sense(int i) { int v = uint8(10+i); return (v << 4) | (v >> 4); }

	int	ground_pres() { return int32(16); }
	int	ground_accel() { return int16(20); }
	int	accel_plus_g() { return int16(22); }
	int	accel_minus_g() { return int16(24);}

	int	acceleration() { return int16(26); }
	int	speed() { return int16(28); }
	int	height_16() { return int16(30); }

	public AltosTelemetryMegaData(int[] bytes) throws AltosCRCException {
		super(bytes);
	}

	public void provide_data(AltosDataListener listener) {
		super.provide_data(listener);

		listener.set_state(state());

		listener.set_battery_voltage(AltosConvert.mega_battery_voltage(v_batt()));
		listener.set_pyro_voltage(AltosConvert.mega_pyro_voltage(v_pyro()));

		listener.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sense(4)));
		listener.set_main_voltage(AltosConvert.mega_pyro_voltage(sense(5)));

		double voltages[] = new double[4];
		for (int i = 0; i < 4; i++)
			voltages[i] = AltosConvert.mega_pyro_voltage(sense(i));

		listener.set_igniter_voltage(voltages);

		AltosCalData cal_data = listener.cal_data();

		cal_data.set_ground_accel(ground_accel());
		cal_data.set_ground_pressure(ground_pres());
		cal_data.set_accel_plus_minus(accel_plus_g(), accel_minus_g());

		/* Fill in the high bits of height from recent GPS
		 * data if available, otherwise guess using the
		 * previous kalman height
		 */

		listener.set_kalman(height_16(), speed()/16.0, acceleration() / 16.0);
	}
}

