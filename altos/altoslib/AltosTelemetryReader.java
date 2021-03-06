/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

import java.text.*;
import java.io.*;
import java.util.concurrent.*;

public class AltosTelemetryReader extends AltosFlightReader {
	AltosLink		link;
	AltosLog		log;
	double			frequency;
	int			telemetry;
	int			telemetry_rate;
	private AltosState	state = null;
	private AltosCalData	cal_data = null;

	LinkedBlockingQueue<AltosLine> telem;

	public AltosState read() throws InterruptedException, ParseException, AltosCRCException, IOException {
		AltosLine l;
		do {
			l = telem.take();
			if (l.line == null)
				throw new IOException("IO error");
		} while (!link.get_monitor());
		AltosTelemetry	telem = AltosTelemetry.parse(l.line);
		if (state == null)
			state = new AltosState(cal_data());
		telem.provide_data(state);
		return state;
	}

	public AltosCalData cal_data() {
		if (cal_data == null)
			cal_data = new AltosCalData();
		return cal_data;
	}

	public void flush() {
		telem.clear();
	}

	public void reset() {
		flush();
		state = null;
		cal_data = null;
	}

	public void close(boolean interrupted) {

		link.remove_monitor(telem);
		log.close();
		try {
			link.close();
		} catch (InterruptedException ie) {
		}
	}

	public void set_frequency(double in_frequency) throws InterruptedException, TimeoutException {
		frequency = in_frequency;
		link.set_radio_frequency(frequency);
	}

	public boolean supports_telemetry(int telemetry) {

		try {
			/* Version 1.0 or later firmware supports all telemetry formats */
			if (link.config_data().compare_version("1.0") >= 0)
				return true;

			/* Version 0.9 firmware only supports 0.9 telemetry */
			if (link.config_data().compare_version("0.9") >= 0) {
				if (telemetry == AltosLib.ao_telemetry_0_9)
					return true;
				else
					return false;
			}

			/* Version 0.8 firmware only supports 0.8 telemetry */
			if (telemetry == AltosLib.ao_telemetry_0_8)
				return true;
			else
				return false;
		} catch (InterruptedException ie) {
			return false;
		} catch (TimeoutException te) {
			return true;
		}
	}

	public boolean supports_telemetry_rate(int telemetry_rate) {
		try {
			/* Version 1.4.1.1 supports all rates, older versions don't */
			if (link.config_data().compare_version("1.4.1.1") >= 0)
				return true;

			if (telemetry_rate == AltosLib.ao_telemetry_rate_38400)
				return true;
			else
				return false;
		} catch (InterruptedException ie) {
			return false;
		} catch (TimeoutException te) {
			return true;
		}
	}

	public void save_frequency() {
		AltosPreferences.set_frequency(link.serial, frequency);
	}

	public void set_telemetry(int in_telemetry) {
		telemetry = in_telemetry;
		link.set_telemetry(telemetry);
	}

	public void save_telemetry() {
		AltosPreferences.set_telemetry(link.serial, telemetry);
	}

	public void set_telemetry_rate(int in_telemetry_rate) {
		telemetry_rate = in_telemetry_rate;
		link.set_telemetry_rate(telemetry_rate);
	}

	public void save_telemetry_rate() {
		AltosPreferences.set_telemetry_rate(link.serial, telemetry_rate);
	}

	public void set_monitor(boolean monitor) {
		link.set_monitor(monitor);
	}

	public File backing_file() {
		return log.file();
	}

	public boolean has_monitor_battery() {
		return link.has_monitor_battery();
	}

	public double monitor_battery() throws InterruptedException {
		return link.monitor_battery();
	}

	public AltosTelemetryReader (AltosLink in_link)
		throws IOException, InterruptedException, TimeoutException {
		link = in_link;
		boolean success = false;
		try {
			log = new AltosLog(link);
			name = link.name;
			telem = new LinkedBlockingQueue<AltosLine>();
			frequency = AltosPreferences.frequency(link.serial);
			set_frequency(frequency);
			telemetry = AltosPreferences.telemetry(link.serial);
			set_telemetry(telemetry);
			telemetry_rate = AltosPreferences.telemetry_rate(link.serial);
			set_telemetry_rate(telemetry_rate);
			link.add_monitor(telem);
			success = true;
		} finally {
			if (!success)
				close(true);
		}
	}
}
