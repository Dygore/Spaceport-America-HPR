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

package altosui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class AltosIgnitor extends AltosUIFlightTab {

	public class Ignitor extends AltosUIUnitsIndicator {
		int		igniter;

		public double value(AltosState state, int i) {
			if (state.igniter_voltage == null ||
			    state.igniter_voltage.length < igniter)
				return AltosLib.MISSING;
			return state.igniter_voltage[igniter];
		}

		public double good() { return AltosLib.ao_igniter_good; }

		public Ignitor (AltosUIFlightTab container, int y) {
			super(container, y, AltosConvert.voltage, String.format ("%s Voltage", AltosLib.igniter_name(y)), 1, true, 1);
			igniter = y;
		}
	}

	Ignitor[] igniters;

	public void show(AltosState state, AltosListenerState listener_state) {
		if (isShowing())
			make_igniters(state);
		super.show(state, listener_state);
	}

	public boolean should_show(AltosState state) {
		if (state == null)
			return false;
		if (state.igniter_voltage == null)
			return false;
		return state.igniter_voltage.length > 0;
	}

	void make_igniters(AltosState state) {
		int n = (state == null || state.igniter_voltage == null) ? 0 : state.igniter_voltage.length;
		int old_n = igniters == null ? 0 : igniters.length;

		if (n != old_n) {

			if (igniters != null) {
				for (int i = 0; i < igniters.length; i++) {
					remove(igniters[i]);
					igniters[i].remove(this);
					igniters = null;
				}
			}

			if (n > 0) {
				setVisible(true);
				igniters = new Ignitor[n];
				for (int i = 0; i < n; i++) {
					igniters[i] = new Ignitor(this, i);
					add(igniters[i]);
				}
			} else
				setVisible(false);
		}
	}

	public String getName() {
		return "Ignitors";
	}
}
