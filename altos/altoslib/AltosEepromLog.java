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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.text.*;
import java.util.concurrent.*;

/*
 * Extract a bit of information from an eeprom-stored flight log.
 */

public class AltosEepromLog {
	public int		serial;
	public boolean		has_flight;
	public int		flight;
	public int		start_block;
	public int		end_block;

	public boolean		download_selected;
	public boolean		delete_selected;
	public boolean		graph_selected;

	public File		file;

	public void set_file(File file) {
		this.file = file;
	}

	public AltosEepromLog(AltosConfigData config_data,
			      AltosLink link,
			      int in_flight, int in_start_block,
			      int in_end_block)
		throws InterruptedException, TimeoutException {

		int		block;

		flight = in_flight;
		if (flight != 0)
			has_flight = true;
		start_block = in_start_block;
		end_block = in_end_block;
		serial = config_data.serial;

		/*
		 * Select all flights for download and graph, but not
		 * for delete
		 */
		download_selected = true;
		delete_selected = false;
		graph_selected = true;
	}
}
