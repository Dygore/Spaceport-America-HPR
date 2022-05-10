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

package org.altusmetrum.altoslib_14;

import java.text.*;
import java.io.*;
import java.util.concurrent.*;

public abstract class AltosFlightReader {
	public String name;

	public int serial;

	public void init() {}

	public abstract AltosState read() throws InterruptedException, ParseException, AltosCRCException, IOException;

	public abstract AltosCalData cal_data();

	public abstract void close(boolean interrupted);

	public void set_frequency(double frequency) throws InterruptedException, TimeoutException { }

	public void save_frequency() { }

	public void set_telemetry(int telemetry) { }

	public void set_telemetry_rate(int telemetry_rate) throws InterruptedException, TimeoutException { }

	public void save_telemetry() { }

	public void save_telemetry_rate() { }

	public boolean supports_telemetry(int telemetry) { return false; }

	public boolean supports_telemetry_rate(int telemetry_rate) { return false; }

	public File backing_file() { return null; }

	public boolean has_monitor_battery() throws InterruptedException { return false; }

	public double monitor_battery() throws InterruptedException { return AltosLib.MISSING; }
}
