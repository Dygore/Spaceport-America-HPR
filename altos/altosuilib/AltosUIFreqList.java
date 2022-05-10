/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

import javax.swing.*;
import org.altusmetrum.altoslib_14.*;

public class AltosUIFreqList extends JComboBox<AltosFrequency> {

	String	product;
	int	serial;
	int	calibrate;

	public void set_frequency(double new_frequency) {
		int i;

		if (new_frequency < 0) {
			setVisible(false);
			return;
		}

		for (i = 0; i < getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) getItemAt(i);

			if (f.close(new_frequency)) {
				setSelectedIndex(i);
				return;
			}
		}
		for (i = 0; i < getItemCount(); i++) {
			AltosFrequency	f = (AltosFrequency) getItemAt(i);

			if (new_frequency < f.frequency)
				break;
		}
		String	description = String.format("%s serial %d", product, serial);
		AltosFrequency	frequency = new AltosFrequency(new_frequency, description);
		AltosUIPreferences.add_common_frequency(frequency);
		insertItemAt(frequency, i);
		setMaximumRowCount(getItemCount());
		setVisible(true);
	}

	public void set_product(String new_product) {
		product = new_product;
	}

	public void set_serial(int new_serial) {
		serial = new_serial;
	}

	public double frequency() {
		AltosFrequency	f = (AltosFrequency) getSelectedItem();
		if (f != null)
			return f.frequency;
		return 434.550;
	}

	public AltosUIFreqList () {
		super(AltosUIPreferences.common_frequencies());
		setMaximumRowCount(getItemCount());
		setEditable(false);
		product = "Unknown";
		serial = 0;
	}

	public AltosUIFreqList(double in_frequency) {
		this();
		set_frequency(in_frequency);
	}
}
