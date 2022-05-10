/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

package org.altusmetrum.micropeak;

import java.lang.*;
import java.io.*;
import java.util.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class MicroData {
	public int		ground_pressure;
	public int		min_pressure;

	AltosUIFlightSeries	flight_series;
	AltosFlightStats	flight_stats;
	AltosCalData		cal_data;

	private double		time_step;
	private ArrayList<Integer>	bytes;
	public int		nsamples;
	public int		log_id;
	String			name;
	String			unique_id;

	public static final int LOG_ID_MICROPEAK = 0;
	public static final int LOG_ID_MICROKITE = 1;
	public static final int LOG_ID_MICROPEAK2 = 2;

	public static final double CLOCK_MP1 = 0.096;
	public static final double CLOCK_MP2 = 0.1;

	public class FileEndedException extends Exception {
	}

	public class NonHexcharException extends Exception {
	}

	public class InvalidCrcException extends Exception {
	}

	private int getc(InputStream f) throws IOException, FileEndedException {
		int	c = f.read();

		if (c == -1)
			throw new FileEndedException();
		bytes.add(c);
		return c;
	}

	private int get_nonwhite(InputStream f) throws IOException, FileEndedException {
		int	c;

		for (;;) {
			c = getc(f);
			if (!Character.isWhitespace(c))
				return c;
		}
	}

	private int get_hexc(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	c = get_nonwhite(f);

		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		throw new NonHexcharException();
	}

	private static final int POLY = 0x8408;

	private int log_crc(int crc, int b) {
		int	i;

		for (i = 0; i < 8; i++) {
			if (((crc & 0x0001) ^ (b & 0x0001)) != 0)
				crc = (crc >> 1) ^ POLY;
			else
				crc = crc >> 1;
			b >>= 1;
		}
		return crc & 0xffff;
	}

	int	file_crc;

	private int get_hex(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	a = get_hexc(f);
		int	b = get_hexc(f);

		int h = (a << 4) + b;

		file_crc = log_crc(file_crc, h);
		return h;
	}

	private boolean find_header(InputStream f) throws IOException, FileEndedException {
		for (;;) {
			if (get_nonwhite(f) == 'M' && get_nonwhite(f) == 'P')
				return true;
		}
	}

	private int get_32(InputStream f)  throws IOException, FileEndedException, NonHexcharException {
		int	v = 0;
		for (int i = 0; i < 4; i++) {
			v += get_hex(f) << (i * 8);
		}
		return v;
	}

	private int get_16(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int	v = 0;
		for (int i = 0; i < 2; i++) {
			v += get_hex(f) << (i * 8);
		}
		return v;
	}

	private String get_line(InputStream f) throws IOException, FileEndedException, NonHexcharException {
		int c;
		StringBuffer line = new StringBuffer();

		do {
			c = f.read();
		} while (Character.isWhitespace(c));

		do {
			line.append((char) c);
			c = f.read();
		} while (!Character.isWhitespace(c));
		return new String(line);
	}

	private int swap16(int i) {
		return ((i << 8) & 0xff00) | ((i >> 8) & 0xff);
	}

	public boolean	crc_valid;

	public boolean	log_empty;

	int mix_in (int high, int low) {
		return  high - (high & 0xffff) + low;
	}

	boolean closer (int target, int a, int b) {
		return Math.abs (target - a) < Math.abs(target - b);
	}

	public double altitude(double time) {
		if (flight_series.altitude_series == null)
			return 0.0;
		return flight_series.altitude_series.value(time);
	}

	public double altitude(int i) {
		return altitude(time(i));
	}

	public String name() {
		return name;
	}

	public double pressure(int i) {
		if (flight_series.pressure_series == null)
			return 0.0;

		return flight_series.pressure_series.value(time(i));
	}

	public double height(double time) {
		if (flight_series.height_series == null)
			return 0.0;

		return flight_series.height_series.value(time);
	}

	public double height(int i) {
		return height(time(i));
	}

	public int length() {
		if (flight_series.pressure_series == null)
			return 0;
		return flight_series.pressure_series.size();
	}

	/* Use the recorded apogee pressure for stats so that it agrees with the device */
	public double apogee_pressure() {
		return min_pressure;
	}

	public double apogee_altitude() {
		return AltosConvert.pressure_to_altitude(apogee_pressure());
	}

	public double apogee_height() {
		return apogee_altitude() - cal_data.ground_altitude;
	}

	public double speed(double time) {
		if (flight_series.speed_series == null)
			return 0.0;
		return flight_series.speed_series.value(time);
	}

	public double speed(int i) {
		return speed(time(i));
	}

	public double acceleration(double time) {
		if (flight_series.accel_series == null)
			return 0.0;
		return flight_series.accel_series.value(time);
	}

	public double acceleration(int i) {
		return acceleration(time(i));
	}

	public double time(int i) {
		return i * time_step;
	}

	public void save (OutputStream f) throws IOException {
		for (int c : bytes)
			f.write(c);
		f.write('\n');
	}

	public boolean is_empty() {
		boolean empty = true;
		for (int c : bytes) {
			if (!Character.isWhitespace(c) && c != 'f')
				empty = false;
		}
		return empty;
	}

	public void export (Writer f) throws IOException {
		PrintWriter	pw = new PrintWriter(f);
		pw.printf("  Time, Press(Pa), Height(m), Height(f), Speed(m/s), Speed(mph), Speed(mach), Accel(m/s²), Accel(ft/s²),  Accel(g)\n");

		for (AltosTimeValue ptv : flight_series.pressure_series) {

			double height = height(ptv.time);
			double speed = speed(ptv.time);
			double accel = acceleration(ptv.time);

			pw.printf("%6.3f,%10.0f,%10.1f,%10.1f,%11.2f,%11.2f,%12.4f,%12.2f,%13.2f,%10.4f\n",
				  ptv.time,
				  ptv.value,
				  height,
				  AltosConvert.meters_to_feet(height),
				  speed,
				  AltosConvert.meters_to_mph(speed),
				  AltosConvert.meters_to_mach(speed),
				  accel,
				  AltosConvert.meters_to_feet(accel),
				  AltosConvert.meters_to_g(accel));
		}
	}

	public void set_name(String name) {
		this.name = name;
	}

	public MicroData() {
		ground_pressure = 101000;
		min_pressure = 101000;
		cal_data = new AltosCalData();
		flight_series = new AltosUIFlightSeries(cal_data);
	}

	public MicroData (InputStream f, String name) throws IOException, InterruptedException, NonHexcharException, FileEndedException {
		this.name = name;
		bytes = new ArrayList<Integer>();

		cal_data = new AltosCalData();
		flight_series = new AltosUIFlightSeries(cal_data);

		if (!find_header(f))
			throw new IOException("No MicroPeak data header found");
		try {
			file_crc = 0xffff;
			ground_pressure = get_32(f);
			min_pressure = get_32(f);
			nsamples = get_16(f);

			log_id = nsamples >> 12;
			nsamples &= 0xfff;
			if (log_id == LOG_ID_MICROPEAK2) {
				int nsamples_high = get_16(f);
				nsamples |= (nsamples_high << 12);
			}

			cal_data.set_ground_pressure(ground_pressure);

			switch (log_id) {
			case LOG_ID_MICROPEAK:
				time_step = 2 * CLOCK_MP1;
				break;
			case LOG_ID_MICROKITE:
				time_step = 200 * CLOCK_MP1;
				break;
			case LOG_ID_MICROPEAK2:
				time_step = CLOCK_MP2;
				break;
			default:
				throw new IOException(String.format("Unknown device type: %d", log_id));
			}
			cal_data.set_ticks_per_sec(1/time_step);
			cal_data.set_tick(0);
			cal_data.set_boost_tick();

			int cur = ground_pressure;
			cal_data.set_tick(0);
			flight_series.set_time(cal_data.time());
			flight_series.set_pressure(cur);
			for (int i = 0; i < nsamples; i++) {
				int	k = get_16(f);
				int	same = mix_in(cur, k);
				int	up = mix_in(cur + 0x10000, k);
				int	down = mix_in(cur - 0x10000, k);

				if (closer (cur, same, up)) {
					if (closer (cur, same, down))
						cur = same;
					else
						cur = down;
				} else {
					if (closer (cur, up, down))
						cur = up;
					else
						cur = down;
				}

				cal_data.set_tick(i+1);
				flight_series.set_time(cal_data.time());
				flight_series.set_pressure(cur);
			}

			int current_crc = swap16(~file_crc & 0xffff);
			int crc = get_16(f);

			crc_valid = (crc == current_crc);

			if (!crc_valid && is_empty()) {
				crc_valid = true;
				nsamples = 0;
			}

			if (log_id == LOG_ID_MICROPEAK2) {
				unique_id = get_line(f);
			}

			flight_series.finish();

			/* Build states */

			flight_series.set_time(0);
			flight_series.set_state(AltosLib.ao_flight_boost);

			if (flight_series.speed_series != null && flight_series.speed_series.max() != null) {
				flight_series.set_time(flight_series.speed_series.max().time);
				flight_series.set_state(AltosLib.ao_flight_coast);
			}

			flight_series.set_time(flight_series.height_series.max().time);
			flight_series.set_state(AltosLib.ao_flight_drogue);

			cal_data.set_tick(nsamples);
			flight_series.set_time(cal_data.time());
			flight_series.set_state(AltosLib.ao_flight_landed);

			flight_series.set_min_pressure(min_pressure);

			flight_series.finish();

			flight_stats = new AltosFlightStats(flight_series);


		} catch (FileEndedException fe) {
			throw new IOException("File Ended Unexpectedly");
		}
	}

}
