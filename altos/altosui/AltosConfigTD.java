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

import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class AltosConfigTD implements ActionListener {

	class int_ref {
		int	value;

		public int get() {
			return value;
		}
		public void set(int i) {
			value = i;
		}
		public int_ref(int i) {
			value = i;
		}
	}

	class string_ref {
		String	value;

		public String get() {
			return value;
		}
		public void set(String i) {
			value = i;
		}
		public string_ref(String i) {
			value = i;
		}
	}

	JFrame		owner;
	AltosDevice	device;
	AltosSerial	serial_line;
	int_ref		serial;
	int_ref		radio_channel;
	int_ref		radio_calibration;
	int_ref		radio_setting;
	int_ref		radio_frequency;
	int_ref		telemetry_rate;
	string_ref	config_version;
	string_ref	version;
	string_ref	product;
	AltosConfigTDUI	config_ui;
	boolean		made_visible;

	boolean get_int(String line, String label, int_ref x) {
		if (line.startsWith(label)) {
			try {
				String tail = line.substring(label.length()).trim();
				String[] tokens = tail.split("\\s+");
				if (tokens.length > 0) {
					int	i = Integer.parseInt(tokens[0]);
					x.set(i);
					return true;
				}
			} catch (NumberFormatException ne) {
			}
		}
		return false;
	}

	boolean get_string(String line, String label, string_ref s) {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			s.set(quoted);
			return true;
		} else {
			return false;
		}
	}

	synchronized void update_ui() {
		config_ui.set_serial(serial.get());
		config_ui.set_product(product.get());
		config_ui.set_version(version.get());
		config_ui.set_radio_frequency(frequency());
		config_ui.set_radio_calibration(radio_calibration.get());
		config_ui.set_telemetry_rate(telemetry_rate.get());
		config_ui.set_clean();
		if (!made_visible) {
			made_visible = true;
			config_ui.make_visible();
		}
	}

	void finish_input(String line) {
		if (line == null) {
			abort();
			return;
		}
		if (line.equals("all finished")) {
			if (serial_line != null)
				update_ui();
			return;
		}
	}

	synchronized void process_line(String line) {
		if (line == null || line.equals("all finished")) {
			final String last_line = line;
			Runnable r = new Runnable() {
					public void run() {
						finish_input(last_line);
					}
				};
			SwingUtilities.invokeLater(r);
		} else {
			get_string(line, "Config version", config_version);
			get_int(line, "serial-number", serial);
			get_int(line, "Radio channel:", radio_channel);
			get_int(line, "Radio cal:", radio_calibration);
			get_int(line, "Frequency:", radio_frequency);
			get_int(line, "Radio setting:", radio_setting);
			get_int(line, "Telemetry rate:", telemetry_rate);
			get_string(line,"software-version", version);
			get_string(line,"product", product);
		}
	}

	synchronized void reset_data() {
		serial.set(0);
		radio_channel.set(0);
		radio_setting.set(0);
		radio_frequency.set(0);
		radio_calibration.set(1186611);
		telemetry_rate.set(Altos.ao_telemetry_rate_38400);
		config_version.set("0.0");
		version.set("unknown");
		product.set("unknown");
	}

	synchronized double frequency() {
		return AltosConvert.radio_to_frequency(radio_frequency.get(),
						       radio_setting.get(),
						       radio_calibration.get(),
						       radio_channel.get());
	}

	synchronized void set_frequency(double freq) {
		int	frequency = radio_frequency.get();
		int	setting = radio_setting.get();

		if (frequency > 0) {
			radio_frequency.set((int) Math.floor (freq * 1000 + 0.5));
		} else if (setting > 0) {
			radio_setting.set(AltosConvert.radio_frequency_to_setting(freq,
										  radio_calibration.get()));
			radio_channel.set(0);
		} else {
			radio_channel.set(AltosConvert.radio_frequency_to_channel(freq));
		}
	}

	synchronized int telemetry_rate() {
		return telemetry_rate.get();
	}

	synchronized void set_telemetry_rate(int new_telemetry_rate){
		int	rate = telemetry_rate.get();

		if (rate >= 0)
			telemetry_rate.set(new_telemetry_rate);
	}

	final static int	serial_mode_read = 0;
	final static int	serial_mode_save = 1;
	final static int	serial_mode_reboot = 2;

	SerialData serial_data;
	Thread serial_thread;

	class SerialData implements Runnable {
		AltosConfigTD	config;
		int		serial_mode;

		void get_data() {
			try {
				boolean	been_there = false;
				config.reset_data();

				while (config.serial_line != null) {
					config.serial_line.printf("c s\nf\nv\n");
					while (config.serial_line != null) {
						try {
							String line = config.serial_line.get_reply(5000);
							config.process_line(line);
							if (line != null && line.startsWith("software-version"))
								break;
						} catch (Exception e) {
							break;
						}
					}
					if (been_there)
						break;
					if (!config_version.get().equals("0.0"))
						break;
					been_there = true;
					if (config != null && config.serial_line != null) {
						config.serial_line.printf("C\n ");
						config.serial_line.flush_input();
					}
				}
			} catch (InterruptedException ie) {
			}
			/*
			 * This makes sure the displayed frequency respects the limits that the
			 * available firmware version might place on the actual frequency
			 */
			config.set_frequency(AltosPreferences.frequency(serial.get()));
			config.set_telemetry_rate(AltosPreferences.telemetry_rate(serial.get()));
			config.process_line("all finished");
		}

		void save_data() {
			double frequency = frequency();
			if (frequency != 0)
				AltosPreferences.set_frequency(serial.get(),
							       frequency);
			AltosPreferences.set_telemetry_rate(serial.get(),
							    telemetry_rate());
		}

		public void run () {
			switch (serial_mode) {
			case serial_mode_save:
				save_data();
				/* fall through ... */
			case serial_mode_read:
				get_data();
				serial_thread = null;
				break;
			}
		}

		public SerialData(AltosConfigTD in_config, int in_serial_mode) {
			config = in_config;
			serial_mode = in_serial_mode;
		}
	}

	void run_serial_thread(int serial_mode) {
		serial_data = new SerialData(this, serial_mode);
		serial_thread = new Thread(serial_data);
		serial_thread.start();
	}

	void abort_serial_thread() {
		if (serial_thread != null) {
			serial_thread.interrupt();
			serial_thread = null;
		}
	}
	void init_ui () throws InterruptedException, TimeoutException {
		config_ui = new AltosConfigTDUI(owner);
		config_ui.addActionListener(this);
		serial_line.set_frame(owner);
		set_ui();
	}

	void abort() {
		abort_serial_thread();
		if (serial_line != null) {
			serial_line.close();
			serial_line = null;
		}
		JOptionPane.showMessageDialog(owner,
					      String.format("Connection to \"%s\" failed",
							    device.toShortString()),
					      "Connection Failed",
					      JOptionPane.ERROR_MESSAGE);
		config_ui.setVisible(false);
	}

	void set_ui() throws InterruptedException, TimeoutException {
		if (serial_line != null)
			run_serial_thread(serial_mode_read);
		else
			update_ui();
	}

	void save_data() {
		double	freq = config_ui.radio_frequency();
		set_frequency(freq);
		int telemetry_rate = config_ui.telemetry_rate();
		set_telemetry_rate(telemetry_rate);
		run_serial_thread(serial_mode_save);
	}

	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();
		try {
			if (cmd.equals("Save")) {
				save_data();
			} else if (cmd.equals("Reset")) {
				set_ui();
			} else if (cmd.equals("Reboot")) {
				if (serial_line != null)
					run_serial_thread(serial_mode_reboot);
			} else if (cmd.equals("Close")) {
				if (serial_line != null)
					serial_line.close();
			}
		} catch (InterruptedException ie) {
			abort();
		} catch (TimeoutException te) {
			abort();
		}
	}

	public AltosConfigTD(JFrame given_owner) {
		owner = given_owner;

		serial = new int_ref(0);
		radio_channel = new int_ref(0);
		radio_setting = new int_ref(0);
		radio_frequency = new int_ref(0);
		radio_calibration = new int_ref(1186611);
		telemetry_rate = new int_ref(AltosLib.ao_telemetry_rate_38400);
		config_version = new string_ref("0.0");
		version = new string_ref("unknown");
		product = new string_ref("unknown");

		device = AltosDeviceUIDialog.show(owner, Altos.product_basestation);
		if (device != null) {
			try {
				serial_line = new AltosSerial(device);
				try {
					init_ui();
				} catch (InterruptedException ie) {
					abort();
				} catch (TimeoutException te) {
					abort();
				}
			} catch (FileNotFoundException ee) {
				JOptionPane.showMessageDialog(owner,
							      ee.getMessage(),
							      "Cannot open target device",
							      JOptionPane.ERROR_MESSAGE);
			} catch (AltosSerialInUseException si) {
				JOptionPane.showMessageDialog(owner,
							      String.format("Device \"%s\" already in use",
									    device.toShortString()),
							      "Device in use",
							      JOptionPane.ERROR_MESSAGE);
			}
		}
	}
}
