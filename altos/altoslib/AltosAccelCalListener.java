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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.concurrent.*;

public interface AltosAccelCalListener {
	public void set_thread(AltosAccelCal cal, Thread thread);

	public void set_phase(AltosAccelCal cal, int phase);

	public void cal_done(AltosAccelCal cal, int plus, int minus);

	public void error(AltosAccelCal cal, String msg);

	public void message(AltosAccelCal cal, String msg);
}
