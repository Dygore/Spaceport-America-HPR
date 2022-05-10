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
import java.lang.Math;
import java.util.*;
import java.util.concurrent.*;

public class AltosMapPathPoint {
	public AltosGPS		gps;
	public double		time;
	public int		state;
	public double		gps_height;

	public int hashCode() {
		return Double.valueOf(gps.lat).hashCode() ^ Double.valueOf(gps.lon).hashCode() ^ state;
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;

		if (!(o instanceof AltosMapPathPoint))
			return false;

		AltosMapPathPoint other = (AltosMapPathPoint) o;

		return gps.lat == other.gps.lat && gps.lon == other.gps.lon && state == other.state;
	}

	public AltosMapPathPoint(AltosGPS gps, double time, int state, double gps_height) {
		this.gps = gps;
		this.time = time;
		this.state = state;
		this.gps_height = gps_height;
	}
}

