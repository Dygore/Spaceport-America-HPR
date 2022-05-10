/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

import java.text.*;

public abstract class AltosUnitsRange {

	AltosUnits	units;

	double		lower_limit;
	String		show_units;
	String		say_units;

	abstract double	value(double v);
	abstract int show_fraction(int width);
	abstract int say_fraction();

	AltosUnitsRange(AltosUnits units, boolean imperial_units) {
		this.units = units;
		this.show_units = units.show_units(imperial_units);
		this.say_units = units.say_units(imperial_units);
	}

	AltosUnitsRange(double lower_limit, String show_units, String say_units) {
		this.units = null;
		this.lower_limit = lower_limit;
		this.show_units = show_units;
		this.say_units = say_units;
	}
}
