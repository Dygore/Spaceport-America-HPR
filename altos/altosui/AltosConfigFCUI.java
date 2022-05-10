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
import javax.swing.event.*;
import java.text.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class AltosConfigFCUI
	extends AltosUIDialog
	implements ActionListener, ItemListener, DocumentListener, AltosConfigValues, AltosUnitsListener
{

	Container		pane;
	JLabel			product_label;
	JLabel			version_label;
	JLabel			serial_label;
	JLabel			main_deploy_label;
	JLabel			apogee_delay_label;
	JLabel			apogee_lockout_label;
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
	JLabel			ignite_mode_label;
	JLabel			pad_orientation_label;
	JLabel			accel_plus_label;
	JLabel			accel_minus_label;
	JLabel			callsign_label;
	JLabel			beep_label;
	JLabel			tracker_motion_label;
	JLabel			tracker_interval_label;

	public boolean		dirty;

	JFrame			owner;
	JLabel			product_value;
	JLabel			version_value;
	JLabel			serial_value;
	JComboBox<String>	main_deploy_value;
	JComboBox<String>	apogee_delay_value;
	JComboBox<String>	apogee_lockout_value;
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
	JComboBox<String>	ignite_mode_value;
	JComboBox<String>	pad_orientation_value;
	JTextField		accel_plus_value;
	JTextField		accel_minus_value;
	JTextField		callsign_value;
	JComboBox<String>	beep_value;
	JComboBox<String>	tracker_motion_value;
	JComboBox<String>	tracker_interval_value;

	JButton			pyro;
	JButton			accel_cal;

	JButton			save;
	JButton			reset;
	JButton			reboot;
	JButton			close;

	AltosPyro[]		pyros;
	double			pyro_firing_time;

	ActionListener		listener;

	static final String	title = "Configure Flight Computer";

	static String[] 	main_deploy_values_m = {
		"100", "150", "200", "250", "300", "350",
		"400", "450", "500"
	};

	static String[] 	main_deploy_values_ft = {
		"250", "500", "750", "1000", "1250", "1500",
		"1750", "2000"
	};

	static String[] 	apogee_delay_values = {
		"0", "1", "2", "3", "4", "5"
	};

	static String[] 	apogee_lockout_values = {
		"0", "5", "10", "15", "20"
	};

	static String[] 	ignite_mode_values = {
		"Dual Deploy",
		"Redundant Apogee",
		"Redundant Main",
		"Separation & Apogee",
	};

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

	static String[] 	beep_values = {
		"2000",
		"2100",
		"2200",
		"3750",
		"4000",
		"4250",
	};

	static String[] 	pad_orientation_values_radio = {
		"Antenna Up",
		"Antenna Down",
	};

	static String[] 	pad_orientation_values_no_radio = {
		"Beeper Up",
		"Beeper Down",
	};

	String[] pad_orientation_values;

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
		AltosConfigFCUI	ui;

		public ConfigListener(AltosConfigFCUI this_ui) {
			ui = this_ui;
		}

		public void windowClosing(WindowEvent e) {
			ui.actionPerformed(new ActionEvent(e.getSource(),
							   ActionEvent.ACTION_PERFORMED,
							   "Close"));
		}
	}

	boolean is_telemini_v1() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMini-v1");
	}

	boolean is_telemini() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMini");
	}

	boolean is_easymini() {
		String	product = product_value.getText();
		return product != null && product.startsWith("EasyMini");
	}

	boolean is_telemetrum() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMetrum");
	}

	boolean is_telemega() {
		String	product = product_value.getText();
		return product != null && product.startsWith("TeleMega");
	}

	boolean is_easymega() {
		String	product = product_value.getText();
		return product != null && product.startsWith("EasyMega");
	}

	boolean is_easytimer() {
		String	product = product_value.getText();
		return product != null && product.startsWith("EasyTimer");
	}

	boolean has_radio() {
		return is_telemega() || is_telemetrum() || is_telemini();
	}

	void set_radio_enable_tool_tip() {
		if (radio_enable_value.isVisible())
			radio_enable_value.setToolTipText("Enable/Disable telemetry and RDF transmissions");
		else
			radio_enable_value.setToolTipText("Firmware version does not support disabling radio");
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
		else {
			if (is_telemini_v1())
				flight_log_max_value.setToolTipText("TeleMini-v1 stores only one flight");
			else
				flight_log_max_value.setToolTipText("Cannot set max value with flight logs in memory");
		}
	}

	void set_ignite_mode_tool_tip() {
		if (ignite_mode_value.isVisible())
			ignite_mode_value.setToolTipText("Select when igniters will be fired");
		else
			ignite_mode_value.setToolTipText("Older firmware could not select ignite mode");
	}

	void set_pad_orientation_tool_tip() {
		if (pad_orientation_value.isVisible()) {
			pad_orientation_value.setToolTipText("How will the computer be mounted in the airframe");
		} else {
			if (is_telemetrum())
				pad_orientation_value.setToolTipText("Older TeleMetrum firmware must fly antenna forward");
			else if (is_telemini() || is_easymini())
				pad_orientation_value.setToolTipText("TeleMini and EasyMini don't care how they are mounted");
			else if (is_easytimer())
				pad_orientation_value.setToolTipText("EasyTimer can be mounted in any of six orientations");
			else
				pad_orientation_value.setToolTipText("Can't select orientation");
		}
	}

	void set_pad_orientation_values() {
		String [] new_values;
		if (has_radio())
			new_values = pad_orientation_values_radio;
		else
			new_values = pad_orientation_values_no_radio;
		if (new_values != pad_orientation_values) {
			int id = pad_orientation_value.getSelectedIndex();
			pad_orientation_value.removeAllItems();
			pad_orientation_values = new_values;
			for (int i = 0; i < new_values.length; i++)
				pad_orientation_value.addItem(pad_orientation_values[i]);
			pad_orientation_value.setSelectedIndex(id);
		}
	}

	void set_accel_tool_tips() {
		if (accel_plus_value.isVisible()) {
			accel_plus_value.setToolTipText("Pad acceleration value in flight orientation");
			accel_minus_value.setToolTipText("Upside-down acceleration value");
		} else {
			accel_plus_value.setToolTipText("No accelerometer");
			accel_minus_value.setToolTipText("No accelerometer");
		}
	}

	void set_beep_tool_tip() {
		if (beep_value.isVisible())
			beep_value.setToolTipText("What frequency the beeper will sound at");
		else
			beep_value.setToolTipText("Older firmware could not select beeper frequency");
	}

	void set_radio_10mw_tool_tip() {
		if (radio_10mw_value.isVisible())
			radio_10mw_value.setToolTipText("Should transmitter power be limited to 10mW");
		else
			radio_10mw_value.setToolTipText("Older firmware could not limit radio power");
	}

	/* Build the UI using a grid bag */
	public AltosConfigFCUI(JFrame in_owner, boolean remote) {
		super (in_owner, title, false);

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

		/* Main deploy */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		main_deploy_label = new JLabel(get_main_deploy_label());
		pane.add(main_deploy_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		main_deploy_value = new JComboBox<String>(main_deploy_values());
		main_deploy_value.setEditable(true);
		main_deploy_value.addItemListener(this);
		pane.add(main_deploy_value, c);
		main_deploy_value.setToolTipText("Height above pad altitude to fire main charge");
		row++;

		/* Apogee delay */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		apogee_delay_label = new JLabel("Apogee Delay(s):");
		pane.add(apogee_delay_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		apogee_delay_value = new JComboBox<String>(apogee_delay_values);
		apogee_delay_value.setEditable(true);
		apogee_delay_value.addItemListener(this);
		pane.add(apogee_delay_value, c);
		apogee_delay_value.setToolTipText("Delay after apogee before charge fires");
		row++;

		/* Apogee lockout */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		apogee_lockout_label = new JLabel("Apogee Lockout(s):");
		pane.add(apogee_lockout_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		apogee_lockout_value = new JComboBox<String>(apogee_lockout_values);
		apogee_lockout_value.setEditable(true);
		apogee_lockout_value.addItemListener(this);
		pane.add(apogee_lockout_value, c);
		apogee_lockout_value.setToolTipText("Time after launch while apogee detection is locked out");
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
		flight_log_max_label = new JLabel("Maximum Flight Log Size (kB):");
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

		/* Ignite mode */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		ignite_mode_label = new JLabel("Igniter Firing Mode:");
		pane.add(ignite_mode_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		ignite_mode_value = new JComboBox<String>(ignite_mode_values);
		ignite_mode_value.setEditable(false);
		ignite_mode_value.addItemListener(this);
		pane.add(ignite_mode_value, c);
		set_ignite_mode_tool_tip();
		row++;

		/* Pad orientation */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		pad_orientation_label = new JLabel("Pad Orientation:");
		pane.add(pad_orientation_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		pad_orientation_values = pad_orientation_values_no_radio;

		pad_orientation_value = new JComboBox<String>(pad_orientation_values);
		pad_orientation_value.setEditable(false);
		pad_orientation_value.addItemListener(this);
		pane.add(pad_orientation_value, c);
		set_pad_orientation_tool_tip();
		row++;

		/* Accel plus */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		accel_plus_label = new JLabel("Accel Plus:");
		pane.add(accel_plus_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		accel_plus_value = new JTextField(10);
		accel_plus_value.setEditable(true);
		accel_plus_value.getDocument().addDocumentListener(this);
		pane.add(accel_plus_value, c);
		row++;

		/* Accel minus */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		accel_minus_label = new JLabel("Accel Minus:");
		pane.add(accel_minus_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		accel_minus_value = new JTextField(10);
		accel_minus_value.setEditable(true);
		accel_minus_value.getDocument().addDocumentListener(this);
		pane.add(accel_minus_value, c);
		row++;
		set_accel_tool_tips();

		/* Beeper */
		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		beep_label = new JLabel("Beeper Frequency:");
		pane.add(beep_label, c);

		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		c.ipady = 5;
		beep_value = new JComboBox<String>(beep_values);
		beep_value.setEditable(true);
		beep_value.addItemListener(this);
		pane.add(beep_value, c);
		set_beep_tool_tip();
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
		tracker_interval_label = new JLabel("Position Reporting Interval(s):");
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

		/* Pyro channels */
		c = new GridBagConstraints();
		c.gridx = 4; c.gridy = row;
		c.gridwidth = 4;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		pyro = new JButton("Configure Pyro Channels");
		pane.add(pyro, c);
		pyro.addActionListener(this);
		pyro.setActionCommand("Pyro");
		row++;

		/* Accel cal */
		c = new GridBagConstraints();
		c.gridx = 5; c.gridy = row;
		c.gridwidth = 5;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		c.ipady = 5;
		accel_cal = new JButton("Calibrate Accelerometer");
		pane.add(accel_cal, c);
		accel_cal.addActionListener(this);
		accel_cal.setActionCommand("Accel");
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
		setTitle(title + " (modified)");
		save.setEnabled(true);
	}

	public void set_clean() {
		dirty = false;
		setTitle(title);
		save.setEnabled(false);
	}

	AltosConfigPyroUI	pyro_ui;

	public void dispose() {
		if (pyro_ui != null)
			pyro_ui.dispose();
		AltosPreferences.unregister_units_listener(this);
		super.dispose();
	}

	/* Listen for events from our buttons */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if (cmd.equals("Pyro")) {
			if (pyro_ui == null && pyros != null)
				pyro_ui = new AltosConfigPyroUI(this, pyros, pyro_firing_time);
			if (pyro_ui != null)
				pyro_ui.make_visible();
			return;
		}

		if (cmd.equals("Close") || cmd.equals("Reboot"))
			if (!check_dirty(cmd))
				return;
		if (cmd.equals("Save"))
			save.setEnabled(false);
		listener.actionPerformed(e);
		if (cmd.equals("Close") || cmd.equals("Reboot")) {
			setVisible(false);
			dispose();
		}
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

	/* set and get all of the dialog values */
	public void set_product(String product) {
		radio_frequency_value.set_product(product);
		product_value.setText(product);
		set_pad_orientation_tool_tip();
		set_accel_tool_tips();
		set_flight_log_max_tool_tip();
		set_pad_orientation_values();
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
		if (new_main_deploy != AltosLib.MISSING)
			main_deploy_value.setSelectedItem(AltosConvert.height.say(new_main_deploy));
		main_deploy_value.setVisible(new_main_deploy != AltosLib.MISSING);
		main_deploy_label.setVisible(new_main_deploy != AltosLib.MISSING);
	}

	public int main_deploy() throws AltosConfigDataException {
		String	str = main_deploy_value.getSelectedItem().toString();
		try {
			return (int) (AltosConvert.height.parse_locale(str) + 0.5);
		} catch (ParseException pe) {
			throw new AltosConfigDataException("invalid main deploy height %s", str);
		}
	}

	String get_main_deploy_label() {
		return String.format("Main Deploy Altitude(%s):", AltosConvert.height.parse_units());
	}

	String[] main_deploy_values() {
		if (AltosConvert.imperial_units)
			return main_deploy_values_ft;
		else
			return main_deploy_values_m;
	}

	void set_main_deploy_values() {
		String[]	v = main_deploy_values();
		while (main_deploy_value.getItemCount() > 0)
			main_deploy_value.removeItemAt(0);
		for (int i = 0; i < v.length; i++)
			main_deploy_value.addItem(v[i]);
		main_deploy_value.setMaximumRowCount(v.length);
	}

	public void units_changed(boolean imperial_units) {
		boolean	was_dirty = dirty;

		String v = main_deploy_value.getSelectedItem().toString();
		main_deploy_label.setText(get_main_deploy_label());
		set_main_deploy_values();
		try {
			int m = (int) (AltosConvert.height.parse_locale(v, !imperial_units) + 0.5);
			set_main_deploy(m);
		} catch (ParseException pe) {
		}

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

	public void set_apogee_delay(int new_apogee_delay) {
		if (new_apogee_delay != AltosLib.MISSING)
			apogee_delay_value.setSelectedItem(Integer.toString(new_apogee_delay));
		apogee_delay_value.setVisible(new_apogee_delay != AltosLib.MISSING);
		apogee_delay_label.setVisible(new_apogee_delay != AltosLib.MISSING);
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

	public int apogee_delay() throws AltosConfigDataException {
		return parse_int("apogee delay", apogee_delay_value.getSelectedItem().toString(), false);
	}

	public void set_apogee_lockout(int new_apogee_lockout) {
		if (new_apogee_lockout != AltosLib.MISSING)
			apogee_lockout_value.setSelectedItem(Integer.toString(new_apogee_lockout));

		apogee_lockout_value.setVisible(new_apogee_lockout != AltosLib.MISSING);
		apogee_lockout_label.setVisible(new_apogee_lockout != AltosLib.MISSING);
	}

	public int apogee_lockout() throws AltosConfigDataException {
		return parse_int("apogee lockout", apogee_lockout_value.getSelectedItem().toString(), false);
	}

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
		radio_calibration_value.setVisible(new_radio_calibration != AltosLib.MISSING);
		radio_calibration_label.setVisible(new_radio_calibration != AltosLib.MISSING);
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

	public void set_telemetry_rate(int new_rate) {
		if (new_rate != AltosLib.MISSING)
			rate_value.set_rate(new_rate);
		rate_label.setVisible(new_rate != AltosLib.MISSING);
		rate_value.setVisible(new_rate != AltosLib.MISSING);
		set_rate_tool_tip();
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
		if (new_flight_log_max != AltosLib.MISSING) {
			flight_log_max_value.setSelectedItem(flight_log_max_label(new_flight_log_max));
			flight_log_max = new_flight_log_max;
		}
		flight_log_max_value.setVisible(new_flight_log_max != AltosLib.MISSING);
		flight_log_max_label.setVisible(new_flight_log_max != AltosLib.MISSING);
		set_flight_log_max_tool_tip();
	}

	public void set_flight_log_max_enabled(boolean enable) {
		flight_log_max_value.setEnabled(enable);
		set_flight_log_max_tool_tip();
	}

	public int flight_log_max() throws AltosConfigDataException {
		if (flight_log_max_value.isVisible())
			return parse_int("flight log max", flight_log_max_value.getSelectedItem().toString(), true);
		return AltosLib.MISSING;
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

	public void set_ignite_mode(int new_ignite_mode) {
		if (new_ignite_mode != AltosLib.MISSING) {
			if (new_ignite_mode >= ignite_mode_values.length)
				new_ignite_mode = 0;
			if (new_ignite_mode < 0) {
				ignite_mode_value.setEnabled(false);
				new_ignite_mode = 0;
			} else {
				ignite_mode_value.setEnabled(true);
			}
			ignite_mode_value.setSelectedIndex(new_ignite_mode);
		}
		ignite_mode_value.setVisible(new_ignite_mode != AltosLib.MISSING);
		ignite_mode_label.setVisible(new_ignite_mode != AltosLib.MISSING);

		set_ignite_mode_tool_tip();
	}

	public int ignite_mode() {
		if (ignite_mode_value.isVisible())
			return ignite_mode_value.getSelectedIndex();
		else
			return AltosLib.MISSING;
	}


	public void set_pad_orientation(int new_pad_orientation) {
		if (new_pad_orientation != AltosLib.MISSING) {
			if (new_pad_orientation >= pad_orientation_values.length)
				new_pad_orientation = 0;
			if (new_pad_orientation < 0)
				new_pad_orientation = 0;
			pad_orientation_value.setSelectedIndex(new_pad_orientation);
		}
		pad_orientation_value.setVisible(new_pad_orientation != AltosLib.MISSING);
		pad_orientation_label.setVisible(new_pad_orientation != AltosLib.MISSING);
		accel_cal.setVisible(new_pad_orientation != AltosLib.MISSING);

		set_pad_orientation_tool_tip();
	}

	public int pad_orientation() {
		if (pad_orientation_value.isVisible())
			return pad_orientation_value.getSelectedIndex();
		else
			return AltosLib.MISSING;
	}

	public void set_accel_cal(int accel_plus, int accel_minus) {
		if (accel_plus != AltosLib.MISSING) {
			accel_plus_value.setText(String.format("%d", accel_plus));
			accel_minus_value.setText(String.format("%d", accel_minus));
		}
		accel_plus_value.setVisible(accel_plus != AltosLib.MISSING);
		accel_plus_label.setVisible(accel_plus != AltosLib.MISSING);
		accel_minus_value.setVisible(accel_minus != AltosLib.MISSING);
		accel_minus_label.setVisible(accel_minus != AltosLib.MISSING);

		set_accel_tool_tips();
	}

	public int accel_cal_plus() {
		if (accel_plus_value.isVisible())
			return Integer.parseInt(accel_plus_value.getText());
		return AltosLib.MISSING;
	}

	public int accel_cal_minus() {
		if (accel_minus_value.isVisible())
			return Integer.parseInt(accel_minus_value.getText());
		return AltosLib.MISSING;
	}

	public void set_beep(int new_beep) {
		if (new_beep != AltosLib.MISSING) {
			int new_freq = (int) Math.floor (AltosConvert.beep_value_to_freq(new_beep) + 0.5);
			for (int i = 0; i < beep_values.length; i++)
				if (new_beep == AltosConvert.beep_freq_to_value(Integer.parseInt(beep_values[i]))) {
					beep_value.setSelectedIndex(i);
					set_beep_tool_tip();
					return;
				}
			beep_value.setSelectedItem(String.format("%d", new_freq));
		}
		beep_value.setVisible(new_beep != AltosLib.MISSING);
		beep_label.setVisible(new_beep != AltosLib.MISSING);
		set_beep_tool_tip();
	}

	public int beep() {
		if (beep_value.isVisible())
			return AltosConvert.beep_freq_to_value(Integer.parseInt(beep_value.getSelectedItem().toString()));
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

	public void set_pyros(AltosPyro[] new_pyros) {
		pyros = new_pyros;
		if (pyros != null && pyro_ui != null)
			pyro_ui.set_pyros(pyros);
		pyro.setVisible(pyros != null);
	}

	public AltosPyro[] pyros() throws AltosConfigDataException {
		if (pyro_ui != null)
			pyros = pyro_ui.get_pyros();
		return pyros;
	}

	public void set_pyro_firing_time(double new_pyro_firing_time) {
		pyro_firing_time = new_pyro_firing_time;
		if (pyro_firing_time != AltosLib.MISSING && pyro_ui != null)
			pyro_ui.set_pyro_firing_time(pyro_firing_time);
		pyro.setVisible(pyro_firing_time != AltosLib.MISSING);
	}

	public double pyro_firing_time() throws AltosConfigDataException {
		if (pyro_ui != null)
			pyro_firing_time = pyro_ui.get_pyro_firing_time();
		return pyro_firing_time;
	}

	private String aprs_interval_string(int interval) {
		if (interval == 0)
			return "Disabled";
		return Integer.toString(interval);
	}

	private int aprs_interval_value(String interval) throws AltosConfigDataException {
		if (interval.equalsIgnoreCase("Disabled"))
			return 0;
		return parse_int("aprs interval", interval, false);
	}

	public void set_aprs_interval(int new_aprs_interval) {
		if (new_aprs_interval != AltosLib.MISSING)
			aprs_interval_value.setSelectedItem(aprs_interval_string(new_aprs_interval));
		aprs_interval_value.setVisible(new_aprs_interval != AltosLib.MISSING);
		aprs_interval_label.setVisible(new_aprs_interval != AltosLib.MISSING);
		set_aprs_interval_tool_tip();
	}

	public int aprs_interval() throws AltosConfigDataException {
		if (aprs_interval_value.isVisible())
			return aprs_interval_value(aprs_interval_value.getSelectedItem().toString());
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
