/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.concurrent.*;

public class AltosAccelCal implements Runnable {

	AltosLink		link;
	AltosAccelCalListener	listener;

	boolean			remote;
	boolean			close_on_exit;
	double			frequency;
	String			callsign;

	Thread			accel_thread;

	AltosConfigData		config_data;

	public static final int	phase_antenna_up = 0;
	public static final int	phase_antenna_down = 1;

	void start_link() throws InterruptedException, TimeoutException {
		if (remote) {
			link.set_radio_frequency(frequency);
			link.set_callsign(callsign);
			link.start_remote();
		} else
			link.flush_input();
	}

	boolean stop_link() throws InterruptedException, TimeoutException {
		if (remote)
			link.stop_remote();
		return link.reply_abort;
	}

	public void set_frequency(double in_frequency) {
		frequency = in_frequency;
		link.abort_reply();
	}

	public void set_callsign(String in_callsign) {
		callsign = in_callsign;
		link.abort_reply();
	}

	public void abort() throws InterruptedException {
		while (accel_thread.isAlive()) {
			accel_thread.interrupt();
			link.abort_reply();
			Thread.sleep(100);
		}
		accel_thread.join();
	}

	static private final String press_msg = "press a key...";

	private Semaphore ui_signal_semaphore;
	private boolean ui_signal_reply;

	public void signal(boolean reply) {
		System.out.printf("Signal cal semaphore %b\n", reply);
		ui_signal_reply = reply;
		ui_signal_semaphore.release();
	}

	private boolean wait_signal() throws InterruptedException {
		System.out.printf("\twait for cal signal...\n");
		ui_signal_semaphore.acquire();
		System.out.printf("\tgot cal signal %b\n", ui_signal_reply);
		return ui_signal_reply;
	}

	private boolean wait_press(int timeout) throws InterruptedException {
		for (;;) {
			String line = link.get_reply(timeout);
			if (line == null) {
				System.out.printf("get_reply timeout\n");
				return false;
			}
			System.out.printf("got line %s\n", line);
			if (line.contains(press_msg))
				return true;
			if (line.contains("Invalid"))
				return false;
			if (line.contains("Syntax"))
				return false;
			if (line.contains("Calibrating"))
				listener.message(this, line);
		}
	}

	static final int cal_timeout = 20 * 1000;

	public void run() {
		System.out.printf("start accel cal procedure\n");
		try {
			AltosConfigData new_config = null;

			try {
				start_link();
				config_data = link.config_data();

				/* set back to antenna up for calibration */
				if (config_data.pad_orientation != 0)
					link.printf("c o 0\n");

				/* Start calibration */
				try {
					System.out.printf("*** start cal\n");
					link.set_match(press_msg);
					link.printf("c a 0\n");
					System.out.printf("*** wait press\n");
					if (!wait_press(cal_timeout))
						throw new TimeoutException("timeout");
					System.out.printf("*** set_phase antenna_up\n");
					listener.set_phase(this, phase_antenna_up);
					System.out.printf("*** wait_signal\n");
					if (!wait_signal())
						throw new InterruptedException("aborted");
					link.set_match(press_msg);
					System.out.printf("*** send newline\n");
					link.printf("\n");
					System.out.printf("*** wait press\n");
					if (!wait_press(cal_timeout))
						throw new TimeoutException("timeout");
					System.out.printf("***set_phase antenna_down\n");
					listener.set_phase(this, phase_antenna_down);
					System.out.printf("*** wait_signal\n");
					if (!wait_signal())
						throw new InterruptedException("aborted");
					System.out.printf("*** send newline and version command\n");
					link.printf("\nv\n");
				} catch (TimeoutException e) {
					throw e;
				} catch (InterruptedException e) {
					throw e;
				}
				link.set_match(null);

				boolean worked = true;
				for (;;) {
					String line = link.get_reply(cal_timeout);
					if (line == null)
						throw new TimeoutException();
					System.out.printf("*** waiting for finish: %s\n", line);
					if (line.contains("Invalid"))
						worked = false;
					if (line.contains("software-version"))
						break;
					if (line.contains("Calibrating"))
						listener.message(this, line);
				}
				System.out.printf("*** worked: %b\n", worked);
				if (worked)
					new_config = new AltosConfigData(link);
			} finally {
				int plus = config_data.accel_cal_plus(config_data.pad_orientation);
				int minus = config_data.accel_cal_minus(config_data.pad_orientation);
				System.out.printf("Restore orientation %d +g %d -g %d\n",
						  config_data.pad_orientation,
						  plus, minus);
				if (config_data.pad_orientation != AltosLib.MISSING)
					link.printf("c o %d\n", config_data.pad_orientation);
				if (plus != AltosLib.MISSING && minus != AltosLib.MISSING && plus != 0) {
					if (plus < 0)
						plus = 65536 + plus;
					if (minus < 0)
						minus = 65536 + minus;
					if (config_data.accel_zero_along != AltosLib.MISSING)
						link.printf("c a %d %d %d %d %d\n",
							    plus, minus,
							    config_data.accel_zero_along,
							    config_data.accel_zero_across,
							    config_data.accel_zero_through);
					else
						link.printf("c a %d %d\n", plus, minus);
				}
				link.flush_output();
				stop_link();
			}
			if (new_config != null) {
				int plus = new_config.accel_cal_plus(AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP);
				int minus = new_config.accel_cal_minus(AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP);
				System.out.printf("*** +1g %d -1g %d\n", plus, minus);
				listener.cal_done(this, plus, minus);
				if (!wait_signal())
					throw new InterruptedException("aborted");
			} else
				listener.error(this, "Calibration failed");
		} catch (TimeoutException te) {
			System.out.printf("timeout");
			listener.error(this, "timeout");
		} catch (InterruptedException ie) {
			System.out.printf("interrupted\n");
			listener.error(this, "interrupted");
		}
	}

	public void start() {
		accel_thread = new Thread(this);
		listener.set_thread(this, accel_thread);
		accel_thread.start();
	}

	public AltosAccelCal(AltosLink link, AltosAccelCalListener listener) {
		this.link = link;
		this.listener = listener;
		ui_signal_semaphore = new Semaphore(0);
	}
}
