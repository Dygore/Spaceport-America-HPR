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

public class AltosTime extends AltosUnits {
	public double value(double v, boolean imperial_units) { return v; }

	public double inverse(double v, boolean imperial_unis) { return v; }

	public String show_units(boolean imperial_units) { return "s"; }

	public String say_units(boolean imperial_units) { return "seconds"; }

	public int show_fraction(int width, boolean imperial_units) {
		if (width < 5)
			return 0;
		return width - 5;
	}

	public int say_fraction(boolean imperial_units) { return 0; }
}
