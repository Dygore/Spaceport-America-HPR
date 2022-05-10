/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
import java.util.*;
import java.text.*;

public class AltosFrequency {
	public double	frequency;
	public String	description;

	public int hashCode() {
		return Double.valueOf(frequency).hashCode();
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;
		if (!(o instanceof AltosFrequency))
			return false;
		AltosFrequency other = (AltosFrequency) o;
		return other.frequency == frequency;
	}

	public String toString() {
		return String.format("%7.3f MHz %-20s",
				     frequency, description);
	}

	public String toShortString() {
		return String.format("%7.3f MHz %s",
				     frequency, description);
	}

	public String frequency_string() {
		return String.format("%7.3f", frequency);
	}

	public boolean close(double f) {
		double	diff = Math.abs(frequency - f);

		return diff < 0.010;
	}
	public AltosFrequency(double f, String d) {
		frequency = f;
		description = d;
	}
	public AltosFrequency() {
		this(0, null);
	}
}
