/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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
import java.lang.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

public class AltosLaunchSite {
	public String	name;
	public double	latitude;
	public double	longitude;

	public String toString() {
		return name;
	}

	public AltosLaunchSite(String in_name, double in_latitude, double in_longitude) {
		name = in_name;
		latitude = in_latitude;
		longitude = in_longitude;
	}

	public AltosLaunchSite(String line) throws ParseException {
		String[]	elements = line.split(":");

		if (elements.length < 3)
			throw new ParseException(String.format("Invalid site line %s", line), 0);

		name = elements[0];

		try {
			latitude = AltosParse.parse_double_net(elements[1]);
			longitude = AltosParse.parse_double_net(elements[2]);
		} catch (ParseException pe) {
			throw new ParseException(String.format("Invalid site line %s", line), 0);
		}
	}
}

