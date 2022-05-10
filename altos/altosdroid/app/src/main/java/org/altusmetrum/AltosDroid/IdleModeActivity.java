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

package org.altusmetrum.AltosDroid;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.*;

import org.altusmetrum.altoslib_14.*;

public class IdleModeActivity extends Activity {
	private EditText callsignText;
	private TextView frequencyView;
	private Button connect;
	private Button disconnect;
	private Button reboot;
	private Button igniters;
	private double frequency;

	public static final String EXTRA_IDLE_MODE = "idle_mode";
	public static final String EXTRA_IDLE_RESULT = "idle_result";

	public static final int IDLE_MODE_CONNECT = 1;
	public static final int IDLE_MODE_REBOOT = 2;
	public static final int IDLE_MODE_IGNITERS = 3;
	public static final int IDLE_MODE_DISCONNECT = 4;

	private void done(int type) {
		AltosPreferences.set_callsign(callsign());
		Intent intent = new Intent();
		intent.putExtra(EXTRA_IDLE_RESULT, type);
		setResult(Activity.RESULT_OK, intent);
		finish();
	}

	private String callsign() {
		return callsignText.getEditableText().toString();
	}

	public void connect_idle() {
		done(IDLE_MODE_CONNECT);
	}

	public void disconnect_idle() {
		AltosDebug.debug("Disconnect idle button pressed");
		done(IDLE_MODE_DISCONNECT);
	}

	public void reboot_idle() {
		done(IDLE_MODE_REBOOT);
	}

	public void igniters_idle() {
		done(IDLE_MODE_IGNITERS);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		setTheme(AltosDroid.dialog_themes[AltosDroidPreferences.font_size()]);
		super.onCreate(savedInstanceState);

		// Setup the window
		setContentView(R.layout.idle_mode);

		callsignText = (EditText) findViewById(R.id.set_callsign);
		callsignText.setText(new StringBuffer(AltosPreferences.callsign()));

		frequency = getIntent().getDoubleExtra(AltosDroid.EXTRA_FREQUENCY, 0.0);
		frequencyView = (TextView) findViewById(R.id.frequency);
		frequencyView.setText(String.format("Frequency: %7.3f MHz", frequency));

		connect = (Button) findViewById(R.id.connect_idle);
		connect.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					connect_idle();
				}
			});
		disconnect = (Button) findViewById(R.id.disconnect_idle);
		disconnect.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					disconnect_idle();
				}
			});

		boolean	idle_mode = getIntent().getBooleanExtra(AltosDroid.EXTRA_IDLE_MODE, false);

		if (idle_mode)
			connect.setVisibility(View.GONE);
		else
			disconnect.setVisibility(View.GONE);

		reboot = (Button) findViewById(R.id.reboot_idle);
		reboot.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					reboot_idle();
				}
			});
		igniters = (Button) findViewById(R.id.igniters_idle);
		igniters.setOnClickListener(new OnClickListener() {
				public void onClick(View v) {
					igniters_idle();
				}
			});

		// Set result CANCELED incase the user backs out
		setResult(Activity.RESULT_CANCELED);
	}
}
