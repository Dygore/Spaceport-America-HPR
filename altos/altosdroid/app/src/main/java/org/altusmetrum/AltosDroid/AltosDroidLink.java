/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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

import android.os.Handler;

import org.altusmetrum.altoslib_14.*;

public abstract class AltosDroidLink extends AltosLink {

	Handler		handler;

	Thread          input_thread   = null;

	public double frequency() {
		return frequency;
	}

	public int telemetry_rate() {
		return telemetry_rate;
	}

	public void save_frequency() {
		AltosPreferences.set_frequency(0, frequency);
	}

	public void save_telemetry_rate() {
		AltosPreferences.set_telemetry_rate(0, telemetry_rate);
	}

	Object closed_lock = new Object();
	boolean closing = false;
	boolean closed = false;

	public boolean closed() {
		synchronized(closed_lock) {
			return closing;
		}
	}

	void connected() throws InterruptedException {
		input_thread = new Thread(this);
		input_thread.start();

		// Configure the newly connected device for telemetry
		print("~\nE 0\n");
		set_monitor(false);
		AltosDebug.debug("ConnectThread: connected");

		/* Let TelemetryService know we're connected
		 */
		handler.obtainMessage(TelemetryService.MSG_CONNECTED, this).sendToTarget();

		/* Notify other waiting threads that we're connected now
		 */
		notifyAll();
	}

	public void closing() {
		synchronized(closed_lock) {
			AltosDebug.debug("Marked closing true");
			closing = true;
		}
	}

	private boolean actually_closed() {
		synchronized(closed_lock) {
			return closed;
		}
	}

	abstract void close_device();

	public void close() {
		AltosDebug.debug("close(): begin");

		closing();

		flush_output();

		synchronized (closed_lock) {
			AltosDebug.debug("Marked closed true");
			closed = true;
		}

		close_device();

		synchronized(this) {

			if (input_thread != null) {
				AltosDebug.debug("close(): stopping input_thread");
				try {
					AltosDebug.debug("close(): input_thread.interrupt().....");
					input_thread.interrupt();
					AltosDebug.debug("close(): input_thread.join().....");
					input_thread.join();
				} catch (Exception e) {}
				input_thread = null;
			}
			notifyAll();
		}
	}

	abstract int write(byte[] buffer, int len);

	abstract int read(byte[] buffer, int len);

	private static final int buffer_size = 64;

	private byte[] in_buffer = new byte[buffer_size];
	private byte[] out_buffer = new byte[buffer_size];
	private int buffer_len = 0;
	private int buffer_off = 0;
	private int out_buffer_off = 0;

	private byte[] debug_chars = new byte[buffer_size];
	private int debug_off;

	private void debug_input(byte b) {
		if (b == '\n') {
			AltosDebug.debug("            " + new String(debug_chars, 0, debug_off));
			debug_off = 0;
		} else {
			if (debug_off < buffer_size)
				debug_chars[debug_off++] = b;
		}
	}

	private void disconnected() {
		if (closed()) {
			AltosDebug.debug("disconnected after closed");
			return;
		}

		AltosDebug.debug("Sending disconnected message");
		handler.obtainMessage(TelemetryService.MSG_DISCONNECTED, this).sendToTarget();
	}

	public int getchar() {

		if (actually_closed())
			return ERROR;

		while (buffer_off == buffer_len) {
			buffer_len = read(in_buffer, buffer_size);
			if (buffer_len < 0) {
				AltosDebug.debug("ERROR returned from getchar()");
				disconnected();
				return ERROR;
			}
			buffer_off = 0;
		}
//		if (AltosDebug.D)
//			debug_input(in_buffer[buffer_off]);
		return in_buffer[buffer_off++];
	}

	public void flush_output() {
		super.flush_output();

		if (actually_closed()) {
			out_buffer_off = 0;
			return;
		}

		while (out_buffer_off != 0) {
			int	sent = write(out_buffer, out_buffer_off);

			if (sent <= 0) {
				AltosDebug.debug("flush_output() failed");
				out_buffer_off = 0;
				break;
			}

			if (sent < out_buffer_off)
				System.arraycopy(out_buffer, 0, out_buffer, sent, out_buffer_off - sent);

			out_buffer_off -= sent;
		}
	}

	public void putchar(byte c) {
		out_buffer[out_buffer_off++] = c;
		if (out_buffer_off == buffer_size)
			flush_output();
	}

	public void print(String data) {
		byte[] bytes = data.getBytes();
//		AltosDebug.debug(data.replace('\n', '\\'));
		for (byte b : bytes)
			putchar(b);
	}

	public AltosDroidLink(Handler handler) {
		this.handler = handler;
	}
}
