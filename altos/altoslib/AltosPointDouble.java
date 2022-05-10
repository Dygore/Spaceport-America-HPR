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

public class AltosPointDouble {
	public double	x, y;

	public int hashCode() {
		return Double.valueOf(x).hashCode() ^ Double.valueOf(y).hashCode();
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;

		if (!(o instanceof AltosPointDouble))
			return false;

		AltosPointDouble n = (AltosPointDouble) o;

		return n.x == x && n.y == y;
	}

	public AltosPointDouble(double x, double y) {
		this.x = x;
		this.y = y;
	}

	public AltosPointDouble(int x, int y) {
		this.x = x;
		this.y = y;
	}

	public AltosPointDouble(AltosPointInt p) {
		this.x = p.x;
		this.y = p.y;
	}
}
