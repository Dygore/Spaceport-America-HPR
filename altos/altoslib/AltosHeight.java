/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

public class AltosHeight extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.meters_to_feet(v);
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.feet_to_meters(v);
		return v;
	}

	public String show_units(boolean imperial_units) {
		if (imperial_units)
			return "ft";
		return "m";
	}

	public String say_units(boolean imperial_units) {
		if (imperial_units)
			return "feet";
		return "meters";
	}

	public int show_fraction(int width, boolean imperial_units) {
		return width / 9;
	}
}