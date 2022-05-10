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

public class AltosAdxl375 implements Cloneable {

	private int[]	accels = new int[3];
	private int	axis;

	public static final int X_AXIS = 0;
	public static final int Y_AXIS = 1;
	public static final int Z_AXIS = 2;

	public boolean parse_line(String line) throws NumberFormatException {
		if (line.startsWith("ADXL375 value")) {
			String[] items = line.split("\\s+");
			if (axis == AltosLib.MISSING)
				throw new NumberFormatException("No ADXL375 axis specified");
			if (items.length >= 3) {
				for (int i = 0; i < 3; i++)
					accels[i] = Integer.parseInt(items[2+i]);
				return true;
			}
		}
		return false;
	}

	public AltosAdxl375 clone() {
		AltosAdxl375	n = new AltosAdxl375(axis);

		for (int i = 0; i < 3; i++)
			n.accels[i] = accels[i];
		return n;
	}

	private int accel_along() {
		return accels[axis];
	}

	private int accel_across() {
		if (axis == X_AXIS)
			return accels[Y_AXIS];
		else
			return accels[X_AXIS];
	}

	private int accel_through() {
		return accels[Z_AXIS];
	}

	static public void provide_data(AltosDataListener listener, AltosLink link, boolean three_axis, int imu_type) throws InterruptedException, AltosUnknownProduct {
		try {
			AltosCalData	cal_data = listener.cal_data();
			AltosAdxl375	adxl375 = new AltosAdxl375(link, cal_data.adxl375_axis);

			if (adxl375 != null) {
				int accel = adxl375.accel_along();
				if (!cal_data.adxl375_inverted)
					accel = -accel;
				if (cal_data.pad_orientation == 1)
					accel = -accel;
				listener.set_acceleration(cal_data.acceleration(accel));
				if (three_axis) {
					cal_data.set_imu_type(imu_type);
					double accel_along = cal_data.accel_along(-accel);
					double accel_across = cal_data.accel_across(adxl375.accel_across());
					double accel_through = cal_data.accel_through(adxl375.accel_through());
					listener.set_accel_ground(accel_along,
								  accel_across,
								  accel_through);
					listener.set_accel(accel_along,
							   accel_across,
							   accel_through);
				}
			}
		} catch (TimeoutException te) {
		} catch (NumberFormatException ne) {
		}
	}

	public AltosAdxl375() {
		for (int i = 0; i < 3; i++)
			accels[i] = AltosLib.MISSING;
		axis = AltosLib.MISSING;
	}

	public AltosAdxl375(int axis) {
		this();
		this.axis = axis;
	}

	public AltosAdxl375(AltosLink link, int axis) throws InterruptedException, TimeoutException, NumberFormatException {
		this(axis);
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
