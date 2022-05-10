/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 * Copyright © 2012 Mike Beattie <mike@ethernal.org>
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


package org.altusmetrum.AltosDroid;

import java.text.*;
import java.io.*;
import java.util.concurrent.*;
import android.os.Handler;

import org.altusmetrum.altoslib_14.*;


public class TelemetryReader extends Thread {

	int         crc_errors;

	Handler     handler;

	AltosLink   link;

	LinkedBlockingQueue<AltosLine> telemQueue;

	public AltosTelemetry read() throws ParseException, AltosCRCException, InterruptedException, IOException {
		AltosLine l = telemQueue.take();
		if (l.line == null)
			throw new IOException("IO error");
		AltosTelemetry telem = AltosTelemetryLegacy.parse(l.line);
		return telem;
	}

	public void close() {
		link.remove_monitor(telemQueue);
		link = null;
		telemQueue.clear();
		telemQueue = null;
	}

	public void run() {
		try {
			AltosDebug.debug("starting loop");
			while (telemQueue != null) {
				try {
					AltosTelemetry	telem = read();
					telem.set_frequency(link.frequency);
					handler.obtainMessage(TelemetryService.MSG_TELEMETRY, telem).sendToTarget();
				} catch (ParseException pp) {
					AltosDebug.error("Parse error: %d \"%s\"", pp.getErrorOffset(), pp.getMessage());
				} catch (AltosCRCException ce) {
					++crc_errors;
					handler.obtainMessage(TelemetryService.MSG_CRC_ERROR, new Integer(crc_errors)).sendToTarget();
				}
			}
		} catch (InterruptedException ee) {
		} catch (IOException ie) {
			AltosDebug.error("IO exception in telemetry reader");
			handler.obtainMessage(TelemetryService.MSG_DISCONNECTED, link).sendToTarget();
		} finally {
			close();
		}
	}

	public TelemetryReader (AltosLink in_link, Handler in_handler) {
		AltosDebug.debug("connected TelemetryReader create started");
		link    = in_link;
		handler = in_handler;

		telemQueue = new LinkedBlockingQueue<AltosLine>();
		link.add_monitor(telemQueue);
		link.set_telemetry(AltosLib.ao_telemetry_standard);

		AltosDebug.debug("connected TelemetryReader created");
	}
}
