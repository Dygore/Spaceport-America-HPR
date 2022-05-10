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

package org.altusmetrum.telegps;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class TeleGPSStatus extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public class Value {
		JLabel		label;
		JTextField	value;

		void show(AltosState state, AltosListenerState listener_state) {}

		void reset() {
			value.setText("");
		}

		void set_font() {
			label.setFont(AltosUILib.status_font);
			value.setFont(AltosUILib.status_font);
		}

		void setVisible(boolean visible) {
			label.setVisible(visible);
			value.setVisible(visible);
		}

		public Value (GridBagLayout layout, int x, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(5, 5, 5, 5);
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(AltosUILib.status_font);
			label.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField("");
			value.setEditable(false);
			value.setFont(AltosUILib.status_font);
			value.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Call extends Value {
		String	call;

		void show(AltosState state, AltosListenerState listener_state) {
			AltosCalData cal_data = state.cal_data();
			if (cal_data == null)
				System.out.printf("null cal data?\n");
			if (cal_data.callsign != call) {
				value.setText(cal_data.callsign);
				call = cal_data.callsign;
			}
			if (cal_data.callsign == null)
				setVisible(false);
			else
				setVisible(true);
		}

		public void reset() {
			super.reset();
			call = "";
		}

		public Call (GridBagLayout layout, int x) {
			super (layout, x, "Callsign");
		}
	}

	Call call;

	class Serial extends Value {
		int	serial = -1;
		void show(AltosState state, AltosListenerState listener_state) {
			AltosCalData cal_data = state.cal_data();
			if (cal_data.serial != serial) {
				if (cal_data.serial == AltosLib.MISSING)
					value.setText("none");
				else
					value.setText(String.format("%d", cal_data.serial));
				serial = cal_data.serial;
			}
		}

		public void reset() {
			super.reset();
			serial = -1;
		}

		public Serial (GridBagLayout layout, int x) {
			super (layout, x, "Serial");
		}
	}

	Serial serial;

	class Flight extends Value {

		int	last_flight = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			AltosCalData cal_data = state.cal_data();
			if (cal_data.flight != last_flight) {
				if (cal_data.flight == AltosLib.MISSING)
					value.setText("none");
				else
					value.setText(String.format("%d", cal_data.flight));
				last_flight = cal_data.flight;
			}
		}

		public void reset() {
			super.reset();
			last_flight = -1;
		}

		public Flight (GridBagLayout layout, int x) {
			super (layout, x, "Flight");
		}
	}

	Flight flight;

	class RSSI extends Value {
		int	rssi = 10000;

		void show(AltosState state, AltosListenerState listener_state) {
			int	new_rssi = state.rssi();

			if (new_rssi != rssi) {
				value.setText(String.format("%d", new_rssi));
				if (state.rssi == AltosLib.MISSING)
					setVisible(false);
				else
					setVisible(true);
				rssi = new_rssi;
			}
		}

		public void reset() {
			super.reset();
			rssi = 10000;
		}

		public RSSI (GridBagLayout layout, int x) {
			super (layout, x, "RSSI");
		}
	}

	RSSI rssi;

	class LastPacket extends Value {

		long	last_secs = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			if (listener_state.running) {
				long secs = (System.currentTimeMillis() - state.received_time + 500) / 1000;

				if (secs != last_secs) {
					value.setText(String.format("%d", secs));
					last_secs = secs;
				}
			} else {
				value.setText("done");
			}
		}

		void reset() {
			super.reset();
			last_secs = -1;
		}

		void disable() {
			value.setText("");
		}

		public LastPacket(GridBagLayout layout, int x) {
			super (layout, x, "Age");
		}
	}

	LastPacket last_packet;

	public void disable_receive() {
		last_packet.disable();
	}

	public void reset () {
		call.reset();
		serial.reset();
		flight.reset();
		rssi.reset();
		last_packet.reset();
	}

	public void font_size_changed(int font_size) {
		call.set_font();
		serial.set_font();
		flight.set_font();
		rssi.set_font();
		last_packet.set_font();
	}

	public void units_changed(boolean imperial_units) {
	}

	public void show (AltosState state, AltosListenerState listener_state) {
		call.show(state, listener_state);
		serial.show(state, listener_state);
		flight.show(state, listener_state);
		rssi.show(state, listener_state);
		last_packet.show(state, listener_state);
		if (!listener_state.running)
			stop();
	}

	public int height() {
		Dimension d = layout.preferredLayoutSize(this);
		return d.height;
	}

	TeleGPSStatusUpdate	status_update;
	javax.swing.Timer	timer;

	public void start(TeleGPSStatusUpdate status_update) {
		this.status_update = status_update;
		timer = new javax.swing.Timer(100, status_update);
		timer.start();
	}

	public void stop() {
		if (timer != null) {
			timer.stop();
			timer = null;
		}
	}

	public TeleGPSStatus() {
		layout = new GridBagLayout();

		setLayout(layout);

		call = new Call(layout, 0);
		serial = new Serial(layout, 1);
		flight = new Flight(layout, 2);
		rssi = new RSSI(layout, 4);
		last_packet = new LastPacket(layout, 5);
	}
}
