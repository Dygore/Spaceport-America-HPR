/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

import java.util.*;
import java.text.*;

public class AltosParse {
	public static boolean isdigit(char c) {
		return '0' <= c && c <= '9';
	}

	public static int parse_int(String v) throws ParseException {
		try {
			return (int) AltosLib.fromdec(v);
		} catch (NumberFormatException e) {
			throw new ParseException("error parsing int " + v, 0);
		}
	}

	public static long parse_long(String v) throws ParseException {
		try {
			return AltosLib.fromdec(v);
		} catch (NumberFormatException e) {
			throw new ParseException("error parsing int " + v, 0);
		}
	}

	public static int parse_hex(String v) throws ParseException {
		try {
			return AltosLib.fromhex(v);
		} catch (NumberFormatException e) {
			throw new ParseException("error parsing hex " + v, 0);
		}
	}

	static NumberFormat get_nf_locale() {
		NumberFormat nf = NumberFormat.getInstance();
		nf.setParseIntegerOnly(false);
		nf.setGroupingUsed(false);
		return nf;
	}

	static NumberFormat nf_locale = get_nf_locale();

	static NumberFormat get_nf_net() {
		NumberFormat nf = NumberFormat.getInstance(Locale.ROOT);
		nf.setParseIntegerOnly(false);
		nf.setGroupingUsed(false);
		return nf;
	}

	static NumberFormat nf_net = get_nf_net();

	public static double parse_double_locale(String str) throws ParseException {
		try {
			return nf_locale.parse(str.trim()).doubleValue();
		} catch (ParseException pe) {
			throw new ParseException("error parsing double " + str, 0);
		}
	}

	public static String format_double_locale(double number) {
		return nf_locale.format(number);
	}

	public static double parse_double_net(String str) throws ParseException {
		try {
			String t = str.trim();
//			System.out.printf("Parse string \"%s\" trim \"%s\"\n", str, t);
			return nf_net.parse(t).doubleValue();
		} catch (ParseException pe) {
			throw new ParseException("error parsing double " + str, 0);
		}
	}

	public static String format_double_net(double number) {
		String ret = nf_net.format(number);
//		System.out.printf("format double %f \"%s\"\n", number, ret);
		return ret;
	}

	public static double parse_coord(String coord) throws ParseException {
		String[]	dsf = coord.split("\\D+");

		if (dsf.length != 3) {
			throw new ParseException("error parsing coord " + coord, 0);
		}
		int deg = parse_int(dsf[0]);
		int min = parse_int(dsf[1]);
		int frac = parse_int(dsf[2]);

		double r = deg + (min + frac / 10000.0) / 60.0;
		if (coord.endsWith("S") || coord.endsWith("W"))
			r = -r;
		return r;
	}

	public static String strip_suffix(String v, String suffix) {
		if (v.endsWith(suffix))
			return v.substring(0, v.length() - suffix.length());
		return v;
	}

	public static void word(String v, String m) throws ParseException {
		if (!v.equals(m)) {
			throw new ParseException("error matching '" + v + "' '" + m + "'", 0);
		}
	}
}
