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

public class AltosMapRectangle {
	AltosLatLon	ul, lr;

	public AltosMapRectangle(AltosLatLon a, AltosLatLon b) {
		double	ul_lat, ul_lon;
		double	lr_lat, lr_lon;

		if (a.lat > b.lat) {
			ul_lat = a.lat;
			lr_lat = b.lat;
		} else {
			ul_lat = b.lat;
			lr_lat = a.lat;
		}
		if (a.lon < b.lon) {
			ul_lon = a.lon;
			lr_lon = b.lon;
		} else {
			ul_lon = b.lon;
			lr_lon = a.lon;
		}

		ul = new AltosLatLon(ul_lat, ul_lon);
		lr = new AltosLatLon(lr_lat, lr_lon);
	}
}
