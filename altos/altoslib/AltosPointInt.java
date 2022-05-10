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

public class AltosPointInt {
	public int	x, y;

	public int hashCode() {
		return  x ^ y;
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;

		if (!(o instanceof AltosPointInt))
			return false;

		AltosPointInt n = (AltosPointInt) o;

		return n.x == x && n.y == y;
	}

	public AltosPointInt(int x, int y) {
		this.x = x;
		this.y = y;
	}

	public AltosPointInt(double x, double y) {
		this.x = (int) (x + 0.5);
		this.y = (int) (y + 0.5);
	}

	public AltosPointInt(AltosPointDouble pt_d) {
		this.x = (int) (pt_d.x + 0.5);
		this.y = (int) (pt_d.y + 0.5);
	}
}
