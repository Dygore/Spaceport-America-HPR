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

public class AltosDistance extends AltosUnits {

	public double value(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.meters_to_miles(v);
		return v;
	}

	public double inverse(double v, boolean imperial_units) {
		if (imperial_units)
			return AltosConvert.miles_to_meters(v);
		return v;
	}

	public String show_units(boolean imperial_units) {
		if (imperial_units)
			return "miles";
		return "m";
	}

	public String say_units(boolean imperial_units) {
		if (imperial_units)
			return "miles";
		return "meters";
	}

	public int show_fraction(int width, boolean imperial_units) {
		if (imperial_units)
			return width / 3;
		return width / 9;
	}

	public int say_fraction(boolean imperial_units) {
		if (imperial_units)
			return 1;
		return 0;
	}

	public AltosDistance() {
		range_metric = new AltosUnitsRange[2];

		range_metric[0] = new AltosUnitsRange(0, "m", "meters") {
				double value(double v) {
					return v;
				}
				int show_fraction(int width) {
					return width / 9;
				}
				int say_fraction() {
					return 0;
				}
			};
		range_metric[1] = new AltosUnitsRange(2000, "km", "kilometers") {
				double value(double v) {
					return v / 1000;
				}
				int show_fraction(int width) {
					return width / 5;
				}
				int say_fraction() {
					return 1;
				}
			};

		range_imperial = new AltosUnitsRange[2];

		range_imperial[0] = new AltosUnitsRange(0, "ft", "feet") {
				double value(double v) {
					return AltosConvert.meters_to_feet(v);
				}
				int show_fraction(int width) {
					return width / 9;
				}
				int say_fraction() {
					return 0;
				}
			};

		range_imperial[1] = new AltosUnitsRange(AltosConvert.feet_to_meters(5280),
							"mi", "miles") {
				double value(double v) {
					return AltosConvert.meters_to_miles(v);
				}
				int show_fraction(int width) {
					return width / 5;
				}
				int say_fraction() {
					return 1;
				}
			};
	}
}
