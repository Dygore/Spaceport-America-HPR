/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

class AltosSensorMetrum {
	int		tick;
	int		sense_a;
	int		sense_m;
	int		v_batt;

	public AltosSensorMetrum(AltosLink link) throws InterruptedException, TimeoutException {
		String[] items = link.adc();
		for (int i = 0; i < items.length;) {
			if (items[i].equals("tick:")) {
				tick = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("drogue:")) {
				sense_a = Integer.parseInt(items[i+1]);
				i += 2;
				continue;
			}
			if (items[i].equals("main:")) {
				sense_m = Integer.parseInt(items[i+1]);
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
			AltosSensorMetrum	sensor_metrum = new AltosSensorMetrum(link);
			listener.set_battery_voltage(AltosConvert.mega_battery_voltage(sensor_metrum.v_batt));
			listener.set_apogee_voltage(AltosConvert.mega_pyro_voltage(sensor_metrum.sense_a));
			listener.set_main_voltage(AltosConvert.mega_pyro_voltage(sensor_metrum.sense_m));
		} catch (TimeoutException te) {
		}
	}
}

