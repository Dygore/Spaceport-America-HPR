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
import java.io.*;

public class AltosMag implements Cloneable {
	public int		along = AltosLib.MISSING;
	public int		across = AltosLib.MISSING;
	public int		through = AltosLib.MISSING;

	public static final int model_hmc5883 = 0;
	public static final int model_mmc5983 = 1;

	public int mag_model = AltosLib.MISSING;

	public static final double counts_per_gauss_hmc5883 = 1090;
	public static final double counts_per_gauss_mmc5983 = 4096;

	public static double counts_per_gauss(int imu_type, int mag_model) {
		switch(mag_model) {
		case AltosLib.model_hmc5883:
			return counts_per_gauss_hmc5883;
		case AltosLib.model_mmc5983:
			return counts_per_gauss_mmc5983;
		}
		switch (imu_type) {
		case AltosIMU.imu_type_telemega_v1_v2:
		case AltosIMU.imu_type_easymega_v1:
			return counts_per_gauss_hmc5883;
		}

		return AltosIMU.counts_per_gauss(imu_type, mag_model);
	}

	public static double convert_gauss(double counts, int imu_type, int mag_model) {
		double cpg = counts_per_gauss(imu_type, mag_model);

		if (cpg == AltosLib.MISSING)
			return AltosLib.MISSING;

		return counts / cpg;
	}

	public boolean parse_string(String line) {

		if (line.startsWith("X:")) {

			String[] items = line.split("\\s+");

			mag_model = model_hmc5883;

			if (items.length >= 6) {
				across = Integer.parseInt(items[1]);
				through = Integer.parseInt(items[3]);
				along = Integer.parseInt(items[5]);
			}
			return true;
		}
		if (line.startsWith("MMC5983:")) {
			String[] items = line.split("\\s+");

			mag_model = model_mmc5983;

			if (items.length >= 4) {
				along = Integer.parseInt(items[1]);
				across = Integer.parseInt(items[2]);
				through = Integer.parseInt(items[3]);
			}
			return true;
		}

		return false;
	}

	public AltosMag clone() {
		AltosMag n = new AltosMag();

		n.along = along;
		n.across = across;
		n.through = through;
		return n;
	}

	public AltosMag() {
	}

	static public void provide_data(AltosDataListener listener, AltosLink link) throws InterruptedException {
		try {
			AltosMag	mag = new AltosMag(link);
			AltosCalData	cal_data = listener.cal_data();

			if (mag != null) {
				if (mag.mag_model != AltosLib.MISSING)
					cal_data.set_mag_model(mag.mag_model);
				listener.set_mag(cal_data.mag_along(mag.along),
						 cal_data.mag_across(mag.across),
						 cal_data.mag_through(mag.through));
			}
		} catch (TimeoutException te) {
		}
	}

	public AltosMag(AltosLink link) throws InterruptedException, TimeoutException {
		this();
		link.printf("M\n");
		for (;;) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null) {
				throw new TimeoutException();
			}
			if (parse_string(line))
				break;
		}
	}
}
