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

import java.io.*;
import java.util.*;
import java.text.*;
import java.util.concurrent.*;

class AltosEepromNameData extends AltosDataListener {
	AltosGPS	gps = null;

	boolean avoid_duplicate_files = false;

	public void set_rssi(int rssi, int status) { }
	public void set_received_time(long received_time) { }

	public void set_acceleration(double accel) { }
	public void set_pressure(double pa) { }
	public void set_thrust(double N) { }

	public void set_temperature(double deg_c) { }
	public void set_battery_voltage(double volts) { }

	public void set_apogee_voltage(double volts) { }
	public void set_main_voltage(double volts) { }

	public void set_avoid_duplicate_files() {
		avoid_duplicate_files = true;
	}

	public void set_gps(AltosGPS gps, boolean set_location, boolean set_sats) {
		super.set_gps(gps, set_location, set_sats);
		if (gps != null &&
		    gps.year != AltosLib.MISSING &&
		    gps.month != AltosLib.MISSING &&
		    gps.day != AltosLib.MISSING) {
			this.gps = gps;
		}
	}

	public boolean done() {
		if (gps == null)
			return false;
		return true;
	}

	public void set_gyro(double roll, double pitch, double yaw) { }
	public void set_accel_ground(double along, double across, double through) { }
	public void set_accel(double along, double across, double through) { }
	public void set_mag(double along, double across, double through) { }
	public void set_pyro_voltage(double volts) { }
	public void set_igniter_voltage(double[] voltage) { }
	public void set_pyro_fired(int pyro_mask) { }
	public void set_companion(AltosCompanion companion) { }
	public void set_kalman(double height, double speed, double acceleration) { }
	public void set_orient(double new_orient) { }
	public void set_motor_pressure(double motor_pressure) { }

	public AltosEepromNameData(AltosCalData cal_data) {
		super(cal_data);
	}
}

public class AltosEepromDownload implements Runnable {

	AltosLink		link;
	boolean			remote;
	Thread			eeprom_thread;
	AltosEepromMonitor	monitor;

	AltosEepromList		flights;
	String			parse_errors;

	private boolean has_gps_date(AltosState state) {
		if (state == null)
			return false;

		AltosGPS gps = state.gps;

		return gps != null &&
			gps.year != AltosLib.MISSING &&
			gps.month != AltosLib.MISSING &&
			gps.day != AltosLib.MISSING;
	}

	private AltosFile MakeFile(int serial, int flight, AltosEepromNameData name_data) throws IOException {
		AltosFile		eeprom_name;

		for (;;) {
			if (name_data.gps != null) {
				AltosGPS		gps = name_data.gps;
				eeprom_name = new AltosFile(gps.year, gps.month, gps.day,
							    serial, flight, "eeprom");
			} else
				eeprom_name = new AltosFile(serial, flight, "eeprom");
			if (!name_data.avoid_duplicate_files)
				break;
			if (!eeprom_name.exists())
				break;
			flight++;
		}

		return eeprom_name;
	}

	boolean			done;
	int			prev_state;
	int			state_block;

	void LogError(String error) {
		if (parse_errors != null)
			parse_errors.concat(error.concat("\n"));
		else
			parse_errors = error;
	}

	class BlockCache extends Hashtable<Integer,AltosEepromChunk> {
		AltosEepromLog	log;

		AltosEepromChunk get(int start, boolean add) throws TimeoutException, InterruptedException {
			if (contains(start))
				return super.get(start);
			AltosEepromChunk eechunk = new AltosEepromChunk(link, start, start == log.start_block);
			if (add)
				put(start, eechunk);
			return eechunk;
		}

		public BlockCache(AltosEepromLog log) {
			this.log = log;
		}
	}

	int FindLastLog(AltosEepromLog log, BlockCache cache) throws TimeoutException, InterruptedException {
		int	low = log.start_block;
		int	high = log.end_block - 1;

		while (low <= high) {
			int mid = (high + low) / 2;

			if (!cache.get(mid, true).erased())
				low = mid + 1;
			else
				high = mid - 1;
		}
		return low;
	}

	void CaptureLog(AltosEepromLog log) throws IOException, InterruptedException, TimeoutException, ParseException {
		int			block, state_block = 0;
		int			log_format = flights.config_data.log_format;
		BlockCache		cache = new BlockCache(log);

		done = false;

		if (flights.config_data.serial < 0)
			throw new IOException("no serial number found");

		/* Set serial number in the monitor dialog window */
		monitor.set_serial(log.serial);
		monitor.set_flight(log.flight);

		int	start_block = log.start_block;
		int	end_block = FindLastLog(log, cache);

		monitor.set_max(end_block - start_block - 1);

		ArrayList<Byte> data = new ArrayList<Byte>();

		/* Now scan the eeprom, reading blocks of data to create a byte array of data */

		for (block = start_block; block < end_block; block++) {
			monitor.set_block(block - start_block);

			AltosEepromChunk	eechunk = cache.get(block, false);

			for (int i = 0; i < eechunk.data.length; i++)
				data.add((byte) eechunk.data[i]);
		}

		/* Construct our internal representation of the eeprom data */
		AltosEeprom	eeprom = new AltosEeprom(flights.config_data, data);

		/* Now see if we can't actually parse the resulting
		 * file to generate a better filename. Note that this
		 * doesn't need to work; we'll still save the data using
		 * a less accurate name.
		 */
		AltosEepromRecordSet		set = new AltosEepromRecordSet(eeprom);
		AltosEepromNameData name_data = new AltosEepromNameData(set.cal_data());

		for (AltosEepromRecord record : set.ordered) {
			record.provide_data(name_data, set.cal_data());
			if (name_data.done())
				break;
		}

		AltosFile f = MakeFile(flights.config_data.serial, log.flight, name_data);

		log.set_file(f);

		boolean do_write = true;

		if (f.exists())
			do_write = monitor.check_overwrite(f);

		if (do_write) {
			FileWriter w = new FileWriter(f);

			eeprom.write(w);
			w.close();
		}

		if (eeprom.errors != 0)
			throw new ParseException(String.format("%d CRC Errors", eeprom.errors), 0);
	}

	static String label(int flight) {
		if (flight < 0)
			return "Corrupt";
		else
			return "Flight";
	}

	static int flight(int flight) {
		if (flight < 0)
			return -flight;
		return flight;
	}

	public void run () {
		boolean success = false;

		try {
			if (remote)
				link.start_remote();

			for (AltosEepromLog log : flights) {
				parse_errors = null;
				if (log.download_selected) {
					monitor.reset();
					try {
						CaptureLog(log);
					} catch (ParseException e) {
						LogError(e.getMessage());
					}
				}
				success = true;
				if (parse_errors != null) {
					monitor.show_message(String.format("%s %d download error. Valid log data saved\n%s",
									   label(log.flight),
									   flight(log.flight),
									   parse_errors),
							     link.name,
							     AltosEepromMonitor.WARNING_MESSAGE);
				}
			}
		} catch (IOException ee) {
			monitor.show_message(ee.getLocalizedMessage(),
					     link.name,
					     AltosEepromMonitor.ERROR_MESSAGE);
		} catch (InterruptedException ie) {
			monitor.show_message(String.format("Connection to \"%s\" interrupted",
							   link.name),
					     "Connection Interrupted",
					     AltosEepromMonitor.ERROR_MESSAGE);
		} catch (TimeoutException te) {
			monitor.show_message(String.format("Connection to \"%s\" failed",
							   link.name),
					     "Connection Failed",
					     AltosEepromMonitor.ERROR_MESSAGE);
		} finally {
			if (remote) {
				try {
					link.stop_remote();
				} catch (InterruptedException ie) {
				}
			}
			link.flush_output();
		}
		monitor.done(success);
	}

	public void start() {
		eeprom_thread = new Thread(this);
		monitor.set_thread(eeprom_thread);
		eeprom_thread.start();
	}

	public AltosEepromDownload(AltosEepromMonitor given_monitor,
				   AltosLink given_link,
				   boolean given_remote,
				   AltosEepromList given_flights) {

		monitor = given_monitor;
		link = given_link;
		remote = given_remote;
		flights = given_flights;

		monitor.start();
	}
}
