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

import java.text.*;

public abstract class AltosUnits {

	AltosUnitsRange[]	range_metric, range_imperial;

	private AltosUnitsRange range(double v, boolean imperial_units) {
		AltosUnitsRange[]	ranges = imperial_units ? range_imperial : range_metric;

		for (int i = ranges.length - 1; i > 0; i--)
			if (v >= ranges[i].lower_limit)
				return ranges[i];
		return ranges[0];
	}

	private AltosUnitsRange first_range(boolean imperial_units) {
		return imperial_units ? range_imperial[0] : range_metric[0];
	}

	public abstract double value(double v, boolean imperial_units);

	public abstract double inverse(double v, boolean imperial_units);

	public String string_value(double v, boolean imperial_units) {
		return Double.valueOf(value(v, imperial_units)).toString();
	}

	public abstract String show_units(boolean imperial_units);

	public abstract String say_units(boolean imperial_units);

	public abstract int show_fraction(int width, boolean imperial_units);

	private double value(double v) {
		return value(v, AltosConvert.imperial_units);
	}

	private double inverse(double v) {
		return inverse(v, AltosConvert.imperial_units);
	}

	private String show_units() {
		return show_units(AltosConvert.imperial_units);
	}

	private String say_units() {
		return say_units(AltosConvert.imperial_units);
	}

	private int show_fraction(int width) {
		return show_fraction(width, AltosConvert.imperial_units);
	}

	private int say_fraction(boolean imperial_units) {
		return 0;
	}

	private String show_format(AltosUnitsRange range, int width) {
		return String.format("%%%d.%df %s", width, range.show_fraction(width), range.show_units);
	}

	private String say_format(AltosUnitsRange range) {
		return String.format("%%1.%df", range.say_fraction());
	}

	private String say_units_format(AltosUnitsRange range)  {
		return String.format("%%1.%df %s", range.say_fraction(), range.say_units);
	}

	public String show(int width, double v, boolean imperial_units) {
		AltosUnitsRange range = range(v, imperial_units);

		return String.format(show_format(range, width), range.value(v));
	}

	public String say(double v, boolean imperial_units) {
		AltosUnitsRange range = range(v, imperial_units);

		return String.format(say_format(range), range.value(v));
	}

	public String say_units(double v, boolean imperial_units) {
		AltosUnitsRange range = range(v, imperial_units);

		return String.format(say_units_format(range), range.value(v));
	}

	public String show(int width, double v) {
		return show(width, v, AltosConvert.imperial_units);
	}

	public String say(double v) {
		return say(v, AltosConvert.imperial_units);
	}

	public String say_units(double v) {
		return say_units(v, AltosConvert.imperial_units);
	}

	public String string_value(double v) {
		return string_value(v, AltosConvert.imperial_units);
	}

	/* Parsing functions. Use the first range of the type */
	public String parse_units(boolean imperial_units) {
		return first_range(imperial_units).show_units;
	}

	public String parse_units() {
		return parse_units(AltosConvert.imperial_units);
	}

	public double parse_value(double v, boolean imperial_units) {
		return first_range(imperial_units).value(v);
	}

	public double parse_value(double v) {
		return parse_value(v, AltosConvert.imperial_units);
	}

	/* Graphing functions. Use the first range of the type */
	public String graph_units(boolean imperial_units) {
		return first_range(imperial_units).show_units;
	}

	public String graph_units() {
		return graph_units(AltosConvert.imperial_units);
	}

	public double graph_value(double v, boolean imperial_units) {
		return first_range(imperial_units).value(v);
	}

	public double graph_value(double v) {
		return graph_value(v, AltosConvert.imperial_units);
	}

	private String graph_format(AltosUnitsRange range, int width) {
		return String.format(String.format("%%%d.%df", width, range.show_fraction(width)), 0.0);
	}

	public String graph_format(int width, boolean imperial_units) {
		return graph_format(first_range(imperial_units), width);
	}

	public String graph_format(int width) {
		return graph_format(width, AltosConvert.imperial_units);
	}

	/* Parsing functions. */
	public double parse_locale(String s, boolean imperial_units) throws ParseException {
		double v = AltosParse.parse_double_locale(s);
		return inverse(v, imperial_units);
	}

	public double parse_net(String s, boolean imperial_units) throws ParseException {
		double v = AltosParse.parse_double_net(s);
		return inverse(v, imperial_units);
	}

	public double parse_locale(String s) throws ParseException {
		return parse_locale(s, AltosConvert.imperial_units);
	}

	public double parse_net(String s) throws ParseException {
		return parse_net(s, AltosConvert.imperial_units);
	}

	public AltosUnits() {
		range_metric = new AltosUnitsRange[1];

		range_metric[0] = new AltosUnitsRange(this, false) {
				double value(double v) {
					return units.value(v, false);
				}

				int show_fraction(int width) {
					return units.show_fraction(width, false);
				}

				int say_fraction() {
					return units.say_fraction(false);
				}
			};

		range_imperial = new AltosUnitsRange[1];

		range_imperial[0] = new AltosUnitsRange(this, true) {
				double value(double v) {
					return units.value(v, true);
				}

				int show_fraction(int width) {
					return units.show_fraction(width, true);
				}

				int say_fraction() {
					return units.say_fraction(true);
				}
			};
	}
}
