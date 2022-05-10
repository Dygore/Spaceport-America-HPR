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

public class AltosMs5607 {
	public int	reserved = AltosLib.MISSING;
	public int	sens = AltosLib.MISSING;
	public int	off = AltosLib.MISSING;
	public int	tcs = AltosLib.MISSING;
	public int	tco = AltosLib.MISSING;
	public int	tref = AltosLib.MISSING;
	public int	tempsens = AltosLib.MISSING;
	public int	crc = AltosLib.MISSING;
	private boolean	ms5611 = false;

	public boolean valid_config() {
		return reserved != AltosLib.MISSING &&
			sens != AltosLib.MISSING &&
			off != AltosLib.MISSING &&
			tcs != AltosLib.MISSING &&
			tco != AltosLib.MISSING &&
			tref != AltosLib.MISSING &&
			tempsens != AltosLib.MISSING &&
			crc  != AltosLib.MISSING;
	}

	public AltosPresTemp pres_temp(int raw_pres, int raw_temp) {
		int	dT;
		int	TEMP;
		long	OFF;
		long	SENS;
		int	P;

		if (raw_pres == AltosLib.MISSING || raw_temp == AltosLib.MISSING)
			return new AltosPresTemp(AltosLib.MISSING, AltosLib.MISSING);

		dT = raw_temp - ((int) tref << 8);

		TEMP = (int) (2000 + (((long) dT * (long) tempsens) >> 23));

		if (ms5611) {
			OFF = ((long) off << 16) + (((long) tco * (long) dT) >> 7);

			SENS = ((long) sens << 15) + (((long) tcs * (long) dT) >> 8);
		} else {
			OFF = ((long) off << 17) + (((long) tco * (long) dT) >> 6);

			SENS = ((long) sens << 16) + (((long) tcs * (long) dT) >> 7);
		}

		if (TEMP < 2000) {
			int	T2 = (int) (((long) dT * (long) dT) >> 31);
			int TEMPM = TEMP - 2000;
			long OFF2 = ((long) 61 * (long) TEMPM * (long) TEMPM) >> 4;
			long SENS2 = (long) 2 * (long) TEMPM * (long) TEMPM;
			if (TEMP < -1500) {
				int TEMPP = TEMP + 1500;
				long TEMPP2 = (long) TEMPP * (long) TEMPP;
				OFF2 = OFF2 + 15 * TEMPP2;
				SENS2 = SENS2 + 8 * TEMPP2;
			}
			TEMP -= T2;
			OFF -= OFF2;
			SENS -= SENS2;
		}

		P = (int) (((((long) raw_pres * SENS) >> 21) - OFF) >> 15);

		return new AltosPresTemp(P, TEMP / 100.0);
	}

	public AltosPresTemp pres_temp(AltosLink link) throws InterruptedException, TimeoutException {
		int	raw_pres = AltosLib.MISSING;
		int	raw_temp = AltosLib.MISSING;
		boolean	done = false;

		link.printf("B\n");
		while (!done) {
			String line = link.get_reply_no_dialog(5000);
			if (line == null)
				throw new TimeoutException();

			String[] items = line.split("\\s+");
			if (line.startsWith("Pressure:")) {
				if (items.length >= 2) {
					raw_pres = Integer.parseInt(items[1]);
				}
			} else if (line.startsWith("Temperature:")) {
				if (items.length >= 2)
					raw_temp = Integer.parseInt(items[1]);
			} else if (line.startsWith("Altitude:")) {
				done = true;
			}
		}
		return pres_temp(raw_pres, raw_temp);
	}

	public AltosMs5607(boolean ms5611) {
		this.ms5611 = ms5611;
	}

	public AltosMs5607() {
		this(false);
	}

	public AltosMs5607(AltosMs5607 old) {
		reserved = old.reserved;
		sens = old.sens;
		off = old.off;
		tcs = old.tcs;
		tco = old.tco;
		tref = old.tref;
		tempsens = old.tempsens;
		crc = old.crc;
	}

	static public void provide_data(AltosDataListener listener, AltosLink link) throws InterruptedException {
		try {
			AltosCalData	cal_data = listener.cal_data();
			AltosMs5607	ms5607 = cal_data.ms5607;

			if (ms5607 != null) {
				AltosPresTemp	pt = ms5607.pres_temp(link);
				listener.set_temperature(pt.temp);
				listener.set_pressure(pt.pres);
			}
		} catch (TimeoutException te) {
		}
	}

	public AltosMs5607(AltosConfigData config_data) {
		this(config_data.ms5607());
	}

	public AltosMs5607 (AltosLink link, AltosConfigData config_data) throws InterruptedException, TimeoutException {
		this(config_data);
	}
}
