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

import java.io.*;

public class AltosGauss extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		return v;
	}

	public String show_units(boolean imperial_units) {
		return "G";
	}

	public String say_units(boolean imperial_units) {
		return "gauss";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return width - 1;
	}

	public AltosGauss() {
		range_metric = new AltosUnitsRange[1];

		range_metric[0] = new AltosUnitsRange(0, "µT", "microtesla") {
				double value(double v) {
					return v * 100;
				}
				int show_fraction(int width) {
					return width / 9;
				}
				int say_fraction() {
					return 0;
				}
			};

		range_imperial = range_metric;
	}
}
