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

import java.io.*;
import java.text.*;
import java.util.concurrent.*;

/*
 * This creates a thread to capture telemetry data and write it to
 * a log file
 */
public class AltosLog implements Runnable {

	LinkedBlockingQueue<AltosLine>	input_queue;
	LinkedBlockingQueue<String>	pending_queue;
	int				serial;
	int				flight;
	int				receiver_serial;
	FileWriter			log_file;
	Thread				log_thread;
	AltosFile			file;
	AltosLink			link;
	AltosLogTrace			trace;

	private void trace(String format, Object ... arguments) {
		if (trace != null)
			trace.trace(format, arguments);
	}

	private void close_log_file() {
		if (log_file != null) {
			try {
				log_file.close();
			} catch (IOException io) {
			}
			log_file = null;
		}
	}

	public void close() {
		link.remove_monitor(input_queue);
		close_log_file();
		if (log_thread != null) {
			log_thread.interrupt();
			log_thread = null;
		}
	}

	public File file() {
		return file;
	}

	boolean open (AltosCalData cal_data) throws IOException, InterruptedException {
		trace("open serial %d flight %d receiver_serial %d",
		      cal_data.serial,
		      cal_data.flight,
		      cal_data.receiver_serial);

		AltosFile	a = new AltosFile(cal_data);

		trace("open file %s\n", a.getPath());

		try {
			log_file = new FileWriter(a, true);
		} catch (IOException ie) {
			log_file = null;
			trace("open file failed\n");
			if (trace != null)
				trace.open_failed(a);
		}
		if (log_file != null) {
			trace("open file success\n");
			while (!pending_queue.isEmpty()) {
				String s = pending_queue.take();
				log_file.write(s);
				log_file.write('\n');
			}
			log_file.flush();
			file = a;
			AltosPreferences.set_logfile(link.serial, file);
		}
		return log_file != null;
	}

	public void run () {
		trace("log run start\n");
		try {
			AltosConfigData	receiver_config = link.config_data();
			AltosCalData	cal_data = new AltosCalData();
			AltosState	state = null;
			cal_data.set_receiver_serial(receiver_config.serial);
			for (;;) {
				AltosLine	line = input_queue.take();
				if (line.line == null)
					continue;
				try {
					AltosTelemetry	telem = AltosTelemetry.parse(line.line);
					if (state == null)
						state = new AltosState(cal_data);
					telem.provide_data(state);

					if (cal_data.serial != serial ||
					    cal_data.flight != flight ||
					    log_file == null)
					{
						close_log_file();
						serial = cal_data.serial;
						flight = cal_data.flight;
						state = null;
						if (cal_data.serial != AltosLib.MISSING &&
						    cal_data.flight != AltosLib.MISSING)
							open(cal_data);
					}
				} catch (ParseException pe) {
				} catch (AltosCRCException ce) {
				}
				if (log_file != null) {
					log_file.write(line.line);
					log_file.write('\n');
					log_file.flush();
				} else
					pending_queue.put(line.line);
			}
		} catch (InterruptedException ie) {
			trace("interrupted exception\n");
		} catch (TimeoutException te) {
			trace("timeout exception\n");
		} catch (IOException ie) {
			trace("io exception %s message %s\n", ie.toString(), ie.getMessage());
		}
		trace("log run done\n");
		close();
	}

	public AltosLog (AltosLink link, AltosLogTrace trace) {
		pending_queue = new LinkedBlockingQueue<String> ();
		input_queue = new LinkedBlockingQueue<AltosLine> ();
		link.add_monitor(input_queue);
		serial = -1;
		flight = -1;
		this.trace = trace;
		this.link = link;
		log_file = null;
		log_thread = new Thread(this);
		log_thread.start();
	}

	public AltosLog (AltosLink link) {
		this(link, null);
	}
}
