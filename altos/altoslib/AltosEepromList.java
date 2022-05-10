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
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

/*
 * Temporary structure to hold the list of stored flights;
 * each of these will be queried in turn to generate more
 * complete information
 */

class AltosEepromFlight {
	int	flight;
	int	start;
	int	end;

	public AltosEepromFlight(int in_flight, int in_start, int in_end) {
		flight = in_flight;
		start = in_start;
		end = in_end;
	}

	public AltosEepromFlight() {
		flight = 0;
		start = 0;
		end = 0;
	}
}

/*
 * Construct a list of flights available in a connected device
 */

public class AltosEepromList extends ArrayList<AltosEepromLog> {
	public AltosConfigData	config_data;

	public AltosEepromList (AltosLink link, boolean remote)
		throws IOException, InterruptedException, TimeoutException
	{
		try {
			if (remote)
				link.start_remote();
			config_data = new AltosConfigData (link);

			/* With the list of flights collected, collect more complete
			 * information on them by reading the first block or two of
			 * data. This will add GPS coordinates and a date. For older
			 * firmware, this will also extract the flight number.
			 */
			if (config_data.flights != null) {
				for (AltosEepromFlight flight : config_data.flights) {
					add(new AltosEepromLog(config_data, link,
							       flight.flight, flight.start, flight.end));
				}
			}
		} finally {
			if (remote)
				link.stop_remote();
			link.flush_output();
		}
	}
}
