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

import java.text.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class TeleGPSConfigUI
	extends AltosUIDialog
	implements ActionListener, ItemListener, DocumentListener, AltosConfigValues, AltosUnitsListener
{

	Container		pane;
	JLabel			product_label;
	JLabel			version_label;
	JLabel			serial_label;
	JLabel			frequency_label;
	JLabel			radio_calibration_label;
	JLabel			radio_frequency_label;
	JLabel			radio_enable_label;
	JLabel			radio_10mw_label;
	JLabel			rate_label;
	JLabel			aprs_interval_label;
	JLabel			aprs_ssid_label;
	JLabel			aprs_format_label;
	JLabel			aprs_offset_label;
	JLabel			flight_log_max_label;
	JLabel			callsign_label;
	JLabel			tracker_motion_label;
	JLabel			tracker_interval_label;

	public boolean		dirty;

	JFrame			owner;
	JLabel			product_value;
	JLabel			version_value;
	JLabel			serial_value;
	AltosUIFreqList		radio_frequency_value;
	JLabel			radio_calibration_value;
	JRadioButton		radio_enable_value;
	JRadioButton		radio_10mw_value;
	AltosUIRateList		rate_value;
	JComboBox<String>	aprs_interval_value;
	JComboBox<Integer>	aprs_ssid_value;
	JComboBox<String>	aprs_format_value;
	JComboBox<Integer>	aprs_offset_value;
	JComboBox<String>	flight_log_max_value;
	JTextField		callsign_value;
	JComboBox<String>	tracker_motion_value;
	JComboBox<String>	tracker_interval_value;

	JButton			save;
	JButton			reset;
	JButton			reboot;
	JButton			close;

	ActionListener		listener;

	static String[] 	aprs_interval_values = {
		"Disabled",
		"2",
		"5",
		"10"
	};

	static Integer[]	aprs_ssid_values = {
		0, 1, 2 ,3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};

	static Integer[]	aprs_offset_values = {
		0, 2, 4, 6, 8, 10, 12, 14, 16, 18
	};

	static String[]		tracker_motion_values_m = {
		"2",
		"5",
		"10",
		"25",
	};

	static String[]		tracker_motion_values_ft = {
		"5",
		"20",
		"50",
		"100"
	};

	static String[]		tracker_interval_values = {
		"1",
		"2",
		"5",
		"10"
	};

	/* A window listener to catch closing events and tell the config code */
	class ConfigListener extends WindowAdapter {
		TeleGPSConfigUI	ui;

		public ConfigListener(TeleGPSConfigUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "Close"));
		}
	}

	public void set_pyros(AltosPyro[] new_pyros) {
	}

	public AltosPyro[] pyros() {
		return null;
	}

	public void set_pyro_firing_time(double new_pyro_firing_time) {
	}

	public double pyro_firing_time() {
		return AltosLib.MISSING;
	}

	boolean is_telemetrum() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleGPS");
	}

	void set_radio_enable_tool_tip() {
		if (radio_enable_value.isVisible())
			radio_enable_value.setToolTipText("Enable/Disable telemetry and RDF transmissions");
		else
			radio_enable_value.setToolTipText("Firmware version does not support disabling radio");
	}

	void set_radio_10mw_tool_tip() {
		if (radio_10mw_value.isVisible())
			radio_10mw_value.setToolTipText("Should transmitter power be limited to 10mW");
		else
			radio_10mw_value.setToolTipText("Older firmware could not limit radio power");
	}

	void set_rate_tool_tip() {
		if (rate_value.isVisible())
			rate_value.setToolTipText("Select telemetry baud rate");
		else
			rate_value.setToolTipText("Firmware version does not support variable telemetry rates");
	}

	void set_aprs_interval_tool_tip() {
		if (aprs_interval_value.isVisible())
			aprs_interval_value.setToolTipText("Enable APRS and set the interval between APRS reports");
		else
			aprs_interval_value.setToolTipText("Hardware doesn't support APRS");
	}

	void set_aprs_ssid_tool_tip() {
		if (aprs_ssid_value.isVisible())
			aprs_ssid_value.setToolTipText("Set the APRS SSID (secondary station identifier)");
		else if (aprs_ssid_value.isVisible())
			aprs_ssid_value.setToolTipText("Software version doesn't support setting the APRS SSID");
		else
			aprs_ssid_value.setToolTipText("Hardware doesn't support APRS");
	}

	void set_aprs_format_tool_tip() {
		if (aprs_format_value.isVisible())
			aprs_format_value.setToolTipText("Set the APRS format (compressed/uncompressed)");
		else if (aprs_format_value.isVisible())
			aprs_format_value.setToolTipText("Software version doesn't support setting the APRS format");
		else
			aprs_format_value.setToolTipText("Hardware doesn't support APRS");
	}

	void set_aprs_offset_tool_tip() {
		if (aprs_offset_value.isVisible())
			aprs_offset_value.setToolTipText("Set the APRS offset from top of minute");
		else if (aprs_offset_value.isVisible())
			aprs_offset_value.setToolTipText("Software version doesn't support setting the APRS offset");
		else
			aprs_offset_value.setToolTipText("Hardware doesn't support APRS");
	}

	void set_flight_log_max_tool_tip() {
		if (flight_log_max_value.isVisible())
			flight_log_max_value.setToolTipText("Size reserved for each flight log (in kB)");
		else
			flight_log_max_value.setToolTipText("Cannot set max value with flight logs in memory");
	}

	/* Build the UI using a grid bag */
	public TeleGPSConfigUI(JFrame in_owner) {
		super (in_owner, "Configure Device", false);

		owner = in_owner;
		GridBagConstraints c;
		int row = 0;

		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getScrollablePane();
		pane.setLayout(new GridBagLayout());

		/* Product */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		product_label = new JLabel("Product:");
		pane.add(product_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		product_value = new JLabel("");
		pane.add(product_value, c);
		row++;

		/* Version */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		version_label = new JLabel("Software version:");
		pane.add(version_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		version_value = new JLabel("");
		pane.add(version_value, c);
		row++;

		/* Serial */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		serial_value = new JLabel("");
		pane.add(serial_value, c);
		row++;

		/* Frequency */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_frequency_label = new JLabel("Frequency:");
		pane.add(radio_frequency_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_frequency_value = new AltosUIFreqList();
		radio_frequency_value.addItemListener(this);
		pane.add(radio_frequency_value, c);
		radio_frequency_value.setToolTipText("Telemetry, RDF and packet frequency");
		row++;

		/* Radio Calibration */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_calibration_label = new JLabel("RF Calibration:");
		pane.add(radio_calibration_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_calibration_value = new JLabel(String.format("%d", 1186611));
		pane.add(radio_calibration_value, c);
		row++;

		/* Radio Enable */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_enable_label = new JLabel("Telemetry/RDF/APRS Enable:");
		pane.add(radio_enable_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_enable_value = new JRadioButton("Enabled");
		radio_enable_value.addItemListener(this);
		pane.add(radio_enable_value, c);
		set_radio_enable_tool_tip();
		row++;

		/* Radio 10mW limit */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		radio_10mw_label = new JLabel("Limit transmit to 10mW:");
		pane.add(radio_10mw_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		radio_10mw_value = new JRadioButton("Limited");
		radio_10mw_value.addItemListener(this);
		pane.add(radio_10mw_value, c);
		set_radio_10mw_tool_tip();
		row++;

		/* Telemetry Rate */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		rate_label = new JLabel("Telemetry baud rate:");
		pane.add(rate_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		rate_value = new AltosUIRateList();
		rate_value.addItemListener(this);
		pane.add(rate_value, c);
		set_rate_tool_tip();
		row++;

		/* APRS interval */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		aprs_interval_label = new JLabel("APRS Interval(s):");
		pane.add(aprs_interval_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		aprs_interval_value = new JComboBox<String>(aprs_interval_values);
		aprs_interval_value.setEditable(true);
		aprs_interval_value.addItemListener(this);
		pane.add(aprs_interval_value, c);
		set_aprs_interval_tool_tip();
		row++;

		/* APRS SSID */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		aprs_ssid_label = new JLabel("APRS SSID:");
		pane.add(aprs_ssid_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		aprs_ssid_value = new JComboBox<Integer>(aprs_ssid_values);
		aprs_ssid_value.setEditable(false);
		aprs_ssid_value.addItemListener(this);
		aprs_ssid_value.setMaximumRowCount(aprs_ssid_values.length);
		pane.add(aprs_ssid_value, c);
		set_aprs_ssid_tool_tip();
		row++;

		/* APRS format */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		aprs_format_label = new JLabel("APRS format:");
		pane.add(aprs_format_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		aprs_format_value = new JComboBox<String>(AltosLib.ao_aprs_format_name);
		aprs_format_value.setEditable(false);
		aprs_format_value.addItemListener(this);
		aprs_format_value.setMaximumRowCount(AltosLib.ao_aprs_format_name.length);
		pane.add(aprs_format_value, c);
		set_aprs_format_tool_tip();
		row++;

		/* APRS offset */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		aprs_offset_label = new JLabel("APRS offset:");
		pane.add(aprs_offset_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		aprs_offset_value = new JComboBox<Integer>(aprs_offset_values);
		aprs_offset_value.setEditable(false);
		aprs_offset_value.addItemListener(this);
		aprs_offset_value.setMaximumRowCount(aprs_offset_values.length);
		pane.add(aprs_offset_value, c);
		set_aprs_offset_tool_tip();
		row++;

		/* Callsign */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		callsign_label = new JLabel("Callsign:");
		pane.add(callsign_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		callsign_value = new JTextField(AltosUIPreferences.callsign());
		callsign_value.getDocument().addDocumentListener(this);
		pane.add(callsign_value, c);
		callsign_value.setToolTipText("Callsign reported in telemetry data");
		row++;

		/* Flight log max */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		flight_log_max_label = new JLabel("Maximum Log Size (kB):");
		pane.add(flight_log_max_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		flight_log_max_value = new JComboBox<String>();
		flight_log_max_value.setEditable(true);
		flight_log_max_value.addItemListener(this);
		pane.add(flight_log_max_value, c);
		set_flight_log_max_tool_tip();
		row++;

		/* Tracker triger horiz distances */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		tracker_motion_label = new JLabel(get_tracker_motion_label());
		pane.add(tracker_motion_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		tracker_motion_value = new JComboBox<String>(tracker_motion_values());
		tracker_motion_value.setEditable(true);
		tracker_motion_value.addItemListener(this);
		pane.add(tracker_motion_value, c);
		row++;

		/* Tracker triger vert distances */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		tracker_interval_label = new JLabel("Position Reporting Interval (s):");
		pane.add(tracker_interval_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		tracker_interval_value = new JComboBox<String>(tracker_interval_values);
		tracker_interval_value.setEditable(true);
		tracker_interval_value.addItemListener(this);
		pane.add(tracker_interval_value, c);
		set_tracker_tool_tip();
		row++;

		/* Buttons */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		save = new JButton("Save");
		pane.add(save, c);
		save.addActionListener(this);
		save.setActionCommand("Save");

		c = new GridBagConstraints();
		c.gridx = 2; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reset = new JButton("Reset");
		pane.add(reset, c);
		reset.addActionListener(this);
		reset.setActionCommand("Reset");

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = il;
		reboot = new JButton("Reboot");
		pane.add(reboot, c);
		reboot.addActionListener(this);
		reboot.setActionCommand("Reboot");

		c = new GridBagConstraints();
		c.gridx = 6; c.gridy = row;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_END;
		c.insets = il;
		close = new JButton("Close");
		pane.add(close, c);
		close.addActionListener(this);
		close.setActionCommand("Close");

		addWindowListener(new ConfigListener(this));
		AltosPreferences.register_units_listener(this);
	}

	/* Once the initial values are set, the config code will show the dialog */
	public void make_visible() {
		pack();
		setLocationRelativeTo(owner);
		setVisible(true);
	}

	/* If any values have been changed, confirm before closing */
	public boolean check_dirty(String operation) {
		if (dirty) {
			Object[] options = { String.format("%s anyway", operation), "Keep editing" };
			int i;
			i = JOptionPane.showOptionDialog(this,
							 String.format("Configuration modified. %s anyway?", operation),
							 "Configuration Modified",
							 JOptionPane.DEFAULT_OPTION,
							 JOptionPane.WARNING_MESSAGE,
							 null, options, options[1]);
			if (i != 0)
				return false;
		}
		return true;
	}

	public void set_dirty() {
		dirty = true;
		save.setEnabled(true);
	}

	public void set_clean() {
		dirty = false;
		save.setEnabled(false);
	}

	public void dispose() {
		AltosPreferences.unregister_units_listener(this);
		super.dispose();
	}

	public int accel_cal_plus() {
		return AltosLib.MISSING;
	}

	public int accel_cal_minus() {
		return AltosLib.MISSING;
	}

	public void set_accel_cal(int accel_plus, int accel_minus) {
	}

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Close") || cmd.equals("Reboot"))
			if (!check_dirty(cmd))
				return;
		listener.actionPerformed(e);
		if (cmd.equals("Close") || cmd.equals("Reboot")) {
			setVisible(false);
			dispose();
		}
		set_clean();
	}

	/* ItemListener interface method */
	public void itemStateChanged(ItemEvent e) {
		set_dirty();
	}

	/* DocumentListener interface methods */
	public void changedUpdate(DocumentEvent e) {
		set_dirty();
	}

	public void insertUpdate(DocumentEvent e) {
		set_dirty();
	}

	public void removeUpdate(DocumentEvent e) {
		set_dirty();
	}

	/* Let the config code hook on a listener */
	public void addActionListener(ActionListener l) {
		listener = l;
	}

	public void units_changed(boolean imperial_units) {
		boolean	was_dirty = dirty;

		if (tracker_motion_value.isVisible()) {
			String motion = tracker_motion_value.getSelectedItem().toString();
			tracker_motion_label.setText(get_tracker_motion_label());
			set_tracker_motion_values();
			try {
				int m = (int) (AltosConvert.height.parse_locale(motion, !imperial_units) + 0.5);
				set_tracker_motion(m);
			} catch (ParseException pe) {
			}
		}
		if (!was_dirty)
			set_clean();
	}

	/* set and get all of the dialog values */
	public void set_product(String product) {
		radio_frequency_value.set_product(product);
		product_value.setText(product);
		set_flight_log_max_tool_tip();
	}

	public void set_version(String version) {
		version_value.setText(version);
	}

	public void set_serial(int serial) {
		radio_frequency_value.set_serial(serial);
		serial_value.setText(String.format("%d", serial));
	}

	public void set_altitude_32(int altitude_32) {
	}

	public void set_main_deploy(int new_main_deploy) {
	}

	public int main_deploy() {
		return AltosLib.MISSING;
	}

	public void set_apogee_delay(int new_apogee_delay) { }

	public int apogee_delay() {
		return AltosLib.MISSING;
	}

	public void set_apogee_lockout(int new_apogee_lockout) { }

	public int apogee_lockout() { return AltosLib.MISSING; }

	public void set_radio_frequency(double new_radio_frequency) {
		if (new_radio_frequency != AltosLib.MISSING)
			radio_frequency_value.set_frequency(new_radio_frequency);
		radio_frequency_label.setVisible(new_radio_frequency != AltosLib.MISSING);
		radio_frequency_value.setVisible(new_radio_frequency != AltosLib.MISSING);
	}

	public double radio_frequency() {
		return radio_frequency_value.frequency();
	}

	public void set_radio_calibration(int new_radio_calibration) {
		if (new_radio_calibration != AltosLib.MISSING)
			radio_calibration_value.setText(String.format("%d", new_radio_calibration));
		radio_calibration_value.setVisible(new_radio_calibration == AltosLib.MISSING);
		radio_calibration_label.setVisible(new_radio_calibration == AltosLib.MISSING);
	}

	public void set_radio_enable(int new_radio_enable) {
		if (new_radio_enable != AltosLib.MISSING)
			radio_enable_value.setSelected(new_radio_enable != 0);
		radio_enable_label.setVisible(new_radio_enable != AltosLib.MISSING);
		radio_enable_value.setVisible(new_radio_enable != AltosLib.MISSING);
		set_radio_enable_tool_tip();
	}

	public int radio_enable() {
		if (radio_enable_value.isVisible())
			return radio_enable_value.isSelected() ? 1 : 0;
		else
			return AltosLib.MISSING;
	}

	public void set_radio_10mw(int new_radio_10mw) {
		if (new_radio_10mw != AltosLib.MISSING) {
			radio_10mw_value.setSelected(new_radio_10mw != 0);
		}
		radio_10mw_value.setVisible(new_radio_10mw != AltosLib.MISSING);
		radio_10mw_label.setVisible(new_radio_10mw != AltosLib.MISSING);
		set_radio_10mw_tool_tip();
	}

	public int radio_10mw() {
		if (radio_10mw_value.isVisible())
			return radio_10mw_value.isSelected() ? 1 : 0;
		else
			return AltosLib.MISSING;
	}

	public void set_telemetry_rate(int new_rate) {
		if (new_rate != AltosLib.MISSING)
			rate_value.set_rate(new_rate);
		rate_label.setVisible(new_rate != AltosLib.MISSING);
		rate_value.setVisible(new_rate != AltosLib.MISSING);
	}

	public int telemetry_rate() {
		return rate_value.rate();
	}

	public void set_callsign(String new_callsign) {
		if (new_callsign != null)
			callsign_value.setText(new_callsign);
		callsign_value.setVisible(new_callsign != null);
		callsign_label.setVisible(new_callsign != null);
	}

	public String callsign() {
		if (callsign_value.isVisible())
			return callsign_value.getText();
		return null;
	}

	private int parse_int(String name, String s, boolean split) throws AltosConfigDataException {
		String v = s;
		if (split)
			v = s.split("\\s+")[0];
		try {
			return Integer.parseInt(v);
		} catch (NumberFormatException ne) {
			throw new AltosConfigDataException("Invalid %s \"%s\"", name, s);
		}
	}

	int	flight_log_max_limit;
	int	flight_log_max;

	public String flight_log_max_label(int flight_log_max) {
		if (flight_log_max_limit != 0) {
			int	nflight = flight_log_max_limit / flight_log_max;
			String	plural = nflight > 1 ? "s" : "";

			return String.format("%d (%d flight%s)", flight_log_max, nflight, plural);
		}
		return String.format("%d", flight_log_max);
	}

	public void set_flight_log_max(int new_flight_log_max) {
		flight_log_max_value.setSelectedItem(flight_log_max_label(new_flight_log_max));
		flight_log_max = new_flight_log_max;
		set_flight_log_max_tool_tip();
	}

	public void set_flight_log_max_enabled(boolean enable) {
		flight_log_max_value.setEnabled(enable);
		set_flight_log_max_tool_tip();
	}

	public int flight_log_max() throws AltosConfigDataException {
		return parse_int("flight log max", flight_log_max_value.getSelectedItem().toString(), true);
	}

	public void set_flight_log_max_limit(int new_flight_log_max_limit, int new_storage_erase_unit) {
		flight_log_max_limit = new_flight_log_max_limit;
		if (new_flight_log_max_limit != AltosLib.MISSING) {
			flight_log_max_value.removeAllItems();
			for (int i = 8; i >= 1; i--) {
				int	size = flight_log_max_limit / i;
				if (new_storage_erase_unit != 0)
					size &= ~(new_storage_erase_unit - 1);
				flight_log_max_value.addItem(String.format("%d (%d flights)", size, i));
			}
		}
		if (flight_log_max != 0 && flight_log_max != AltosLib.MISSING)
			set_flight_log_max(flight_log_max);
	}

	public void set_ignite_mode(int new_ignite_mode) { }
	public int ignite_mode() { return AltosLib.MISSING; }


	public void set_pad_orientation(int new_pad_orientation) { }
	public int pad_orientation() { return AltosLib.MISSING; }

	public void set_beep(int new_beep) { }

	public int beep() { return AltosLib.MISSING; }

	String[] tracker_motion_values() {
		if (AltosConvert.imperial_units)
			return tracker_motion_values_ft;
		else
			return tracker_motion_values_m;
	}

	void set_tracker_motion_values() {
		String[]	v = tracker_motion_values();
		while (tracker_motion_value.getItemCount() > 0)
			tracker_motion_value.removeItemAt(0);
		for (int i = 0; i < v.length; i++)
			tracker_motion_value.addItem(v[i]);
		tracker_motion_value.setMaximumRowCount(v.length);
	}

	String get_tracker_motion_label() {
		return String.format("Logging Trigger Motion (%s):", AltosConvert.height.parse_units());
	}

	void set_tracker_tool_tip() {
		if (tracker_motion_value.isVisible())
			tracker_motion_value.setToolTipText("How far the device must move before logging");
		else
			tracker_motion_value.setToolTipText("This device doesn't disable logging when stationary");
		if (tracker_interval_value.isVisible())
			tracker_interval_value.setToolTipText("How often to report GPS position");
		else
			tracker_interval_value.setToolTipText("This device can't configure interval");
	}

	public void set_tracker_motion(int tracker_motion) {
		if (tracker_motion != AltosLib.MISSING)
			tracker_motion_value.setSelectedItem(AltosConvert.height.say(tracker_motion));
		tracker_motion_label.setVisible(tracker_motion != AltosLib.MISSING);
		tracker_motion_value.setVisible(tracker_motion != AltosLib.MISSING);
	}

	public int tracker_motion() throws AltosConfigDataException {
		if (tracker_motion_value.isVisible()) {
			String str = tracker_motion_value.getSelectedItem().toString();
			try {
				return (int) (AltosConvert.height.parse_locale(str) + 0.5);
			} catch (ParseException pe) {
				throw new AltosConfigDataException("invalid tracker motion %s", str);
			}
		}
		return AltosLib.MISSING;
	}

	public void set_tracker_interval(int tracker_interval) {
		if (tracker_interval != AltosLib.MISSING)
			tracker_interval_value.setSelectedItem(String.format("%d", tracker_interval));
		tracker_interval_label.setVisible(tracker_interval != AltosLib.MISSING);
		tracker_interval_value.setVisible(tracker_interval != AltosLib.MISSING);
	}

	public int tracker_interval() throws AltosConfigDataException {
		if (tracker_interval_value.isVisible())
			return parse_int ("tracker interval", tracker_interval_value.getSelectedItem().toString(), false);
		return AltosLib.MISSING;
	}

	public void set_aprs_interval(int new_aprs_interval) {
		if (new_aprs_interval != AltosLib.MISSING)
			aprs_interval_value.setSelectedItem(Integer.toString(new_aprs_interval));
		aprs_interval_value.setVisible(new_aprs_interval != AltosLib.MISSING);
		aprs_interval_label.setVisible(new_aprs_interval != AltosLib.MISSING);
		set_aprs_interval_tool_tip();
	}

	public int aprs_interval() throws AltosConfigDataException {
		if (aprs_interval_value.isVisible()) {
			String	s = aprs_interval_value.getSelectedItem().toString();

			return parse_int("aprs interval", s, false);
		}
		return AltosLib.MISSING;
	}

	public void set_aprs_ssid(int new_aprs_ssid) {
		if (new_aprs_ssid != AltosLib.MISSING)
			aprs_ssid_value.setSelectedItem(new_aprs_ssid);
		aprs_ssid_value.setVisible(new_aprs_ssid != AltosLib.MISSING);
		aprs_ssid_label.setVisible(new_aprs_ssid != AltosLib.MISSING);
		set_aprs_ssid_tool_tip();
	}

	public int aprs_ssid() throws AltosConfigDataException {
		if (aprs_ssid_value.isVisible()) {
			Integer i = (Integer) aprs_ssid_value.getSelectedItem();
			return i;
		}
		return AltosLib.MISSING;
	}

	public void set_aprs_format(int new_aprs_format) {
		if (new_aprs_format != AltosLib.MISSING)
			aprs_format_value.setSelectedIndex(new_aprs_format);
		aprs_format_value.setVisible(new_aprs_format != AltosLib.MISSING);
		aprs_format_label.setVisible(new_aprs_format != AltosLib.MISSING);
		set_aprs_format_tool_tip();
	}

	public int aprs_format() throws AltosConfigDataException {
		if (aprs_format_value.isVisible())
			return aprs_format_value.getSelectedIndex();
		return AltosLib.MISSING;
	}
	public void set_aprs_offset(int new_aprs_offset) {
		if (new_aprs_offset != AltosLib.MISSING)
			aprs_offset_value.setSelectedItem(new_aprs_offset);
		aprs_offset_value.setVisible(new_aprs_offset != AltosLib.MISSING);
		aprs_offset_label.setVisible(new_aprs_offset != AltosLib.MISSING);
		set_aprs_offset_tool_tip();
	}

	public int aprs_offset() throws AltosConfigDataException {
		if (aprs_offset_value.isVisible()) {
			Integer i = (Integer) aprs_offset_value.getSelectedItem();
			return i;
		}
		return AltosLib.MISSING;
	}
}
