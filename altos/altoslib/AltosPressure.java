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

public class AltosPressure extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.pa_to_psi(v);
		return v / 1000.0;
	}

	public double inverse(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.psi_to_pa(v);
		return v * 1000.0;
	}

	public String show_units(boolean imperial_units) {
		if (imperial_units)
			return "psi";
		return "kPa";
	}

	public String say_units(boolean imperial_units) {
		if (imperial_units)
			return "p s i";
		return "kilopascals";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return width / 5;
	}
}
