/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

package org.altusmetrum.altosuilib_14;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import javax.swing.*;
import javax.swing.event.*;
import org.altusmetrum.altoslib_14.*;

public class AltosUIAccelCal
	extends AltosUIDialog
	implements AltosAccelCalListener, ActionListener
{
	Frame owner;
	AltosLink link;
	AltosAccelCal cal;
	AltosConfigValues	config_values;
	Thread thread;
	Container pane;
	JTextField message;
	JButton	antenna_up;
	JButton	antenna_down;
	JButton	ok;
	JButton cancel;
	boolean success;
	int accel_plus, accel_minus;

	private void make_visible() {
		pack();
		cal.start();
		setVisible(true);
	}

	public boolean doit() {
		success = false;
		make_visible();
		return success;
	}

	public int accel_cal_plus() {
		if (success)
			return accel_plus;
		return AltosLib.MISSING;
	}

	public int accel_cal_minus() {
		if (success)
			return accel_minus;
		return AltosLib.MISSING;
	}

	private void setDefaultButton(JButton button) {
		this.getRootPane().setDefaultButton(button);
	}

	/* AltosAccelCalListener interface */
	public void set_thread(AltosAccelCal cal, Thread thread) {
		this.thread = thread;
	}

	public void set_phase(AltosAccelCal cal, final int phase) {
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					switch (phase) {
					case AltosAccelCal.phase_antenna_up:
						message.setText("Orient antenna upwards and click on Antenna Up");
						antenna_up.setEnabled(true);
						setDefaultButton(antenna_up);
						antenna_down.setEnabled(false);
						ok.setEnabled(false);
						break;
					case AltosAccelCal.phase_antenna_down:
						message.setText("Orient antenna downwards and click on Antenna Down");
						antenna_up.setEnabled(false);
						antenna_down.setEnabled(true);
						setDefaultButton(antenna_down);
						ok.setEnabled(false);
						break;
					}
				}
			});
	}

	public void cal_done(AltosAccelCal cal, int plus, int minus) {
		accel_plus = plus;
		accel_minus = minus;
		success = true;
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					message.setText(String.format("Calibration succeeded, plus %d minus %d, press OK to continue", accel_plus, accel_minus));
					antenna_up.setEnabled(false);
					antenna_down.setEnabled(false);
					ok.setEnabled(true);
					setDefaultButton(ok);
				}
			});
	}

	public void message(AltosAccelCal cal, final String msg) {
		SwingUtilities.invokeLater(new Runnable() {
				public void run() {
					message.setText(msg);
				}
			});
	}

	public void error(AltosAccelCal cal, String msg) {
		message(cal, msg);
	}

	/* ActionListener interface */
	public void actionPerformed(ActionEvent e) {
		String	cmd = e.getActionCommand();

		if ("up".equals(cmd)) {
			cal.signal(true);
			antenna_up.setEnabled(false);
		} else if ("down".equals(cmd)) {
			cal.signal(true);
			antenna_down.setEnabled(false);
			this.setDefaultButton(antenna_down);
		} else if ("ok".equals(cmd)) {
			cal.signal(true);
			this.setVisible(false);
			if (success) {
				config_values.set_accel_cal(accel_plus, accel_minus);
				config_values.set_dirty();
			}
			try {
				cal.abort();
			} catch (InterruptedException ie) {
			}
		} else if ("cancel".equals(cmd)) {
			cal.signal(false);
			this.setVisible(false);
			try {
				cal.abort();
			} catch (InterruptedException ie) {
			}
		}
	}
	public AltosUIAccelCal(Frame owner, AltosLink link, AltosConfigValues config_values) {
		super(owner, "Calibrate Accelerometer", true);

		this.owner = owner;
		this.link = link;
		this.config_values = config_values;

		pane = getScrollablePane();
		pane.setLayout(new GridBagLayout());

		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(4,4,4,4);

		int x = 0;
		int y = 0;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridy = y;
		c.gridwidth = 4;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		message = new JTextField(64);
		pane.add(message, c);

		y++; x = 0;

		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridy = y;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		antenna_up = new JButton("Antenna Up");
		antenna_up.setActionCommand("up");
		antenna_up.setEnabled(false);
		antenna_up.addActionListener(this);
		pane.add(antenna_up, c);

		x++;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridy = y;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		antenna_down = new JButton("Antenna Down");
		antenna_down.setActionCommand("down");
		antenna_down.setEnabled(false);
		antenna_down.addActionListener(this);
		pane.add(antenna_down, c);

		x++;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridy = y;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		ok = new JButton("OK");
		ok.setActionCommand("ok");
		ok.setEnabled(false);
		ok.addActionListener(this);
		pane.add(ok, c);

		x++;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.WEST;
		c.gridx = x;
		c.gridy = y;
		c.gridwidth = 1;
		c.gridheight = 1;
		c.weightx = 0;
		c.weighty = 0;
		cancel = new JButton("Cancel");
		cancel.setActionCommand("cancel");
		cancel.setEnabled(true);
		cancel.addActionListener(this);
		pane.add(cancel, c);

		cal = new AltosAccelCal(this.link, this);
	}
}
