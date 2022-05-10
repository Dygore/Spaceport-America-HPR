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

import java.util.concurrent.*;

public class AltosMma655x implements Cloneable {

	private int	accel;

	public boolean parse_line(String line) throws NumberFormatException {
		if (line.startsWith("MMA655X value")) {
			String[] items = line.split("\\s+");
			if (items.length >= 3) {
				accel = Integer.parseInt(items[2]);
				return true;
			}
		}
		return false;
	}

	public AltosMma655x() {
		accel = AltosLib.MISSING;
	}

	public AltosMma655x clone() {
		AltosMma655x	n = new AltosMma655x();

		n.accel = accel;
		return n;
	}

	static public void provide_data(AltosDataListener listener, AltosLink link) throws InterruptedException, AltosUnknownProduct {
		try {
			AltosMma655x	mma655x = new AltosMma655x(link);
			AltosCalData	cal_data = listener.cal_data();

			if (mma655x != null) {
				int accel = mma655x.accel;
				if (cal_data.mma655x_inverted)
					accel = 4095 - accel;
				if (cal_data.pad_orientation == AltosLib.AO_PAD_ORIENTATION_ANTENNA_DOWN)
					accel = 4095 - accel;
				listener.set_acceleration(cal_data.acceleration(accel));
			}
		} catch (TimeoutException te) {
		} catch (NumberFormatException ne) {
		}
	}

	public AltosMma655x(AltosLink link) throws InterruptedException, TimeoutException, NumberFormatException {
		this();
		link.printf("A\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();
			if (parse_line(line))
				break;
		}
	}
}
