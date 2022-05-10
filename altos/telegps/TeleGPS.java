/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import java.text.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class TeleGPS
	extends AltosUIFrame
	implements AltosFlightDisplay, AltosFontListener, AltosUnitsListener, ActionListener, AltosEepromGrapher
{

	static String[] telegps_icon_names = {
		"/altusmetrum-telegps-16.png",
		"/altusmetrum-telegps-32.png",
		"/altusmetrum-telegps-48.png",
		"/altusmetrum-telegps-64.png",
		"/altusmetrum-telegps-128.png",
		"/altusmetrum-telegps-256.png"
	};

	static { set_icon_names(telegps_icon_names); }

	static AltosVoice	voice;

	static AltosVoice voice() {
		if (voice == null)
			voice = new AltosVoice();
		return voice;
	}

	AltosFlightReader	reader;
	TeleGPSDisplayThread	thread;
	boolean			idle_mode;

	JMenuBar		menu_bar;

	JMenu			file_menu;
	JMenu			monitor_menu;
	JMenu			device_menu;
	AltosUIFreqList		frequencies;
	ActionListener		frequency_listener;
	AltosUIRateList		rates;
	ActionListener		rate_listener;

	Container		bag;

	TeleGPSStatus		telegps_status;
	TeleGPSStatusUpdate	status_update;

	JTabbedPane		pane;

	AltosUIMap  		map;
	TeleGPSInfo		gps_info;
	TeleGPSState		gps_state;
	AltosInfoTable		info_table;

	LinkedList<AltosFlightDisplay>	displays;

	/* File menu */
	final static String	new_command = "new";
	final static String	graph_command = "graph";
	final static String	export_command = "export";
	final static String	load_maps_command = "loadmaps";
	final static String	preferences_command = "preferences";
	final static String	close_command = "close";
	final static String	exit_command = "exit";

	static final String[][] file_menu_entries = new String[][] {
		{ "New Window",		new_command },
		{ "Graph Data",		graph_command },
		{ "Export Data",	export_command },
		{ "Load Maps",		load_maps_command },
		{ "Preferences",	preferences_command },
		{ "Close",		close_command },
		{ "Exit",		exit_command },
	};

	/* Monitor menu */
	final static String	connect_command = "connect";
	final static String	disconnect_command = "disconnect";
	final static String	scan_command = "scan";

	static final String[][] monitor_menu_entries = new String[][] {
		{ "Connect Device",	connect_command },
		{ "Disconnect",		disconnect_command },
		{ "Scan Channels",	scan_command },
	};

	/* Device menu */
	final static String	download_command = "download";
	final static String	configure_command = "configure";
	final static String	flash_command = "flash";

	static final String[][] device_menu_entries = new String[][] {
		{ "Download Data",	download_command },
		{ "Configure Device",	configure_command },
		{ "Flash Device",	flash_command },
	};

	void stop_display() {
		if (thread != null && thread.isAlive()) {
			thread.interrupt();
			try {
				thread.join();
			} catch (InterruptedException ie) {}
		}
		thread = null;
	}

	public void reset() {
		for (AltosFlightDisplay display : displays)
			display.reset();
	}

	public void font_size_changed(int font_size) {
		for (AltosFlightDisplay display : displays)
			display.font_size_changed(font_size);
	}

	public void units_changed(boolean imperial_units) {
		for (AltosFlightDisplay display : displays)
			display.units_changed(imperial_units);
	}

	public void show(AltosState state, AltosListenerState listener_state) {
		try {
			status_update.saved_state = state;
			status_update.saved_listener_state = listener_state;

			if (state == null)
				state = new AltosState(new AltosCalData());

			int i = 0;
			for (AltosFlightDisplay display : displays) {
				display.show(state, listener_state);
				i++;
			}
		} catch (Exception ex) {
			System.out.printf("Exception %s\n", ex.toString());
			for (StackTraceElement e : ex.getStackTrace())
				System.out.printf("%s\n", e.toString());
		}
	}

	void new_window() {
		new TeleGPS();
	}

	void preferences() {
		new TeleGPSPreferences(this, voice());
	}

	void load_maps() {
		new AltosUIMapPreload(this);
	}

	void disconnect() {
		setTitle("TeleGPS");
		stop_display();
		telegps_status.stop();

		telegps_status.disable_receive();
		disable_frequency_menu();
		disable_rate_menu();
	}

	void connect_flight(AltosDevice device) {
		try {
			AltosFlightReader	reader = new AltosTelemetryReader(new AltosSerial(device));
			set_reader(reader, device, false);
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(this,
						      ee.getMessage(),
						      String.format ("Cannot open %s", device.toShortString()),
						      JOptionPane.ERROR_MESSAGE);
		} catch (AltosSerialInUseException si) {
			JOptionPane.showMessageDialog(this,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Unknown I/O error on %s", device.toShortString()),
						      "Unknown I/O error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Timeout on %s", device.toShortString()),
						      "Timeout error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			JOptionPane.showMessageDialog(this,
						      String.format("Interrupted %s", device.toShortString()),
						      "Interrupted exception",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	void connect_idle(AltosDevice device) {
		try {
       			AltosFlightReader	reader = new AltosIdleReader(new AltosSerial(device), false);
			set_reader(reader, device, true);
		} catch (FileNotFoundException ee) {
			JOptionPane.showMessageDialog(this,
						      ee.getMessage(),
						      String.format ("Cannot open %s", device.toShortString()),
						      JOptionPane.ERROR_MESSAGE);
		} catch (AltosSerialInUseException si) {
			JOptionPane.showMessageDialog(this,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ee) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Unknown I/O error on %s", device.toShortString()),
						      "Unknown I/O error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			JOptionPane.showMessageDialog(this,
						      String.format ("Timeout on %s", device.toShortString()),
						      "Timeout error",
						      JOptionPane.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			JOptionPane.showMessageDialog(this,
						      String.format("Interrupted %s", device.toShortString()),
						      "Interrupted exception",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	void connect(AltosDevice device) {
		if (reader != null)
			disconnect();
		if (device.matchProduct(AltosLib.product_basestation))
			connect_flight(device);
		else
			connect_idle(device);
	}

	void connect() {
		AltosDevice	device = AltosDeviceUIDialog.show(this,
								  AltosLib.product_any);
		if (device == null)
			return;
		connect(device);
	}

	public void scan_device_selected(AltosDevice device) {
		connect(device);
	}

	void scan() {
		new AltosScanUI(this, false);
	}

	void download(){
		new AltosEepromManage(this, this, AltosLib.product_telegps);
	}

	void configure() {
		new TeleGPSConfig(this);
	}

	private static AltosFlightSeries make_series(AltosRecordSet set) {
		AltosFlightSeries series = new AltosFlightSeries(set.cal_data());
		set.capture_series(series);
		series.finish();
		return series;
	}

	void export() {
		AltosDataChooser chooser = new AltosDataChooser(this);

		AltosRecordSet set = chooser.runDialog();
		if (set == null)
			return;
		AltosFlightSeries series = make_series(set);
		new AltosCSVUI(this, series, chooser.file());
	}

	void graph() {
		AltosDataChooser chooser = new AltosDataChooser(this);
		AltosRecordSet set = chooser.runDialog();
		graph_file(this, set, chooser.file());
	}

	public void graph_flights(AltosEepromList list) {
		for (AltosEepromLog log : list) {
			if (log.file != null) {
				AltosRecordSet set = record_set(log.file);
				graph_file(this, set, log.file);
			}
		}
	}

	void flash() {
		AltosFlashUI.show(this);
	}

	public void actionPerformed(ActionEvent ev) {

		/* File menu */
		if (new_command.equals(ev.getActionCommand())) {
			new_window();
			return;
		}
		if (preferences_command.equals(ev.getActionCommand())) {
			preferences();
			return;
		}
		if (load_maps_command.equals(ev.getActionCommand())) {
			load_maps();
			return;
		}
		if (close_command.equals(ev.getActionCommand())) {
			close();
			return;
		}
		if (exit_command.equals(ev.getActionCommand()))
			System.exit(0);

		/* Monitor menu */
		if (connect_command.equals(ev.getActionCommand())) {
			connect();
			return;
		}
		if (disconnect_command.equals(ev.getActionCommand())) {
			disconnect();
			return;
		}
		if (scan_command.equals(ev.getActionCommand())) {
			scan();
			return;
		}

		/* Device menu */
		if (download_command.equals(ev.getActionCommand())) {
			download();
			return;
		}
		if (configure_command.equals(ev.getActionCommand())) {
			configure();
			return;
		}
		if (export_command.equals(ev.getActionCommand())) {
			export();
			return;
		}
		if (graph_command.equals(ev.getActionCommand())) {
			graph();
			return;
		}
		if (flash_command.equals(ev.getActionCommand())) {
			flash();
			return;
		}
	}

	void enable_frequency_menu(int serial, final AltosFlightReader reader) {

		if (frequency_listener != null)
			disable_frequency_menu();

		frequency_listener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					double frequency = frequencies.frequency();
					try {
						reader.set_frequency(frequency);
					} catch (TimeoutException te) {
					} catch (InterruptedException ie) {
					}
					reader.save_frequency();
				}
			};

		frequencies.addActionListener(frequency_listener);
		frequencies.set_product("Monitor");
		frequencies.set_serial(serial);
		frequencies.set_frequency(AltosUIPreferences.frequency(serial));
		frequencies.setEnabled(true);

	}

	void disable_frequency_menu() {
		if (frequency_listener != null) {
			frequencies.removeActionListener(frequency_listener);
			frequencies.setEnabled(false);
			frequency_listener = null;
		}

	}

	void enable_rate_menu(int serial, final AltosFlightReader reader) {

		if (rate_listener != null)
			disable_rate_menu();

		rate_listener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					int rate = rates.rate();
					try {
						reader.set_telemetry_rate(rate);
					} catch (TimeoutException te) {
					} catch (InterruptedException ie) {
					}
					reader.save_telemetry_rate();
				}
			};

		rates.addActionListener(rate_listener);
		rates.set_product("Monitor");
		rates.set_serial(serial);
		rates.set_rate(AltosUIPreferences.telemetry_rate(serial));
		rates.setEnabled(reader.supports_telemetry_rate(AltosLib.ao_telemetry_rate_2400));
	}

	void disable_rate_menu() {
		if (rate_listener != null) {
			rates.removeActionListener(rate_listener);
			rates.setEnabled(false);
			rate_listener = null;
		}

	}

	public void set_reader(AltosFlightReader reader, AltosDevice device, boolean idle_mode) {
		this.idle_mode = idle_mode;
		status_update = new TeleGPSStatusUpdate(telegps_status);

		telegps_status.start(status_update);

		setTitle(String.format("TeleGPS %s", reader.name));
		thread = new TeleGPSDisplayThread(this, voice(), this, reader);
		thread.start();

		if (device != null) {
			if (idle_mode) {
				disable_frequency_menu();
				disable_rate_menu();
			} else {
				enable_frequency_menu(device.getSerial(), reader);
				enable_rate_menu(device.getSerial(), reader);
			}
		}
	}

	static int	number_of_windows;

	static public void add_window() {
		++number_of_windows;
	}

	static public void subtract_window() {
		--number_of_windows;
		if (number_of_windows == 0)
			System.exit(0);
	}

	private void close() {
		disconnect();
		AltosUIPreferences.unregister_font_listener(this);
		AltosPreferences.unregister_units_listener(this);
		setVisible(false);
		dispose();
		subtract_window();
	}

	private void add_menu(JMenu menu, String label, String action) {
		JMenuItem	item = new JMenuItem(label);
		menu.add(item);
		item.addActionListener(this);
		item.setActionCommand(action);
	}


	private JMenu make_menu(String label, String[][] items) {
		JMenu	menu = new JMenu(label);
		for (int i = 0; i < items.length; i++) {
			if (MAC_OS_X) {
				if (items[i][1].equals("exit"))
					continue;
				if (items[i][1].equals("preferences"))
					continue;
			}
			add_menu(menu, items[i][0], items[i][1]);
		}
		menu_bar.add(menu);
		return menu;
	}

	/* OSXAdapter interfaces */
	public void macosx_file_handler(String path) {
		process_graph(new File(path));
	}

	public void macosx_quit_handler() {
		System.exit(0);
	}

	public void macosx_preferences_handler() {
		preferences();
	}

	public TeleGPS() {

		AltosUIPreferences.set_component(this);

		register_for_macosx_events();

		reader = null;

		bag = getContentPane();
		bag.setLayout(new GridBagLayout());

		setTitle("TeleGPS");

		menu_bar = new JMenuBar();
		setJMenuBar(menu_bar);

		file_menu = make_menu("File", file_menu_entries);
		monitor_menu = make_menu("Monitor", monitor_menu_entries);
		device_menu = make_menu("Device", device_menu_entries);

		set_inset(3);
		frequencies = new AltosUIFreqList();
		frequencies.setEnabled(false);
		bag.add(frequencies, constraints (0, 1));

		rates = new AltosUIRateList();
		rates.setEnabled(false);
		bag.add(rates, constraints(1, 1));
		next_row();
		set_inset(0);

		displays = new LinkedList<AltosFlightDisplay>();

		int serial = -1;

		/* TeleGPS status is always visible */
		telegps_status = new TeleGPSStatus();
		bag.add(telegps_status, constraints(0, 3, GridBagConstraints.HORIZONTAL));
		next_row();

		displays.add(telegps_status);


		/* The rest of the window uses a tabbed pane to
		 * show one of the alternate data views
		 */
		pane = new JTabbedPane();

		/* Make the tabbed pane use the rest of the window space */
		bag.add(pane, constraints(0, 3, GridBagConstraints.BOTH));

		map = new AltosUIMap();
		pane.add(map.getName(), map);
		displays.add(map);

		gps_info = new TeleGPSInfo();
		pane.add(gps_info.getName(), gps_info);
		displays.add(gps_info);

		gps_state = new TeleGPSState();
		pane.add(gps_state.getName(), gps_state);
		displays.add(gps_state);

		info_table = new AltosInfoTable();
		pane.add("Table", info_table);
		displays.add(info_table);

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

		AltosUIPreferences.register_font_listener(this);
		AltosPreferences.register_units_listener(this);

		addWindowListener(new WindowAdapter() {
				@Override
				public void windowClosing(WindowEvent e) {
					close();
				}
			});

		pack();
		setVisible(true);

		add_window();
	}

	public TeleGPS(AltosFlightReader reader, boolean idle_mode) {
		this();
		set_reader(reader, null, idle_mode);
	}

	public TeleGPS(AltosDevice device) {
		this();
		connect(device);
	}

	static AltosRecordSet record_set(File file) {
		try {
			return AltosLib.record_set(file);
		} catch (IOException ie) {
			System.out.printf("%s\n", ie.getMessage());
                }
                return null;
	}

	static AltosReplayReader replay_file(File file) {
		AltosRecordSet set = record_set(file);
		if (set == null)
			return null;
		return new AltosReplayReader(set, file);
	}

	private static boolean graph_file(TeleGPS telegps, AltosRecordSet set, File file) {
		if (set == null)
			return false;
		if (!set.valid()) {
			JOptionPane.showMessageDialog(telegps,
						      String.format("Failed to parse file %s", file),
						      "Graph Failed",
						      JOptionPane.ERROR_MESSAGE);
			return false;
		}
		try {
			new TeleGPSGraphUI(set, file);
		} catch (IOException e) {
			System.out.printf("Exception %s\n", e.toString());
		} catch (InterruptedException e) {
			System.out.printf("Exception %s\n", e.toString());
			return false;
		}
		return true;
	}

	static boolean process_graph(File file) {
		AltosRecordSet set = record_set(file);
		return graph_file(null, set, file);
	}

	static boolean process_replay(File file) {
		AltosReplayReader new_reader = replay_file(file);
		if (new_reader == null)
			return false;

		new TeleGPS(new_reader, true);
		return true;
	}

	static final int process_none = 0;
	static final int process_csv = 1;
	static final int process_kml = 2;
	static final int process_graph = 3;
	static final int process_replay = 4;
	static final int process_summary = 5;
	static final int process_cat = 6;

	public static boolean load_library(Frame frame) {
		if (!AltosUILib.load_library()) {
			JOptionPane.showMessageDialog(frame,
						      String.format("No AltOS library in \"%s\"",
								    System.getProperty("java.library.path","<undefined>")),
						      "Cannot load device access library",
						      JOptionPane.ERROR_MESSAGE);
			return false;
		}
		return true;
	}

	public static void help(int code) {
		System.out.printf("Usage: altosui [OPTION]... [FILE]...\n");
		System.out.printf("  Options:\n");
		System.out.printf("    --replay <filename>\t\trelive the glory of past flights \n");
		System.out.printf("    --graph <filename>\t\tgraph a flight\n");
		System.out.printf("    --csv\tgenerate comma separated output for spreadsheets, etc\n");
		System.out.printf("    --kml\tgenerate KML output for use with Google Earth\n");
		System.exit(code);
	}

	public static void main(String[] args) {
		int	errors = 0;

		load_library(null);
		try {
			UIManager.setLookAndFeel(AltosUIPreferences.look_and_feel());
		} catch (Exception e) {
		}

		boolean	any_created = false;


		/* Handle batch-mode */
		int process = process_none;
		for (int i = 0; i < args.length; i++) {
			if (args[i].equals("--help"))
				help(0);
			else if (args[i].equals("--replay"))
				process = process_replay;
			else if (args[i].equals("--kml"))
				process = process_kml;
			else if (args[i].equals("--csv"))
				process = process_csv;
			else if (args[i].equals("--graph"))
				process = process_graph;
			else if (args[i].equals("--summary"))
				process = process_summary;
			else if (args[i].equals("--cat"))
				process = process_cat;
			else if (args[i].startsWith("--"))
				help(1);
			else {
				File file = new File(args[i]);
				switch (process) {
				case process_none:
				case process_graph:
					if (!process_graph(file))
						++errors;
					break;
				case process_replay:
					if (!process_replay(file))
						++errors;
					any_created = true;
					break;
				case process_kml:
					++errors;
					break;
				case process_csv:
					++errors;
					break;
				case process_summary:
					++errors;
					break;
				case process_cat:
					++errors;
				}
			}
		}
		if (errors != 0)
			System.exit(errors);
		if (number_of_windows == 0) {
			java.util.List<AltosDevice> devices = AltosUSBDevice.list(AltosLib.product_basestation);
			if (devices != null)
				for (AltosDevice device : devices) {
					new TeleGPS(device);
					any_created = true;
				}
			if (number_of_windows == 0)
				new TeleGPS();
		}
	}
}
