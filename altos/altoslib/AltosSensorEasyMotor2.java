/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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

import java.util.concurrent.TimeoutException;

class AltosSensorEasyMotor2 {
	int		tick;
	int		v_batt;
	int		motor_pressure;

	public AltosSensorEasyMotor2(AltosLink link) throws InterruptedException, TimeoutException {
		String[] items = link.adc();
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("motor_pressure:")) {
				motor_pressure = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("batt:")) {
				v_batt = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			i++;
		}
	}

	static public void provide_data(AltosDataListener listener, AltosLink link) throws InterruptedException {
		try {
			AltosSensorEasyMotor2	sensor_easymotor2 = new AltosSensorEasyMotor2(link);

			listener.set_battery_voltage(AltosConvert.easy_mini_2_voltage(sensor_easymotor2.v_batt));
			double pa = AltosConvert.easy_motor_2_motor_pressure(sensor_easymotor2.motor_pressure,
									     listener.cal_data().ground_motor_pressure);
			listener.set_motor_pressure(pa);

		} catch (TimeoutException te) {
		}
	}
}

