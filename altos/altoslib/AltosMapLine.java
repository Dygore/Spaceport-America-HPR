/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

public abstract class AltosMapLine {
	public AltosLatLon	start, end;

	static public int stroke_width = 6;

	public abstract void paint(AltosMapTransform t);

	private AltosLatLon lat_lon(AltosPointInt pt, AltosMapTransform t) {
		return t.screen_lat_lon(pt);
	}

	public void dragged(AltosPointInt pt, AltosMapTransform t) {
		end = lat_lon(pt, t);
	}

	public void pressed(AltosPointInt pt, AltosMapTransform t) {
		start = lat_lon(pt, t);
		end = null;
	}

	public String line_dist() {
		AltosGreatCircle	g = new AltosGreatCircle(start.lat, start.lon,
								 end.lat, end.lon);

		return AltosConvert.distance.show(7, g.distance);
	}
}
