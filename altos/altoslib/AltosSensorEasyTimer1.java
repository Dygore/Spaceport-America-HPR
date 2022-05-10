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

class AltosSensorEasyTimer1 {
	int		tick;
	int[]		sense;
	int		v_batt;
	int		v_pbatt;
	int		temp;

	public AltosSensorEasyTimer1() {
		sense = new int[2];
	}

	public AltosSensorEasyTimer1(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		String[] items = link.adc();
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("A:")) {
				sense[0] = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("B:")) {
				sense[1] = Integer.parseInt(items[i+1]);
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
			AltosSensorEasyTimer1	sensor_easytimer1 = new AltosSensorEasyTimer1(link);

			listener.set_battery_voltage(AltosConvert.easy_timer_voltage(sensor_easytimer1.v_batt));

			double[]	igniter_voltage = new double[2];
			for (int i = 0; i < 2; i++)
				igniter_voltage[i] = AltosConvert.easy_timer_voltage(sensor_easytimer1.sense[i]);
			listener.set_igniter_voltage(igniter_voltage);

		} catch (TimeoutException te) {
		}
	}
}

