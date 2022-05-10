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

public class AltosMapTransform {

	double	scale_x, scale_y;

	double	offset_x, offset_y;

	int	width, height;

	public AltosLatLon lat_lon (AltosPointDouble point) {
		double lat, lon;
		double rads;

		lon = point.x/scale_x;
		rads = 2 * Math.atan(Math.exp(-point.y/scale_y));
		lat = Math.toDegrees(rads - Math.PI/2);

		return new AltosLatLon(lat,lon);
	}

	public AltosLatLon lat_lon (AltosPointInt point) {
		return lat_lon(new AltosPointDouble(point.x, point.y));
	}

	public AltosPointDouble screen_point(AltosPointInt screen) {
		return new AltosPointDouble(screen.x + offset_x, screen.y + offset_y);
	}

	public AltosPointDouble screen_point(AltosPointDouble screen) {
		return new AltosPointDouble(screen.x + offset_x, screen.y + offset_y);
	}

	public double hypot(AltosLatLon a, AltosLatLon b) {
		AltosPointDouble	a_pt = point(a);
		AltosPointDouble	b_pt = point(b);

		return Math.hypot(a_pt.x - b_pt.x, a_pt.y - b_pt.y);
	}

	public AltosLatLon screen_lat_lon(AltosPointInt screen) {
		return lat_lon(screen_point(screen));
	}

	public AltosLatLon screen_lat_lon(AltosPointDouble screen) {
		return lat_lon(screen_point(screen));
	}

	public  AltosPointDouble point(double lat, double lon) {
		double x, y;
		double e;

		x = lon * scale_x;

		e = Math.sin(Math.toRadians(lat));
		e = Math.max(e,-(1-1.0E-15));
		e = Math.min(e,  1-1.0E-15 );

		y = 0.5*Math.log((1+e)/(1-e))*-scale_y;

		return new AltosPointDouble(x, y);
	}

	public AltosPointDouble point(AltosLatLon lat_lon) {
		return point(lat_lon.lat, lat_lon.lon);
	}

	public AltosPointDouble screen(AltosPointDouble point) {
		return new AltosPointDouble(point.x - offset_x, point.y - offset_y);
	}

	public AltosPointInt screen(AltosPointInt point) {
		return new AltosPointInt((int) (point.x - offset_x + 0.5),
					 (int) (point.y - offset_y + 0.5));
	}

	public AltosRectangle screen(AltosMapRectangle map_rect) {
		AltosPointDouble	ul = screen(map_rect.ul);
		AltosPointDouble	lr = screen(map_rect.lr);

		return new AltosRectangle((int) ul.x, (int) ul.y, (int) (lr.x - ul.x), (int) (lr.y - ul.y));
	}

	public AltosPointDouble screen(AltosLatLon lat_lon) {
		return screen(point(lat_lon));
	}

	public AltosPointDouble screen(double lat, double lon) {
		return screen(point(lat, lon));
	}

	/* Return first longitude value which ends up on-screen */
	public double first_lon(double lon) {
		/* Find a longitude left of the screen */
		for (;;) {
			double x = lon * scale_x - offset_x;
			if (x < 0)
				break;
			lon -= 360.0;
		}
		/* Find the first longitude on the screen */
		for (;;) {
			double x = lon * scale_x - offset_x;
			if (x >= 0)
				break;
			lon += 360.0;
		}
		return lon;
	}

	/* Return last longitude value which ends up on-screen */
	public double last_lon(double lon) {
		lon = first_lon(lon);

		for(;;) {
			double next_lon = lon + 360.0;
			double next_x = next_lon * scale_x - offset_x;
			if (next_x >= width)
				break;
			lon = next_lon;
		}
		return lon;
	}

	private boolean has_location;

	public boolean has_location() {
		return has_location;
	}

	public AltosMapTransform(int width, int height, int zoom, AltosLatLon centre_lat_lon) {
		scale_x = 256/360.0 * Math.pow(2, zoom);
		scale_y = 256/(2.0*Math.PI) * Math.pow(2, zoom);

		this.width = width;
		this.height = height;

		AltosPointDouble centre_pt = point(centre_lat_lon);

		has_location = (centre_lat_lon.lat != 0 || centre_lat_lon.lon != 0);
		offset_x = centre_pt.x - width / 2.0;
		offset_y = centre_pt.y - height / 2.0;
	}

	public static double lon_from_distance(double lat, double distance) {
		double c = AltosGreatCircle.earth_radius * Math.cos(lat * Math.PI / 180) * 2 * Math.PI;

		if (c < 10)
			return 0;
		return distance/c * 360.0;
	}
}
