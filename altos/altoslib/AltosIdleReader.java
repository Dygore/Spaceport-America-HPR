/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

public class AltosIdleReader extends AltosFlightReader {
	AltosLink	link;
	boolean		remote;
	AltosCalData	cal_data = null;
	AltosState	state = null;
	AltosIdleFetch	fetch;
	long		next_millis;
	static final long	report_interval = 5 * 1000;
	static final long	minimum_delay = 1 * 1000;

	private void start_link() throws InterruptedException, TimeoutException {
		if (remote) {
			link.start_remote();
		} else
			link.flush_input();
	}

	private boolean stop_link() throws InterruptedException, TimeoutException {
		if (remote)
			link.stop_remote();
		return link.reply_abort;
	}

	public AltosCalData cal_data() {
		if (cal_data == null) {
			try {
				cal_data = new AltosCalData(link.config_data());
			} catch (InterruptedException ie) {
			} catch (TimeoutException te) {
			}
			if (cal_data == null)
				cal_data = new AltosCalData();
		}
		return cal_data;
	}

	public AltosState read() throws InterruptedException, ParseException, AltosCRCException, IOException {
		boolean worked = false;
		boolean aborted = false;

		long	delay = next_millis - System.currentTimeMillis();

		if (delay > 0)
			Thread.sleep(delay);
		next_millis = System.currentTimeMillis() + report_interval;
		try {
			try {
				start_link();
				if (state == null)
					state = new AltosState(cal_data());
				fetch.provide_data(state);
				if (!link.has_error && !link.reply_abort)
					worked = true;
			} catch (TimeoutException te) {
			} catch (AltosUnknownProduct ue) {
				worked = true;
			}
		} finally {
			try {
				aborted = stop_link();
			} catch (TimeoutException te) {
				aborted = true;
			}
			if (worked) {
				if (remote) {
					try {
						state.set_rssi(link.rssi(), 0);
					} catch (TimeoutException te) {
						state.set_rssi(0, 0);
					}
				}
			}
		}

		long	finish = System.currentTimeMillis();

		if (next_millis - finish < minimum_delay)
			next_millis = finish + minimum_delay;

		return state;
	}

	public void close(boolean interrupted) {
		try {
			link.close();
		} catch (InterruptedException ie) {
		}
	}

	public void set_frequency(double frequency) throws InterruptedException, TimeoutException {
		link.set_radio_frequency(frequency);
	}

	public void save_frequency() {
		AltosPreferences.set_frequency(link.serial, link.frequency);
	}

	public void set_callsign(String callsign) throws InterruptedException, TimeoutException {
		link.set_callsign(callsign);
	}

	public AltosIdleReader (AltosLink link, boolean remote)
		throws IOException, InterruptedException, TimeoutException {
		this.link = link;
		this.remote = remote;
		this.next_millis = System.currentTimeMillis();
		fetch = new AltosIdleFetch(link);
	}
}
