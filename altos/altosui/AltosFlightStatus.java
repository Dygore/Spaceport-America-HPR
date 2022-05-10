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

package altosui;

import java.awt.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class AltosFlightStatus extends JComponent implements AltosFlightDisplay {
	GridBagLayout	layout;

	public abstract class FlightValue {
		JLabel		label;
		JTextField	value;

		void show() {
			label.setVisible(true);
			value.setVisible(true);
		}

		void hide() {
			label.setVisible(false);
			value.setVisible(false);
		}

		abstract void show(AltosState state, AltosListenerState listener_state);

		void reset() {
			value.setText("");
		}

		void set_font() {
			label.setFont(Altos.status_font);
			value.setFont(Altos.status_font);
		}

		void setVisible(boolean visible) {
			label.setVisible(visible);
			value.setVisible(visible);
		}

		public FlightValue (GridBagLayout layout, int x, String text) {
			GridBagConstraints	c = new GridBagConstraints();
			c.insets = new Insets(5, 5, 5, 5);
			c.anchor = GridBagConstraints.CENTER;
			c.fill = GridBagConstraints.BOTH;
			c.weightx = 1;
			c.weighty = 1;

			label = new JLabel(text);
			label.setFont(Altos.status_font);
			label.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 0;
			layout.setConstraints(label, c);
			add(label);

			value = new JTextField("");
			value.setEditable(false);
			value.setFont(Altos.status_font);
			value.setHorizontalAlignment(SwingConstants.CENTER);
			c.gridx = x; c.gridy = 1;
			layout.setConstraints(value, c);
			add(value);
		}
	}

	class Call extends FlightValue {

		String last_call = "";

		boolean same_call(String call) {
			if (last_call == null)
				return call == null;
			else
				return last_call.equals(call);
		}

		void show(AltosState state, AltosListenerState listener_state) {
			if (!same_call(state.cal_data().callsign)) {
				show();
				value.setText(state.cal_data().callsign);
				if (state.cal_data().callsign == null)
					setVisible(false);
				else
					setVisible(true);
				last_call = state.cal_data().callsign;
			}
		}

		public void reset() {
			super.reset();
			last_call = "";
		}

		public Call (GridBagLayout layout, int x) {
			super (layout, x, "Callsign");
		}
	}

	Call call;

	class Serial extends FlightValue {

		int	last_serial = -1;
		void show(AltosState state, AltosListenerState listener_state) {
			AltosCalData	cal_data = state.cal_data();
			if (cal_data.serial != last_serial) {
				show();
				if (cal_data.serial == AltosLib.MISSING)
					value.setText("none");
				else
					value.setText(String.format("%d", cal_data.serial));
				last_serial = cal_data.serial;
			}
		}

		public void reset() {
			super.reset();
			last_serial = -1;
		}

		public Serial (GridBagLayout layout, int x) {
			super (layout, x, "Serial");
		}
	}

	Serial serial;

	class Flight extends FlightValue {

		int	last_flight = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			AltosCalData cal_data = state.cal_data();
			if (cal_data.flight != last_flight) {
				show();
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

	class FlightState extends FlightValue {

		int	last_state = -1;

		void show(AltosState state, AltosListenerState listener_state) {
			if (state.state() != last_state) {
				if (state.state() == AltosLib.ao_flight_stateless)
					hide();
				else {
					show();
					value.setText(state.state_name());
				}
				last_state = state.state();
			}
		}

		public void reset() {
			super.reset();
			last_state = -1;
		}

		public FlightState (GridBagLayout layout, int x) {
			super (layout, x, "State");
		}
	}

	FlightState flight_state;

	class RSSI extends FlightValue {

		int	last_rssi = 10000;

		void show(AltosState state, AltosListenerState listener_state) {
			if (state.rssi() != last_rssi) {
				show();
				value.setText(String.format("%d", state.rssi()));
				if (state.rssi == AltosLib.MISSING)
					setVisible(false);
				else
					setVisible(true);
				last_rssi = state.rssi();
			}
		}

		public void reset() {
			super.reset();
			last_rssi = 10000;
		}

		public RSSI (GridBagLayout layout, int x) {
			super (layout, x, "RSSI");
		}
	}

	RSSI rssi;

	class LastPacket extends FlightValue {

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

		public void reset() {
			super.reset();
			last_secs = -1;
		}

		public LastPacket(GridBagLayout layout, int x) {
			super (layout, x, "Age");
		}
	}

	LastPacket last_packet;

	public void reset () {
		call.reset();
		serial.reset();
		flight.reset();
		flight_state.reset();
		rssi.reset();
		last_packet.reset();
	}

	public void font_size_changed(int font_size) {
		call.set_font();
		serial.set_font();
		flight.set_font();
		flight_state.set_font();
		rssi.set_font();
		last_packet.set_font();
	}

	public void units_changed(boolean imperial_units) {
	}

	public void show (AltosState state, AltosListenerState listener_state) {
		call.show(state, listener_state);
		serial.show(state, listener_state);
		flight.show(state, listener_state);
		flight_state.show(state, listener_state);
		rssi.show(state, listener_state);
		last_packet.show(state, listener_state);
		if (!listener_state.running)
			stop();
	}

	public int height() {
		Dimension d = layout.preferredLayoutSize(this);
		return d.height;
	}

	public String getName() { return "Flight Status"; }

	AltosFlightStatusUpdate	status_update;
	javax.swing.Timer	timer;

	public void start(AltosFlightStatusUpdate status_update) {
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

	public AltosFlightStatus() {
		layout = new GridBagLayout();

		setLayout(layout);

		call = new Call(layout, 0);
		serial = new Serial(layout, 1);
		flight = new Flight(layout, 2);
		flight_state = new FlightState(layout, 3);
		rssi = new RSSI(layout, 4);
		last_packet = new LastPacket(layout, 5);
	}
}
