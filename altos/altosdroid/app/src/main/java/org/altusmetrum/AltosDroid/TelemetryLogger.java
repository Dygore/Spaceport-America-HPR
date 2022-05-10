/*
 * Copyright © 2021 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

import java.io.*;

import org.altusmetrum.altoslib_14.*;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Environment;

public class TelemetryLogger implements AltosLogTrace {
	private TelemetryService service = null;
	private AltosLink link    = null;
	private AltosLog  logger  = null;

	private BroadcastReceiver mExternalStorageReceiver;

	/* AltosLogTrace interface */
	public void trace(String format, Object ... arguments) {
		AltosDebug.debug(format, arguments);
	}

	public void open_failed(File file) {
		service.send_file_failed_to_clients(file);
	}

	public TelemetryLogger(TelemetryService in_service, AltosLink in_link) {
		service = in_service;
		link    = in_link;

		startWatchingExternalStorage();
	}

	public void stop() {
		stopWatchingExternalStorage();
		close();
	}

	private void close() {
		if (logger != null) {
			AltosDebug.debug("Shutting down Telemetry Logging");
			logger.close();
			logger = null;
		}
	}

	void handleExternalStorageState() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			if (logger == null) {
				AltosDebug.debug("Starting up Telemetry Logging");
				logger = new AltosLog(link,this);
			}
		} else {
			AltosDebug.debug("External Storage not present - stopping");
			close();
		}
	}

	void startWatchingExternalStorage() {
		mExternalStorageReceiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				handleExternalStorageState();
			}
		};
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_MEDIA_MOUNTED);
		filter.addAction(Intent.ACTION_MEDIA_REMOVED);
		service.registerReceiver(mExternalStorageReceiver, filter);
		handleExternalStorageState();
	}

	void stopWatchingExternalStorage() {
		service.unregisterReceiver(mExternalStorageReceiver);
	}

}
