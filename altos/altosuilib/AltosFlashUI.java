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

package org.altusmetrum.altosuilib_14;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.io.*;
import java.util.concurrent.*;
import org.altusmetrum.altoslib_14.*;

public class AltosFlashUI
	extends AltosUIDialog
	implements ActionListener
{
	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		serial_value;
	JLabel		file_label;
	JLabel		file_value;
	JProgressBar	pbar;
	JButton		cancel;

	AltosUIFrame	frame;

	// Hex file with rom image
	File		file;

	// Debug connection
	AltosUSBDevice	device;

	AltosLink	link;

	// Desired Rom configuration
	AltosRomconfig	rom_config;

	// Flash controller
	AltosProgrammer	programmer;

	private static final String[] pair_programmed_files = {
		"teleballoon",
		"telebt-v1",
		"teledongle-v0",
		"telefire-v0",
		"telemetrum-v0",
		"telemetrum-v1",
		"telemini-v1",
		"telenano",
		"teleshield"
	};

	private static final String[] pair_programmed_devices = {
		"TeleBalloon",
		"TeleBT-v1",
		"TeleDongle-v0",
		"TeleFire-v0",
		"TeleFire",
		"TeleMetrum-v0",
		"TeleMetrum-v1",
		"TeleMini-v1",
		"TeleNano",
		"TeleShield"
	};

	private static final String[] log_erased_devices = {
		"TeleGPS"
	};

	private boolean is_pair_programmed() {

		if (file != null) {
			String	name = file.getName();
			for (int i = 0; i < pair_programmed_files.length; i++) {
				if (name.startsWith(pair_programmed_files[i]))
					return true;
			}
		}
		if (device != null) {
			String	name = device.toString();
			for (int i = 0; i < pair_programmed_devices.length; i++) {
				if (name.startsWith(pair_programmed_devices[i]))
					return true;
			}
		}
		return false;
	}

	private boolean is_log_erased() {
		if (device != null) {
			String	name = device.toString();
			for (int i = 0; i < log_erased_devices.length; i++) {
				if (name.startsWith(log_erased_devices[i]))
					return true;
			}
		}
		return false;
	}

	public void actionPerformed(ActionEvent e) {
		if (e.getSource() == cancel) {
			if (programmer != null)
				programmer.abort();
			setVisible(false);
			dispose();
		} else {
			String	cmd = e.getActionCommand();
			if (e.getID() == -1) {
				JOptionPane.showMessageDialog(frame,
							      e.getActionCommand(),
							      file.toString(),
							      JOptionPane.ERROR_MESSAGE);
				setVisible(false);
				dispose();
			} else if (cmd.equals(AltosFlashListener.flash_done)) {
				setVisible(false);
				dispose();
			} else if (cmd.equals(AltosFlashListener.flash_start)) {
				setVisible(true);
			} else {
				pbar.setValue(e.getID());
				pbar.setString(cmd);
			}
		}
	}

	public void build_dialog() {
		GridBagConstraints c;
		Insets il = new Insets(4,4,4,4);
		Insets ir = new Insets(4,4,4,4);

		pane = getScrollablePane();
		pane.setLayout(new GridBagLayout());

		c = new GridBagConstraints();
		c.gridx = 0; c.gridy = 0;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		serial_label = new JLabel("Serial:");
		pane.add(serial_label, c);

		c = new GridBagConstraints();
		c.gridx = 1; c.gridy = 0;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		serial_value = new JLabel("");
		pane.add(serial_value, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.gridx = 0; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = il;
		file_label = new JLabel("File:");
		pane.add(file_label, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		file_value = new JLabel(file.toString());
		pane.add(file_value, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(100);
		pbar.setValue(0);
		pbar.setString("");
		pbar.setStringPainted(true);
		pbar.setPreferredSize(new Dimension(600, 20));
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 2;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ib = new Insets(4,4,4,4);
		c.insets = ib;
		pane.add(pbar, c);

		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 3;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ic = new Insets(4,4,4,4);
		c.insets = ic;
		pane.add(cancel, c);
		cancel.addActionListener(this);
		pack();
		setLocationRelativeTo(frame);
	}

	void set_serial(int serial_number) {
		serial_value.setText(String.format("%d", serial_number));
	}

	static class AltosHexfileFilter extends javax.swing.filechooser.FileFilter {
		String head;
		String description;

		public AltosHexfileFilter(String usb_product) {
			int l;
			int dash;

			/* Trim off any trailing variants (1.0a vs 1.0) */
			for (dash = usb_product.length(); dash > 0; dash--) {
				char c = usb_product.charAt(dash-1);
				if (c == '-')
					break;
			}
			if (dash == 0)
				dash = usb_product.length();

			for (l = usb_product.length(); l > dash; l--) {
				char c = usb_product.charAt(l-1);
				if (c < 'a' || 'z' < c)
					break;
			}
			head = usb_product.substring(0, l).toLowerCase();
			description = String.format("%s Image File", usb_product);
		}

		public boolean accept(File file) {
			return !file.isFile() || (file.getName().startsWith(head) && file.getName().endsWith(".ihx"));
		}

		public String getDescription() {
			return description;
		}
	}

	boolean select_source_file() {
		JFileChooser	hexfile_chooser = new JFileChooser();

		File firmwaredir = AltosUIPreferences.firmwaredir();
		if (firmwaredir != null)
			hexfile_chooser.setCurrentDirectory(firmwaredir);

		hexfile_chooser.setDialogTitle("Select Flash Image");

		javax.swing.filechooser.FileFilter ihx_filter = new FileNameExtensionFilter("Flash Image", "ihx");
		hexfile_chooser.addChoosableFileFilter(ihx_filter);
		hexfile_chooser.setFileFilter(ihx_filter);

		if (!is_pair_programmed() && !device.matchProduct(AltosLib.product_altusmetrum)) {
			AltosHexfileFilter filter = new AltosHexfileFilter(device.usb_product());
			hexfile_chooser.addChoosableFileFilter(filter);
			hexfile_chooser.setFileFilter(filter);
		}

		int returnVal = hexfile_chooser.showOpenDialog(frame);

		if (returnVal != JFileChooser.APPROVE_OPTION)
			return false;
		file = hexfile_chooser.getSelectedFile();
		if (file == null)
			return false;
		AltosUIPreferences.set_firmwaredir(file.getParentFile());

		return true;
	}

	boolean select_device() {
		int	product = AltosLib.product_any;

		device = AltosDeviceUIDialog.show_usb(frame, AltosLib.product_any);

		if (device == null)
			return false;
		return true;
	}

	boolean rom_config_matches (AltosRomconfig a, AltosRomconfig b) {
		if (a == null || b == null)
			return (a == null && b == null);

		if (!a.valid || !b.valid)
			return false;

		if (a.usb_id != null && b.usb_id != null &&
		    (a.usb_id.vid != b.usb_id.vid ||
		     a.usb_id.pid != b.usb_id.pid))
			return false;

		if (a.usb_product != null && b.usb_product != null &&
		    !a.usb_product.equals(b.usb_product))
			return false;

		return true;
	}

	boolean update_rom_config_info(AltosRomconfig existing_config, AltosRomconfig image_config) {
		AltosRomconfig	new_config;

		if (!rom_config_matches(existing_config, image_config)) {
			int ret;
			if (existing_config == null || !existing_config.valid) {
				ret = JOptionPane.showConfirmDialog(this,
								    String.format("Cannot determine target device type\nImage is %04x:%04x %s\nFlash anyways?",
										  image_config.usb_id.vid,
										  image_config.usb_id.pid,
										  image_config.usb_product),
								    "Unknown Target Device",
								    JOptionPane.YES_NO_OPTION);
			} else {
				ret = JOptionPane.showConfirmDialog(this,
								    String.format("Device is %04x:%04x %s\nImage is %04x:%04x %s\nFlash anyways?",
										  existing_config.usb_id.vid,
										  existing_config.usb_id.pid,
										  existing_config.usb_product,
										  image_config.usb_id.vid,
										  image_config.usb_id.pid,
										  image_config.usb_product),
								    "Image doesn't match Device",
								    JOptionPane.YES_NO_OPTION);
			}
			if (ret != JOptionPane.YES_OPTION)
				return false;
		}

		if (existing_config != null && existing_config.radio_calibration_broken) {
			int ret = JOptionPane.showConfirmDialog(this,
								String.format("Radio calibration value %d may be incorrect\nFlash anyways?",
									      existing_config.radio_calibration),
								"Radio Calibration Invalid",
								JOptionPane.YES_NO_OPTION);
			if (ret != JOptionPane.YES_OPTION)
				return false;
		}


		new_config = AltosRomconfigUI.show(frame, existing_config);
		if (new_config == null)
			return false;
		rom_config = new_config;
		set_serial(rom_config.serial_number);
		setVisible(true);
		return true;
	}

	void exception (Exception e) {
		if (e instanceof FileNotFoundException) {
			JOptionPane.showMessageDialog(frame,
						      ((FileNotFoundException) e).getMessage(),
						      "Cannot open file",
						      JOptionPane.ERROR_MESSAGE);
		} else if (e instanceof AltosSerialInUseException) {
			JOptionPane.showMessageDialog(frame,
						      String.format("Device \"%s\" already in use",
								    device.toShortString()),
						      "Device in use",
						      JOptionPane.ERROR_MESSAGE);
		} else {
			JOptionPane.showMessageDialog(frame,
						      e.getMessage(),
						      file.toString(),
						      JOptionPane.ERROR_MESSAGE);
		}
	}

	class flash_task implements Runnable, AltosFlashListener {
		AltosFlashUI	ui;
		Thread		t;
		AltosProgrammer	programmer;

		public void position(String in_s, int in_percent) {
			final String s = in_s;
			final int percent = in_percent;
			Runnable r = new Runnable() {
					public void run() {
						try {
							ui.actionPerformed(new ActionEvent(this,
											   percent,
											   s));
						} catch (Exception ex) {
						}
					}
				};
			SwingUtilities.invokeLater(r);
		}

		public void run () {
			try {
				if (ui.is_pair_programmed())
					programmer = new AltosFlash(ui.file, link, this);
				else
					programmer = new AltosSelfFlash(ui.file, link, this);

				final AltosRomconfig	current_config = programmer.target_romconfig(device.usb_id(), device.usb_product());

				final AltosRomconfig	image_config = programmer.image_romconfig();

				System.out.printf("product %s current %s image %s\n", device.usb_product(), current_config, image_config);

				final Semaphore await_rom_config = new Semaphore(0);
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.programmer = programmer;
							ui.update_rom_config_info(current_config, image_config);
							await_rom_config.release();
						}
					});
				await_rom_config.acquire();

				if (ui.rom_config != null) {
					programmer.set_romconfig(ui.rom_config);
					programmer.flash();
				}
			} catch (InterruptedException ee) {
				final Exception	e = ee;
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.exception(e);
						}
					});
			} catch (IOException ee) {
				final Exception	e = ee;
				SwingUtilities.invokeLater(new Runnable() {
						public void run() {
							ui.exception(e);
						}
					});
			} finally {
				if (programmer != null)
					programmer.close();
			}
		}

		public flash_task(AltosFlashUI in_ui) {
			ui = in_ui;
			t = new Thread(this);
			t.start();
		}
	}

	flash_task	flasher;

	boolean erase_answer;

	class open_task implements Runnable {
		AltosDevice	device;
		Thread		t;
		open_dialog	dialog;
		AltosLink 	link;

		public void do_exception(final Exception e) {
			if (link != null) {
				try {
					link.close();
				} catch (Exception ex) {}
			}
			SwingUtilities.invokeLater(
				new Runnable() {
					public void run() {
						try { dialog.open_exception(e); } catch (Exception ex) { }
					}
				});
		}

		public void do_success(final AltosLink link) {
			SwingUtilities.invokeLater(
				new Runnable() {
					public void run() {
						try { dialog.open_success(link); } catch (Exception ex) { }
					}
				});
		}

		public boolean do_notify_erase(final AltosConfigData config_data) {
			erase_answer = false;
			final Semaphore erase_answer_done = new Semaphore(0);
			SwingUtilities.invokeLater(
				new Runnable() {
					public void run() {
						int ret = JOptionPane.showConfirmDialog(dialog.owner,
											   String.format("Updating %s from firmware %s will erase stored data, continue?",
													 config_data.product,
													 config_data.version),
											   "Erase Stored Data?",
											   JOptionPane.YES_NO_OPTION);
						erase_answer = ret == JOptionPane.YES_OPTION;
						erase_answer_done.release();
					}
				});
			try {
				erase_answer_done.acquire();
			} catch (Exception ex) {
				return false;
			}
			return erase_answer;
		}

		public void run () {
			link = null;
			try {
				boolean new_device = false;

				for (;;) {
					System.out.printf("Attempting to open %s\n", device.toShortString());

					for (int i = 0; i < 20; i++) {
						link = new AltosSerial(device);

						if (link != null)
							break;

						if (!new_device)
							break;

						System.out.printf("Waiting for device to become ready\n");

						Thread.sleep(1000);
					}
					if (link == null)
						throw new IOException(String.format("%s: open failed",
										    device.toShortString()));

					System.out.printf("Checking device ready\n");

					/* See if the link is usable already */
					if (is_pair_programmed() || link.is_loader()) {
						System.out.printf("Device ready for use\n");
						do_success(link);
						return;
					}

					System.out.printf("Checking log erased\n");

					if (is_log_erased()) {
						System.out.printf("Fetching config data\n");
						AltosConfigData config_data = link.config_data();
						System.out.printf("version %s\n", config_data.version);
						/* Completely erase TeleGPS flash when firmware is old */
						if (config_data.compare_version("1.9.7") < 0)
						{
							if (!do_notify_erase(config_data))
								throw new IOException(String.format("%s: not erasing log",
												    device.toShortString()));
							System.out.printf("Erasing log\n");
							link.printf("Z DoIt\n");
							link.synchronize(120 * 1000);
						}
					}

					java.util.List<AltosDevice> prev_devices =
						AltosUSBDevice.list(AltosLib.product_altusmetrum);

					/* Nope, switch to loader and
					 * wait for it to re-appear
					 */

					System.out.printf("Switch to loader\n");

					link.to_loader();

					/* This is a bit fragile, but
					 * I'm not sure what else to
					 * do other than ask the user.
					 *
					 * Search for a device which
					 * wasn't there before we
					 * asked the target to switch
					 * to loader mode
					 */

					device = null;
					for (;;) {
						Thread.sleep(100);
						java.util.List<AltosDevice> devices =
							AltosUSBDevice.list(AltosLib.product_altusmetrum);

						for (AltosDevice d : devices) {
							boolean matched = false;
							System.out.printf("\tfound device %s\n", d.toShortString());
							for (AltosDevice p : prev_devices)
								if (d.equals(p)) {
									matched = true;
									break;
								}
							if (!matched) {
								System.out.printf("Identified new device %s\n", d.toShortString());
								device = (AltosUSBDevice) d;
								new_device = true;
								break;
							}
						}
						if (device != null)
							break;
					}
				}
			} catch (AltosSerialInUseException ee) {
				do_exception(ee);
			} catch (FileNotFoundException fe) {
				do_exception(fe);
			} catch (IOException ie) {
				do_exception (ie);
			} catch (TimeoutException te) {
				do_exception (te);
			} catch (InterruptedException ie) {
				do_exception (ie);
			}
		}

		public void cancel() {
			t.interrupt();
		}

		public open_task(AltosDevice device, open_dialog dialog) {
			this.device = device;
			this.dialog = dialog;
			t = new Thread(this);
			t.start();
		}
	}

	class open_dialog
		extends AltosUIDialog
		implements ActionListener
	{
		AltosUIFrame owner;

		private JLabel	opening_label;
		private JButton	cancel_button;

		boolean done = false;

		AltosLink link = null;

		open_task open = null;

		public void open_exception(Exception e) {
			System.out.printf("open_exception\n");
			setVisible(false);
			exception(e);
			done = true;
		}

		public void open_success(AltosLink link) {
			System.out.printf("open_success\n");
			setVisible(false);
			this.link = link;
			done = true;
		}

		public AltosLink do_open(open_task open) throws InterruptedException {
			this.open = open;
			setVisible(true);
			return link;
		}

		public void actionPerformed(ActionEvent e) {
			String cmd = e.getActionCommand();

			if (cmd.equals("cancel"))
				if (open != null)
					open.cancel();
			done = true;
			setVisible(false);
		}

		public open_dialog(AltosUIFrame in_owner) {
			super(in_owner, "Open Flash Target Device", true);
			owner = in_owner;

			Container		pane = getScrollablePane();
			GridBagConstraints	c = new GridBagConstraints();
			Insets			i = new Insets(4,4,4,4);


			pane.setLayout(new GridBagLayout());

			opening_label = new JLabel("Opening Device");
			c.fill = GridBagConstraints.HORIZONTAL;
			c.anchor = GridBagConstraints.LINE_START;
			c.insets = i;
			c.weightx = 0;
			c.weighty = 0;

			c.gridx = 0;
			c.gridy = 0;

			pane.add(opening_label, c);

			cancel_button = new JButton("Cancel");
			cancel_button.addActionListener(this);
			cancel_button.setActionCommand("cancel");

			c.gridy = 1;
			pane.add(cancel_button, c);
			pack();
			setLocationRelativeTo(owner);
		}
	}

	private boolean open_device() throws InterruptedException {

		open_dialog	dialog = new open_dialog(frame);

		open_task	open = new open_task(device, dialog);

		link = dialog.do_open(open);

		return link != null;
	}

	/*
	 * Execute the steps for flashing
	 * a device. Note that this returns immediately;
	 * this dialog is not modal
	 */
	void showDialog() {
		if (!select_device())
			return;
		if (!select_source_file())
			return;
		try {
			if (!open_device())
				return;
		} catch (InterruptedException ie) {
			return;
		}
		build_dialog();
		flash_task	f = new flash_task(this);
	}

	public static void show(AltosUIFrame frame) {
		AltosFlashUI	ui = new AltosFlashUI(frame);
		ui.showDialog();
	}

	public AltosFlashUI(AltosUIFrame in_frame) {
		super(in_frame, "Program Altusmetrum Device", false);

		frame = in_frame;
	}
}
