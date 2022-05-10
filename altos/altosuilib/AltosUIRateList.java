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

public class AltosUIRateList extends JComboBox<String> {

	String	product;
	int	serial;

	public void set_rate(int new_rate) {
		if (new_rate != AltosLib.MISSING)
			setSelectedIndex(new_rate);
		setVisible(new_rate != AltosLib.MISSING);
	}

	public void set_product(String new_product) {
		product = new_product;
	}

	public void set_serial(int new_serial) {
		serial = new_serial;
	}

	public int rate() {
		return getSelectedIndex();
	}

	public AltosUIRateList () {
		super();
		for (int i = 0; i < AltosLib.ao_telemetry_rate_values.length; i++)
			addItem(String.format("%d baud", AltosLib.ao_telemetry_rate_values[i]));
		setMaximumRowCount(getItemCount());
		setEditable(false);
		product = "Unknown";
		serial = 0;
	}

	public AltosUIRateList(int in_rate) {
		this();
		set_rate(in_rate);
	}
}
