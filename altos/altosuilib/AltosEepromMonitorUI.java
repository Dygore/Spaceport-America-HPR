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

import java.io.*;
import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;

	class result_holder {
		static int result;
	}

public class AltosEepromMonitorUI extends AltosUIDialog implements AltosEepromMonitor {
	JFrame		owner;
	Container	pane;
	Box		box;
	JLabel		serial_label;
	JLabel		flight_label;
	JLabel		file_label;
	JLabel		serial_value;
	JLabel		flight_value;
	JButton		cancel;
	JProgressBar	pbar;
	ActionListener	listener;

	static final int	progress_max = 10000;

	public AltosEepromMonitorUI(JFrame owner) {
		super (owner, "Download Flight Data", false);

		setMinimumSize(new Dimension(600, 100));

		this.owner = owner;

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
		flight_label = new JLabel("Flight:");
		pane.add(flight_label, c);

		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.gridx = 1; c.gridy = 1;
		c.anchor = GridBagConstraints.LINE_START;
		c.insets = ir;
		flight_value = new JLabel("");
		pane.add(flight_value, c);

		pbar = new JProgressBar();
		pbar.setMinimum(0);
		pbar.setMaximum(progress_max);
		pbar.setStringPainted(true);
		set_block_internal(0);
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 3;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ib = new Insets(4,4,4,4);
		c.insets = ib;
		pane.add(pbar, c);

		cancel = new JButton("Cancel");
		c = new GridBagConstraints();
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.CENTER;
		c.gridx = 0; c.gridy = 4;
		c.gridwidth = GridBagConstraints.REMAINDER;
		Insets ic = new Insets(4,4,4,4);
		c.insets = ic;
		pane.add(cancel, c);

		pack();
		setLocationRelativeTo(owner);
	}

	public void addActionListener(ActionListener l) {
		listener = l;
	}

	public void set_thread(Thread in_eeprom_thread) {
		final Thread eeprom_thread = in_eeprom_thread;
		cancel.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					if (eeprom_thread != null) {
						eeprom_thread.interrupt();
					}
				}
			});
	}

	public void start() {
		setVisible(true);
	}

	int max_block = 1;

	public void set_block_internal(int block) {
		double	pos;
		String	s;

		pos = (double) block / (double) max_block;

		s = String.format("block %d of %d", block, max_block);

		pbar.setString(s);
		pbar.setStringPainted(true);
		pbar.setValue((int) (pos * progress_max));
	}

	public void set_max(int max_block) {
		this.max_block = max_block;
	}

	public void set_block(int in_block) {
		final int block = in_block;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_block_internal(block);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void set_serial_internal(int serial) {
		serial_value.setText(String.format("%d", serial));
	}

	public void set_serial(int in_serial) {
		final int serial = in_serial;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_serial_internal(serial);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void set_flight_internal(int flight) {
		flight_value.setText(String.format("%d", flight));
	}

	public void set_flight(int in_flight) {
		final int flight = in_flight;
		Runnable r = new Runnable() {
				public void run() {
					try {
						set_flight_internal(flight);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void done_internal(boolean success) {
		listener.actionPerformed(new ActionEvent(this,
							 success ? 1 : 0,
							 "download"));
		setVisible(false);
		dispose();
	}

	public void done(boolean in_success) {
		final boolean success = in_success;
		Runnable r = new Runnable() {
				public void run() {
					try {
						done_internal(success);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void reset_internal() {
		set_max(1);
		set_block_internal(0);
		set_flight_internal(0);
	}

	public void reset() {
		Runnable r = new Runnable() {
				public void run() {
					try {
						reset_internal();
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}

	private void show_message_internal(String message, String title, int message_type) {
		int joption_message_type = JOptionPane.ERROR_MESSAGE;

		switch (message_type) {
		case INFO_MESSAGE:
			joption_message_type = JOptionPane.INFORMATION_MESSAGE;
			break;
		case WARNING_MESSAGE:
			joption_message_type = JOptionPane.WARNING_MESSAGE;
			break;
		case ERROR_MESSAGE:
			joption_message_type = JOptionPane.ERROR_MESSAGE;
			break;
		}
		JOptionPane.showMessageDialog(owner,
					      message,
					      title,
					      joption_message_type);
	}

	public Boolean check_overwrite(File in_file) {
		final Semaphore check_overwrite_done = new Semaphore(0);
		final File file = in_file;
		final result_holder result = new result_holder();

		Runnable r = new Runnable() {
				public void run() {
					result_holder.result = JOptionPane.showConfirmDialog(owner,
											     String.format("\"%s\" already exists, overwrite?",
													   file.toString()),
											     "Overwrite Existing File?",
											     JOptionPane.YES_NO_OPTION);
					check_overwrite_done.release();
				}
			};

		SwingUtilities.invokeLater(r);
		try {
			check_overwrite_done.acquire();
		} catch (Exception e) {}
		return result_holder.result == JOptionPane.YES_OPTION;
	}

	public void show_message(String in_message, String in_title, int in_message_type) {
		final String message = in_message;
		final String title = in_title;
		final int message_type = in_message_type;
		Runnable r = new Runnable() {
				public void run() {
					try {
						show_message_internal(message, title, message_type);
					} catch (Exception ex) {
					}
				}
			};
		SwingUtilities.invokeLater(r);
	}
}
