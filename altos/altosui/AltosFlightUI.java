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
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class AltosFlightUI extends AltosUIFrame implements AltosFlightDisplay {
	AltosVoice		voice;
	AltosFlightReader	reader;
	AltosDisplayThread	thread;

	LinkedList<AltosFlightDisplay> displays;

	JTabbedPane	pane;

	AltosPad	pad;
	AltosIgnitor	igniter;
	AltosAscent	ascent;
	AltosDescent	descent;
	AltosLanded	landed;
	AltosCompanionInfo	companion;
	AltosUIMap      sitemap;
	boolean		has_map;
	boolean		has_companion;
	boolean		has_state;
	boolean		has_igniter;

	private AltosFlightStatus flightStatus;
	private AltosInfoTable flightInfo;

	boolean exit_on_close = false;

	JComponent cur_tab = null;
	JComponent which_tab(AltosState state) {
		if (state.state() < Altos.ao_flight_boost)
			return pad;
		if (state.state() <= Altos.ao_flight_coast)
			return ascent;
		if (state.state() <= Altos.ao_flight_main)
			return descent;
		if (state.state() == AltosLib.ao_flight_stateless)
			return descent;
		return landed;
	}

	void stop_display() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
	}

	void disconnect() {
		stop_display();
	}

	public void reset() {
		for (AltosFlightDisplay d : displays)
			d.reset();
	}

	public void font_size_changed(int font_size) {
		for (AltosFlightDisplay d : displays)
			d.font_size_changed(font_size);
	}

	public void units_changed(boolean imperial_units) {
		for (AltosFlightDisplay d : displays)
			d.units_changed(imperial_units);
	}

	AltosFlightStatusUpdate	status_update;

	public void show(AltosState state, AltosListenerState listener_state) {
		status_update.saved_state = state;
		status_update.saved_listener_state = listener_state;

		if (state == null)
			state = new AltosState(new AltosCalData());

		if (state.state() != Altos.ao_flight_startup) {
			if (!has_state) {
				pane.setTitleAt(0, "Launch Pad");
				pane.add(ascent, 1);
				pane.add(descent, 2);
				pane.add(landed, 3);
				has_state = true;
			}
		}

		JComponent tab = which_tab(state);
		if (tab != cur_tab) {
			if (cur_tab == pane.getSelectedComponent())
				pane.setSelectedComponent(tab);
			cur_tab = tab;
		}

		if (igniter.should_show(state)) {
			if (!has_igniter) {
				pane.add("Ignitor", igniter);
				has_igniter = true;
			}
		} else {
			if (has_igniter) {
				pane.remove(igniter);
				has_igniter = false;
			}
		}

		if (state.companion != null) {
			if (!has_companion) {
				pane.add("Companion", companion);
				has_companion= true;
			}
		} else {
			if (has_companion) {
				pane.remove(companion);
				has_companion = false;
			}
		}

		if (state.gps != null) {
			if (!has_map) {
				pane.add("Site Map", sitemap);
				has_map = true;
			}
		} else {
			if (has_map) {
				pane.remove(sitemap);
				has_map = false;
			}
		}

		for (AltosFlightDisplay d : displays) {
			try {
				d.show(state, listener_state);
			} catch (Exception e) {
				System.out.printf("Exception showing %s\n", d.getName());
				e.printStackTrace();
			}
		}
	}

	public void set_exit_on_close() {
		exit_on_close = true;
	}

	Container		bag;
	AltosUIFreqList		frequencies;
	AltosUIRateList		rates;
	AltosUITelemetryList	telemetries;
	JLabel			telemetry;

	ActionListener	show_timer;

	public AltosFlightUI(AltosVoice in_voice, AltosFlightReader in_reader, final int serial) {
		AltosUIPreferences.set_component(this);

		displays = new LinkedList<AltosFlightDisplay>();

		voice = in_voice;
		reader = in_reader;

		bag = getScrollablePane();
		bag.setLayout(new GridBagLayout());

		setTitle(String.format("AltOS %s", reader.name));

		/* Stick channel selector at top of table for telemetry monitoring */
		if (serial >= 0) {
			set_inset(3);

			// Frequency menu
			frequencies = new AltosUIFreqList(AltosUIPreferences.frequency(serial));
			frequencies.set_product("Monitor");
			frequencies.set_serial(serial);
			frequencies.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						double frequency = frequencies.frequency();
						try {
							reader.set_frequency(frequency);
						} catch (TimeoutException te) {
						} catch (InterruptedException ie) {
						}
						reader.save_frequency();
					}
			});
			bag.add (frequencies, constraints(0, 1));

			// Telemetry rate list
			rates = new AltosUIRateList(AltosUIPreferences.telemetry_rate(serial));
			rates.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						int rate = rates.rate();
						try {
							reader.set_telemetry_rate(rate);
						} catch (TimeoutException te) {
						} catch (InterruptedException ie) {
						}
						reader.save_telemetry_rate();
					}
				});
			rates.setEnabled(reader.supports_telemetry_rate(AltosLib.ao_telemetry_rate_2400));
			bag.add (rates, constraints(1, 1));

			// Telemetry format list
			if (reader.supports_telemetry(Altos.ao_telemetry_standard)) {
				telemetries = new AltosUITelemetryList(serial);
				telemetries.addActionListener(new ActionListener() {
						public void actionPerformed(ActionEvent e) {
							int telemetry = telemetries.get_selected();
							reader.set_telemetry(telemetry);
							reader.save_telemetry();
						}
					});
				bag.add (telemetries, constraints(2, 1));
			} else {
				String	version;

				if (reader.supports_telemetry(Altos.ao_telemetry_0_9))
					version = "Telemetry: 0.9";
				else if (reader.supports_telemetry(Altos.ao_telemetry_0_8))
					version = "Telemetry: 0.8";
				else
					version = "Telemetry: None";

				telemetry = new JLabel(version);
				bag.add (telemetry, constraints(2, 1));
			}
			next_row();
		}
		set_inset(0);

		/* Flight status is always visible */
		flightStatus = new AltosFlightStatus();
		displays.add(flightStatus);
		bag.add(flightStatus, constraints(0, 4, GridBagConstraints.HORIZONTAL));
		next_row();

		/* The rest of the window uses a tabbed pane to
		 * show one of the alternate data views
		 */
		pane = new JTabbedPane();

		pad = new AltosPad();
		displays.add(pad);
		pane.add("Status", pad);

		igniter = new AltosIgnitor();
		displays.add(igniter);
		ascent = new AltosAscent();
		displays.add(ascent);
		descent = new AltosDescent();
		displays.add(descent);
		landed = new AltosLanded(reader);
		displays.add(landed);

		flightInfo = new AltosInfoTable();
		displays.add(flightInfo);
		pane.add("Table", new JScrollPane(flightInfo));

		companion = new AltosCompanionInfo();
		displays.add(companion);
		has_companion = false;
		has_state = false;

		sitemap = new AltosUIMap();
		displays.add(sitemap);
		has_map = false;

		/* Make the tabbed pane use the rest of the window space */
		bag.add(pane, constraints(0, 4, GridBagConstraints.BOTH));

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		AltosUIPreferences.register_font_listener(this);
		AltosPreferences.register_units_listener(this);

		status_update = new AltosFlightStatusUpdate(flightStatus);

		flightStatus.start(status_update);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					flightStatus.stop();
					disconnect();
					setVisible(false);
					dispose();
					AltosUIPreferences.unregister_font_listener(AltosFlightUI.this);
					AltosPreferences.unregister_units_listener(AltosFlightUI.this);
					if (exit_on_close)
						System.exit(0);
				}
			});

		pack();
		setVisible(true);

		thread = new AltosDisplayThread(this, voice, this, reader);

		thread.start();
	}

	public AltosFlightUI (AltosVoice in_voice, AltosFlightReader in_reader) {
		this(in_voice, in_reader, -1);
	}
}
