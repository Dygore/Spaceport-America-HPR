/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.micropeak;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import java.util.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class MicroPeak extends MicroFrame implements ActionListener, ItemListener, AltosFilterListener {

	File		filename;
	AltosGraph	graph;
	AltosUIEnable	enable;
	AltosFlightStatsTable	statsTable;
	MicroRaw	raw;
	MicroData	data;
	Container	container;
	JTabbedPane	pane;
	JMenuBar	menu_bar;
	static int	number_of_windows;

	/* File menu */
	final static String	open_command = "open";
	final static String	save_command = "save";
	final static String	export_command = "export";
	final static String	preferences_command = "preferences";
	final static String	close_command = "close";
	final static String	exit_command = "exit";

	static final String[][] file_menu_entries = new String[][] {
		{ "Open",		open_command },
		{ "Save a Copy",	save_command },
		{ "Export Data",	export_command },
		{ "Preferences",	preferences_command },
		{ "Close",		close_command },
		{ "Exit",		exit_command },
	};

	/* Download menu */
	final static String	download_command = "download";
	final static String	download_label = "Download";

	static final String[][] download_menu_entries = new String[][] {
		{ download_label,	download_command }
	};

	MicroPeak SetData(MicroData data) {
		MicroPeak	mp = this;
		if (this.data != null) {
			mp = new MicroPeak();
			return mp.SetData(data);
		}
		this.data = data;
		if (data.flight_series == null)
			System.out.printf("no data in flight\n");
		if (data.flight_stats == null)
			System.out.printf("no stats in flight\n");
		graph.set_data(data.flight_stats, data.flight_series);
		statsTable.set_stats(data.flight_stats);
		raw.setData(data);
		setTitle(data.name);
		return this;
	}

	void SetName(String name) {
		graph.setName(name);
		setTitle(name);
	}

	private static MicroData ReadFile(File filename) throws IOException, FileNotFoundException {
		MicroData	data = null;
		FileInputStream	fis = new FileInputStream(filename);
		try {
			data = new MicroData((InputStream) fis, filename.getName());
			AltosUIPreferences.set_last_logdir(filename);
		} catch (MicroData.NonHexcharException nhe) {
			data = null;
		} catch (MicroData.FileEndedException nhe) {
			data = null;
		} catch (InterruptedException ie) {
			data = null;
		} finally {
			fis.close();
		}
		return data;
	}

	private void OpenFile(File filename) {
		try {
			SetData(ReadFile(filename));
		} catch (FileNotFoundException fne) {
			JOptionPane.showMessageDialog(this,
						      fne.getMessage(),
						      "Cannot open file",
						      JOptionPane.ERROR_MESSAGE);
		} catch (IOException ioe) {
			JOptionPane.showMessageDialog(this,
						      ioe.getMessage(),
						      "File Read Error",
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	private void SelectFile() {
		MicroFileChooser	chooser = new MicroFileChooser(this);
		File			file = chooser.runDialog();

		if (file != null)
			OpenFile(file);
	}

	private void Preferences() {
		new AltosUIConfigure(this);
	}

	private void DownloadData() {
		AltosDevice	device = MicroDeviceDialog.show(this);
		MicroSerial	serial = null;
		try {
			serial = new MicroSerial(device);
		} catch (FileNotFoundException fe) {
			JOptionPane.showMessageDialog(this,
						      fe.getMessage(),
						      "Cannot open device",
						      JOptionPane.ERROR_MESSAGE);
			return;
		}

		new MicroDownload(this, device, serial);
	}

	private void no_data() {
			JOptionPane.showMessageDialog(this,
						      "No data available",
						      "No data",
						      JOptionPane.INFORMATION_MESSAGE);
	}

	private void Save() {
		if (data == null) {
			no_data();
			return;
		}
		MicroSave	save = new MicroSave (this, data);
		if (save.runDialog())
			SetName(data.name);
	}

	private void Export() {
		if (data == null) {
			no_data();
			return;
		}
		MicroExport	export = new MicroExport (this, data);
		export.runDialog();
	}

	private static void CommandGraph(File file) {
		MicroPeak m = new MicroPeak();
		m.OpenFile(file);
	}

	private static void CommandExport(File file) {
		try {
			MicroData d = ReadFile(file);
			if (d != null) {
				File	csv = new File(AltosLib.replace_extension(file.getPath(), ".csv"));
				try {
					System.out.printf ("Export \"%s\" to \"%s\"\n", file.getPath(), csv.getPath());
					MicroExport.export(csv, d);
				} catch (FileNotFoundException fe) {
					System.err.printf("Cannot create file \"%s\" (%s)\n", csv.getName(), fe.getMessage());
				} catch (IOException ie) {
					System.err.printf("Cannot write file \"%s\" (%s)\n", csv.getName(), ie.getMessage());
				}
			}
		} catch (IOException ie) {
			System.err.printf("Cannot read file \"%s\" (%s)\n", file.getName(), ie.getMessage());
		}
	}

	private void Close() {
		setVisible(false);
		dispose();
		--number_of_windows;
		if (number_of_windows == 0)
			System.exit(0);
	}

	public void actionPerformed(ActionEvent ev) {
		if (open_command.equals(ev.getActionCommand()))
			SelectFile();
		else if (save_command.equals(ev.getActionCommand()))
			Save();
		else if (export_command.equals(ev.getActionCommand()))
			Export();
		else if (preferences_command.equals(ev.getActionCommand()))
			Preferences();
		else if (close_command.equals(ev.getActionCommand()))
			Close();
		else if (exit_command.equals(ev.getActionCommand()))
			System.exit(0);
		else if (download_command.equals(ev.getActionCommand()))
			DownloadData();
	}

	public void itemStateChanged(ItemEvent e) {
	}

	/* OSXAdapter interfaces */
	public void macosx_file_handler(String path) {
		CommandGraph(new File(path));
	}

	public void macosx_quit_handler() {
		System.exit(0);
	}

	public void macosx_preferences_handler() {
		Preferences();
	}

	public void filter_changed(double speed_filter, double accel_filter) {
		data.flight_series.set_filter(speed_filter, accel_filter);
		graph.filter_changed();
		data.flight_stats = new AltosFlightStats(data.flight_series);
		statsTable.filter_changed(data.flight_stats);
	}

	public double speed_filter() {
		if (data != null && data.flight_series != null)
			return data.flight_series.speed_filter_width;
		return 4.0;
	}

	public double accel_filter() {
		if (data != null && data.flight_series != null)
			return data.flight_series.accel_filter_width;
		return 1.0;
	}

	private void add_menu(JMenu menu, String label, String action) {
		JMenuItem	item = new JMenuItem(label);
		menu.add(item);
		item.addActionListener(this);
		item.setActionCommand(action);
	}


	private void make_menu(String label, String[][] items) {
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
	}

	public MicroPeak() {

		++number_of_windows;

		register_for_macosx_events();

		AltosUIPreferences.set_component(this);

		container = getContentPane();
		pane = new JTabbedPane();

		setTitle("MicroPeak");

		menu_bar = new JMenuBar();
		setJMenuBar(menu_bar);

		make_menu("File", file_menu_entries);

		if (MAC_OS_X) {
			make_menu(download_label, download_menu_entries);
		} else {
			JButton download_button = new JButton (download_label);
			download_button.setActionCommand(download_command);
			download_button.addActionListener(this);
			menu_bar.add(download_button);
		}

		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				statsTable.tell_closing();
				raw.tell_closing();
				Close();
			}
		});

		enable = new AltosUIEnable(this);

		graph = new AltosGraph(enable);
		statsTable = new AltosFlightStatsTable();
		raw = new MicroRaw();
		pane.add(graph.panel, "Graph");
		pane.add(enable, "Configure Graph");
		pane.add(statsTable, "Statistics");
		JScrollPane scroll = new JScrollPane(raw);
		pane.add(scroll, "Raw Data");
		pane.doLayout();
		pane.validate();
		container.add(pane);
		container.doLayout();
		container.validate();
		doLayout();
		validate();
		Insets i = getInsets();
		Dimension ps = pane.getPreferredSize();
		ps.width += i.left + i.right;
		ps.height += i.top + i.bottom;
		setSize(ps);
		pack();
		setVisible(true);
	}

	public static void help(int code) {
		System.out.printf("Usage: micropeak [OPTION] ... [FILE]...\n");
		System.out.printf("  Options:\n");
		System.out.printf("    --csv\tgenerate comma separated output for spreadsheets, etc\n");
		System.out.printf("    --graph\tgraph a flight\n");
		System.exit(code);
	}

	public static void main(final String[] args) {
		boolean	opened = false;
		boolean graphing = true;

		try {
			UIManager.setLookAndFeel(AltosUIPreferences.look_and_feel());
		} catch (Exception e) {
		}

		for (int i = 0; i < args.length; i++) {
			if (args[i].equals("--help"))
				help(0);
			else if (args[i].equals("--export"))
				graphing = false;
			else if (args[i].equals("--graph"))
				graphing = true;
			else if (args[i].startsWith("--"))
				help(1);
			else {
				File	file = new File(args[i]);
				try {
					if (graphing)
						CommandGraph(file);
					else
						CommandExport(file);
					opened = true;
				} catch (Exception e) {
					System.err.printf("Error processing \"%s\": %s %s\n",
							  file.getName(), e.toString(), e.getMessage());
					e.printStackTrace();
				}
			}
		}
		if (!opened)
			new MicroPeak();
	}
}
