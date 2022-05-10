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

package org.altusmetrum.altosuilib_14;

import java.util.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;


public class AltosUITelemetryList extends JComboBox<String> {
	public int get_selected() {
		return getSelectedIndex() + 1;
	}

	public void set_selected(int telemetry) {
		setSelectedIndex(telemetry-1);
	}

	public AltosUITelemetryList(int serial) {
		super();
		for (int i = AltosLib.ao_telemetry_min; i <= AltosLib.ao_telemetry_max; i++)
			addItem(AltosLib.telemetry_name(i));

		int telemetry = AltosPreferences.telemetry(serial);
		if (telemetry < AltosLib.ao_telemetry_min || AltosLib.ao_telemetry_max < telemetry)
			telemetry = AltosLib.ao_telemetry_standard;
		setMaximumRowCount(AltosLib.ao_telemetry_max);
		set_selected(telemetry);
		revalidate();
	}
}

