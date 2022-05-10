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

import java.util.*;
import java.text.*;
import java.util.concurrent.*;

/* Don't change the field names in this structure; they're part of all .eeprom files */
public class AltosConfigData {

	/* Version information */
	public String	manufacturer;
	public String	product;
	public int	serial;
	public int	flight;
	public int	log_format;
	public int	log_space;
	public String	version;
	public int	altitude_32;
	public int	config_major, config_minor;

	/* Config information */
	/* HAS_FLIGHT*/
	public int	main_deploy;
	public int	apogee_delay;
	public int	apogee_lockout;

	/* HAS_RADIO */
	public int	radio_frequency;
	public String	callsign;
	public int	radio_enable;
	public int	radio_calibration;
	public int	telemetry_rate;
	/* Old HAS_RADIO values */
	public int	radio_channel;
	public int	radio_setting;

	/* HAS_ACCEL */
	public int	accel_cal_plus, accel_cal_minus;
	private int	accel_cal_plus_cooked, accel_cal_minus_cooked;
	private boolean	accel_cal_adjusted;
	public int	pad_orientation;

	/* HAS_LOG */
	public int	flight_log_max;
	public int 	log_fixed;

	/* HAS_IGNITE */
	public int	ignite_mode;

	/* HAS_AES */
	public String	aes_key;

	/* AO_PYRO_NUM */
	public AltosPyro[]	pyros;
	public int		npyro;
	public int		pyro;
	public double		pyro_firing_time;

	/* HAS_APRS */
	public int		aprs_interval;
	public int		aprs_ssid;
	public int		aprs_format;
	public int		aprs_offset;

	/* HAS_BEEP */
	public int		beep;

	/* HAS_RADIO_10MW */
	public int		radio_10mw;

	/* Storage info replies */
	public int	storage_size;
	public int	storage_erase_unit;

	/* Log listing replies */
	public int	stored_flight;
	public AltosEepromFlight[] flights;

	/* HAS_TRACKER */
	public int	tracker_motion;
	public int	tracker_interval;

	/* HAS_GYRO */
	public int	accel_zero_along, accel_zero_across, accel_zero_through;

	/* ms5607 data */
	AltosMs5607	ms5607;

	public AltosMs5607 ms5607() {
		if (ms5607 == null)
			ms5607 = new AltosMs5607();
		return ms5607;
	}

	public static String get_string(String line, String label) throws  ParseException {
		if (line.startsWith(label)) {
			String	quoted = line.substring(label.length()).trim();

			if (quoted.startsWith("\""))
				quoted = quoted.substring(1);
			if (quoted.endsWith("\""))
				quoted = quoted.substring(0,quoted.length()-1);
			return quoted;
		}
		throw new ParseException("mismatch", 0);
	}

	public static int get_int(String line, String label) throws NumberFormatException, ParseException {
		if (line.startsWith(label)) {
			String tail = line.substring(label.length()).trim();
			String[] tokens = tail.split("\\s+");
			if (tokens.length > 0)
				return  Integer.parseInt(tokens[0]);
		}
		throw new ParseException("mismatch", 0);
	}

	public static int[] get_values(String line, String label) throws NumberFormatException, ParseException {
		if (line.startsWith(label)) {
			String tail = line.substring(label.length()).trim();
			String[] tokens = tail.split("\\s+");
			if (tokens.length > 1) {
				int[]	values = new int[2];
				values[0] = Integer.parseInt(tokens[0]);
				values[1] = Integer.parseInt(tokens[1]);
				return values;
			}
		}
		throw new ParseException("mismatch", 0);
	}

	public int log_space() {
		if (log_space != AltosLib.MISSING)
			return log_space;

		if (storage_size != AltosLib.MISSING) {
			int	space = storage_size;

			if (storage_erase_unit != AltosLib.MISSING && use_flash_for_config())
				space -= storage_erase_unit;

			if (space != AltosLib.MISSING)
				return space;
		}
		return 0;
	}

	public int log_available() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TINY:
			if (flights == null)
				return 1;
			return 0;
		case AltosLib.AO_LOG_FORMAT_TELEMETRY:
		case AltosLib.AO_LOG_FORMAT_TELESCIENCE:
			return 1;
		default:
			if (flight_log_max <= 0)
				return 1;
			int	log_max = flight_log_max * 1024;
			int	log_space = log_space();
			int	log_used;

			if (flights == null)
				log_used = 0;
			else
				log_used = flights.length * log_max;
			int	log_avail;

			if (log_used >= log_space)
				log_avail = 0;
			else
				log_avail = (log_space - log_used) / log_max;

			return log_avail;
		}
	}

	public int invert_accel_value(int value) {
		if (value == AltosLib.MISSING)
			return AltosLib.MISSING;

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_FULL:
			return 0x7fff - value;
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_OLD:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA:
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_3:
			return 4095 - value;
		case AltosLib.AO_LOG_FORMAT_TELEMETRUM:
			/*
			 * TeleMetrum v2 and later use the same log format, but
			 * have different accelerometers. This is the only place
			 * it matters in altoslib.
			 */
			if (product.startsWith("TeleMetrum-v2"))
				return 4095 - value;
			/* fall through */
		case AltosLib.AO_LOG_FORMAT_TELEMEGA_4:
		case AltosLib.AO_LOG_FORMAT_EASYMEGA_2:
		case AltosLib.AO_LOG_FORMAT_EASYMOTOR:
			return -value;
		default:
			if (product.startsWith("EasyTimer-"))
				return -value;
			return AltosLib.MISSING;
		}
	}

	public boolean has_monitor_battery() {
		if (product == null)
			return false;
		if (product.startsWith("TeleBT"))
			return true;
		return false;
	}

	int[] parse_version(String v) {
		String[] parts = v.split("\\.");
		int r[] = new int[parts.length];

		for (int i = 0; i < parts.length; i++) {
			try {
				r[i] = (int) AltosLib.fromdec(parts[i]);
			} catch (NumberFormatException n) {
				r[i] = 0;
			}
		}

		return r;
	}

	public boolean altitude_32() {
		return altitude_32 == 1;
	}

	public int compare_version(String other) {
		int[]	me = parse_version(version);
		int[]	them = parse_version(other);

		int	l = Math.min(me.length, them.length);

		for (int i = 0; i < l; i++) {
			int	d = me[i] - them[i];
			if (d != 0)
				return d;
		}
		if (me.length > l)
			return 1;
		if (them.length > l)
			return -1;
		return 0;
	}

	public void reset() {
		manufacturer = null;
		product = null;
		serial = AltosLib.MISSING;
		flight = AltosLib.MISSING;
		log_format = AltosLib.AO_LOG_FORMAT_UNKNOWN;
		log_space = AltosLib.MISSING;
		version = "unknown";
		config_major = AltosLib.MISSING;
		config_minor = AltosLib.MISSING;

		main_deploy = AltosLib.MISSING;
		apogee_delay = AltosLib.MISSING;
		apogee_lockout = AltosLib.MISSING;

		radio_frequency = AltosLib.MISSING;
		callsign = null;
		radio_enable = AltosLib.MISSING;
		radio_calibration = AltosLib.MISSING;
		radio_channel = AltosLib.MISSING;
		radio_setting = AltosLib.MISSING;
		telemetry_rate = AltosLib.MISSING;

		accel_cal_plus_cooked = AltosLib.MISSING;
		accel_cal_minus_cooked = AltosLib.MISSING;
		accel_cal_plus = AltosLib.MISSING;
		accel_cal_minus = AltosLib.MISSING;
		pad_orientation = AltosLib.MISSING;
		accel_cal_adjusted = false;

		flight_log_max = AltosLib.MISSING;
		log_fixed = AltosLib.MISSING;
		ignite_mode = AltosLib.MISSING;

		aes_key = null;

		pyro = AltosLib.MISSING;
		npyro = AltosLib.MISSING;
		pyros = null;
		pyro_firing_time = AltosLib.MISSING;

		aprs_interval = AltosLib.MISSING;
		aprs_ssid = AltosLib.MISSING;
		aprs_format = AltosLib.MISSING;
		aprs_offset = AltosLib.MISSING;

		beep = AltosLib.MISSING;

		radio_10mw = AltosLib.MISSING;

		tracker_motion = AltosLib.MISSING;
		tracker_interval = AltosLib.MISSING;

		storage_size = AltosLib.MISSING;
		storage_erase_unit = AltosLib.MISSING;
		stored_flight = 0;
		flights = null;

		accel_zero_along = AltosLib.MISSING;
		accel_zero_across = AltosLib.MISSING;
		accel_zero_through = AltosLib.MISSING;
	}

	/* Return + accel calibration relative to a specific pad orientation */
	public int accel_cal_plus(int pad_orientation) {
		adjust_accel_cal();
		if (!accel_cal_adjusted)
			return AltosLib.MISSING;

		switch (pad_orientation) {
		case AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP:
		case AltosLib.AO_PAD_ORIENTATION_WORDS_UPRIGHT:
		case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_UP:
			return accel_cal_plus_cooked;
		case AltosLib.AO_PAD_ORIENTATION_ANTENNA_DOWN:
		case AltosLib.AO_PAD_ORIENTATION_WORDS_UPSIDEDOWN:
		case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_DOWN:
			return invert_accel_value(accel_cal_minus_cooked);
		default:
			return AltosLib.MISSING;
		}
	}

	/* Return - accel calibration relative to a specific pad orientation */
	public int accel_cal_minus(int pad_orientation) {
		adjust_accel_cal();
		if (!accel_cal_adjusted)
			return AltosLib.MISSING;

		switch (pad_orientation) {
		case AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP:
		case AltosLib.AO_PAD_ORIENTATION_WORDS_UPRIGHT:
		case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_UP:
			return accel_cal_minus_cooked;
		case AltosLib.AO_PAD_ORIENTATION_ANTENNA_DOWN:
		case AltosLib.AO_PAD_ORIENTATION_WORDS_UPSIDEDOWN:
		case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_DOWN:
			return invert_accel_value(accel_cal_plus_cooked);
		default:
			return AltosLib.MISSING;
		}
	}

	/* Once we have all of the values from the config data, compute the
	 * accel cal values relative to Antenna Up orientation.
	 */
	private void adjust_accel_cal() {
		if (!accel_cal_adjusted &&
		    product != null &&
		    pad_orientation != AltosLib.MISSING &&
		    accel_cal_plus != AltosLib.MISSING &&
		    accel_cal_minus != AltosLib.MISSING)
		{
			switch (pad_orientation) {
			case AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP:
			case AltosLib.AO_PAD_ORIENTATION_WORDS_UPRIGHT:
			case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_UP:
				accel_cal_plus_cooked = accel_cal_plus;
				accel_cal_minus_cooked = accel_cal_minus;
				accel_cal_adjusted = true;
				break;
			case AltosLib.AO_PAD_ORIENTATION_ANTENNA_DOWN:
			case AltosLib.AO_PAD_ORIENTATION_WORDS_UPSIDEDOWN:
			case AltosLib.AO_PAD_ORIENTATION_BIG_PARTS_DOWN:
				accel_cal_plus_cooked = invert_accel_value(accel_cal_minus);
				accel_cal_minus_cooked = invert_accel_value(accel_cal_plus);
				accel_cal_adjusted = true;
				break;
			default:
				break;
			}
		}
	}

	public void parse_line(String line) {

		/* Version replies */
		try { manufacturer = get_string(line, "manufacturer"); } catch (Exception e) {}
		try { product = get_string(line, "product"); } catch (Exception e) {}
		try { serial = get_int(line, "serial-number"); } catch (Exception e) {}
		try { flight = get_int(line, "current-flight"); } catch (Exception e) {}
		try { log_format = get_int(line, "log-format"); } catch (Exception e) {}
		try { log_space = get_int(line, "log-space"); } catch (Exception e) {}
		try { altitude_32 = get_int(line, "altitude-32"); } catch (Exception e) {}
		try { version = get_string(line, "software-version"); } catch (Exception e) {}

		/* Version also contains MS5607 info, which we ignore here */

		try { ms5607().reserved = get_int(line, "ms5607 reserved:"); } catch (Exception e) {}
		try { ms5607().sens = get_int(line, "ms5607 sens:"); } catch (Exception e) {}
		try { ms5607().off = get_int(line, "ms5607 off:"); } catch (Exception e) {}
		try { ms5607().tcs = get_int(line, "ms5607 tcs:"); } catch (Exception e) {}
		try { ms5607().tco = get_int(line, "ms5607 tco:"); } catch (Exception e) {}
		try { ms5607().tref = get_int(line, "ms5607 tref:"); } catch (Exception e) {}
		try { ms5607().tempsens = get_int(line, "ms5607 tempsens:"); } catch (Exception e) {}
		try { ms5607().crc = get_int(line, "ms5607 crc:"); } catch (Exception e) {}

		/* Config show replies */

		try {
			if (line.startsWith("Config version")) {
				String[] bits = line.split("\\s+");
				if (bits.length >= 3) {
					String[] cfg = bits[2].split("\\.");

					if (cfg.length >= 2) {
						config_major = Integer.parseInt(cfg[0]);
						config_minor = Integer.parseInt(cfg[1]);
					}
				}
			}
		} catch (Exception e) {}

		/* HAS_FLIGHT */
		try { main_deploy = get_int(line, "Main deploy:"); } catch (Exception e) {}
		try { apogee_delay = get_int(line, "Apogee delay:"); } catch (Exception e) {}
		try { apogee_lockout = get_int(line, "Apogee lockout:"); } catch (Exception e) {}

		/* HAS_RADIO */
		try {
			radio_frequency = get_int(line, "Frequency:");
			if (radio_frequency < 0)
				radio_frequency = 434550;
		} catch (Exception e) {}
		try { callsign = get_string(line, "Callsign:"); } catch (Exception e) {}
		try { radio_enable = get_int(line, "Radio enable:"); } catch (Exception e) {}
		try { radio_calibration = get_int(line, "Radio cal:"); } catch (Exception e) {}
		try { telemetry_rate = get_int(line, "Telemetry rate:"); } catch (Exception e) {}

		/* Old HAS_RADIO values */
		try { radio_channel = get_int(line, "Radio channel:"); } catch (Exception e) {}
		try { radio_setting = get_int(line, "Radio setting:"); } catch (Exception e) {}

		/* HAS_ACCEL */
		try {
			if (line.startsWith("Accel cal")) {
				String[] bits = line.split("\\s+");
				if (bits.length >= 6) {
					accel_cal_plus = Integer.parseInt(bits[3]);
					accel_cal_minus = Integer.parseInt(bits[5]);
					accel_cal_adjusted = false;
				}
			}
		} catch (Exception e) {}
		try { pad_orientation = get_int(line, "Pad orientation:"); } catch (Exception e) {}

		/* HAS_LOG */
		try { flight_log_max = get_int(line, "Max flight log:"); } catch (Exception e) {}
		try { log_fixed = get_int(line, "Log fixed:"); } catch (Exception e) {}

		/* HAS_IGNITE */
		try { ignite_mode = get_int(line, "Ignite mode:"); } catch (Exception e) {}

		/* HAS_AES */
		try { aes_key = get_string(line, "AES key:"); } catch (Exception e) {}

		/* AO_PYRO_NUM */
		try {
			npyro = get_int(line, "Pyro-count:");
			pyros = new AltosPyro[npyro];
			pyro = 0;
		} catch (Exception e) {}
		if (npyro != AltosLib.MISSING) {
			try {
				AltosPyro p = new AltosPyro(pyro, line);
				if (pyro < npyro)
					pyros[pyro++] = p;
			} catch (Exception e) {}
		}
		try { pyro_firing_time = get_int(line, "Pyro time:") / 100.0; } catch (Exception e) {}

		/* HAS_APRS */
		try { aprs_interval = get_int(line, "APRS interval:"); } catch (Exception e) {}
		try { aprs_ssid = get_int(line, "APRS SSID:"); } catch (Exception e) {}
		try { aprs_format = get_int(line, "APRS format:"); } catch (Exception e) {}
		try { aprs_offset = get_int(line, "APRS offset:"); } catch (Exception e) {}

		/* HAS_BEEP */
		try { beep = get_int(line, "Beeper setting:"); } catch (Exception e) {}

		/* HAS_RADIO_10MW */
		try { radio_10mw = get_int(line, "Radio 10mw limit:"); } catch (Exception e) {}

		/* HAS_TRACKER */
		try {
			int[] values = get_values(line, "Tracker setting:");
			tracker_motion = values[0];
			tracker_interval = values[1];
		} catch (Exception e) {}

		/* Storage info replies */
		try { storage_size = get_int(line, "Storage size:"); } catch (Exception e) {}
		try { storage_erase_unit = get_int(line, "Storage erase unit:"); } catch (Exception e) {}

		/* Log listing replies */
		try {
			int flight = get_int(line, "flight");
			String[] tokens = line.split("\\s+");
			if (tokens.length >= 6) {
				int	start = -1, end = -1;
				try {
					if (tokens[2].equals("start"))
						start = AltosParse.parse_hex(tokens[3]);
					if (tokens[4].equals("end"))
						end = AltosParse.parse_hex(tokens[5]);
					if (flight != 0 && start >= 0 && end > 0) {
						int len;
						if (flights == null)
							len = 0;
						else
							len = flights.length;
						AltosEepromFlight [] new_flights = new AltosEepromFlight[len + 1];
						for (int i = 0; i < len; i++)
							new_flights[i] = flights[i];
						new_flights[len] = new AltosEepromFlight(flight, start, end);
						flights = new_flights;
						stored_flight = flights.length;
					}
				} catch (ParseException pe) { System.out.printf("Parse error %s\n", pe.toString()); }
			}
		}  catch (Exception e) {}

		/* HAS_GYRO */
		try {
			if (line.startsWith("IMU cal along")) {
				String[] bits = line.split("\\s+");
				if (bits.length >= 8) {
					accel_zero_along = Integer.parseInt(bits[3]);
					accel_zero_across = Integer.parseInt(bits[5]);
					accel_zero_through = Integer.parseInt(bits[7]);
				}
			}
		} catch (Exception e) {}
	}

	public AltosConfigData() {
		reset();
	}

	private void read_link(AltosLink link, String finished) throws InterruptedException, TimeoutException {
		for (;;) {
			String line = link.get_reply();
			if (line == null)
				throw new TimeoutException();
			if (line.contains("Syntax error"))
				continue;
			this.parse_line(line);

			/* signals the end of the version info */
			if (line.startsWith(finished))
				break;
		}
	}

	public boolean has_frequency() {
		return radio_frequency != AltosLib.MISSING || radio_setting != AltosLib.MISSING || radio_channel != AltosLib.MISSING;
	}

	public boolean has_telemetry_rate() {
		return telemetry_rate != AltosLib.MISSING;
	}

	public void set_frequency(double freq) {
		int	frequency = radio_frequency;
		int	setting = radio_setting;

		if (frequency != AltosLib.MISSING) {
			radio_frequency = (int) Math.floor (freq * 1000 + 0.5);
			radio_channel = AltosLib.MISSING;
		} else if (setting != AltosLib.MISSING) {
			radio_setting =AltosConvert.radio_frequency_to_setting(freq, radio_calibration);
			radio_channel = AltosLib.MISSING;
		} else {
			radio_channel = AltosConvert.radio_frequency_to_channel(freq);
		}
	}

	public double frequency() {
		int	channel = radio_channel;
		int	setting = radio_setting;

		if (radio_frequency == AltosLib.MISSING && channel == AltosLib.MISSING && setting == AltosLib.MISSING)
			return AltosLib.MISSING;

		if (channel == AltosLib.MISSING)
			channel = 0;
		if (setting == AltosLib.MISSING)
			setting = 0;

		return AltosConvert.radio_to_frequency(radio_frequency,
						       setting,
						       radio_calibration,
						       channel);
	}

	boolean use_flash_for_config() {
		if (product.startsWith("TeleMega"))
			return false;
		if (product.startsWith("TeleMetrum-v2"))
			return false;
		if (product.startsWith("TeleMetrum-v3"))
			return false;
		if (product.startsWith("EasyMega"))
			return false;
		return true;
	}


	public boolean mma655x_inverted() throws AltosUnknownProduct {
		if (product != null) {
			if (product.startsWith("EasyMega-v1"))
				return false;
			if (product.startsWith("TeleMetrum-v2"))
				return true;
			if (product.startsWith("TeleMega-v2"))
				return false;
			if (product.startsWith("TeleMega-v1"))
				return false;
		}
		throw new AltosUnknownProduct(product);
	}

	public boolean adxl375_inverted() throws AltosUnknownProduct {
		if (product != null) {
			if (product.startsWith("EasyMega-v2"))
				return true;
			if (product.startsWith("TeleMetrum-v3"))
				return true;
			if (product.startsWith("TeleMega-v4"))
				return true;
			if (product.startsWith("EasyMotor-v2"))
				return true;
		}
		throw new AltosUnknownProduct(product);
	}

	public int adxl375_axis() throws AltosUnknownProduct {
		if (product != null) {
			if (product.startsWith("EasyMega-v2"))
				return AltosAdxl375.X_AXIS;
			if (product.startsWith("TeleMetrum-v3"))
				return AltosAdxl375.X_AXIS;
			if (product.startsWith("TeleMega-v4"))
				return AltosAdxl375.X_AXIS;
			if (product.startsWith("EasyMotor-v2"))
				return AltosAdxl375.X_AXIS;

		}
		throw new AltosUnknownProduct(product);
	}

	public void get_values(AltosConfigValues source) throws AltosConfigDataException {

		/* HAS_FLIGHT */
		if (main_deploy != AltosLib.MISSING)
			main_deploy = source.main_deploy();
		if (apogee_delay != AltosLib.MISSING)
			apogee_delay = source.apogee_delay();
		if (apogee_lockout != AltosLib.MISSING)
			apogee_lockout = source.apogee_lockout();

		/* HAS_RADIO */
		if (has_frequency())
			set_frequency(source.radio_frequency());
		if (radio_enable != AltosLib.MISSING)
			radio_enable = source.radio_enable();
		if (callsign != null)
			callsign = source.callsign();
		if (telemetry_rate != AltosLib.MISSING)
			telemetry_rate = source.telemetry_rate();

		/* HAS_ACCEL */
		if (pad_orientation != AltosLib.MISSING)
			pad_orientation = source.pad_orientation();

		if (accel_cal_plus_cooked != AltosLib.MISSING)
			accel_cal_plus_cooked = source.accel_cal_plus();

		if (accel_cal_minus_cooked != AltosLib.MISSING)
			accel_cal_minus_cooked = source.accel_cal_minus();

		/* HAS_LOG */
		if (flight_log_max != AltosLib.MISSING)
			flight_log_max = source.flight_log_max();

		/* HAS_IGNITE */
		if (ignite_mode != AltosLib.MISSING)
			ignite_mode = source.ignite_mode();

		/* AO_PYRO_NUM */
		if (npyro != AltosLib.MISSING)
			pyros = source.pyros();
		if (pyro_firing_time != AltosLib.MISSING)
			pyro_firing_time = source.pyro_firing_time();

		/* HAS_APRS */
		if (aprs_interval != AltosLib.MISSING)
			aprs_interval = source.aprs_interval();
		if (aprs_ssid != AltosLib.MISSING)
			aprs_ssid = source.aprs_ssid();
		if (aprs_format != AltosLib.MISSING)
			aprs_format = source.aprs_format();
		if (aprs_offset != AltosLib.MISSING)
			aprs_offset = source.aprs_offset();

		/* HAS_BEEP */
		if (beep != AltosLib.MISSING)
			beep = source.beep();

		/* HAS_RADIO_10MW */
		if (radio_10mw != AltosLib.MISSING)
			radio_10mw = source.radio_10mw();

		/* HAS_TRACKER */
		if (tracker_motion != AltosLib.MISSING)
			tracker_motion = source.tracker_motion();
		if (tracker_interval != AltosLib.MISSING)
			tracker_interval = source.tracker_interval();
	}

	public void set_values(AltosConfigValues dest) {
		dest.set_serial(serial);
		dest.set_product(product);
		dest.set_version(version);
		dest.set_altitude_32(altitude_32);
		dest.set_main_deploy(main_deploy);
		dest.set_apogee_delay(apogee_delay);
		dest.set_apogee_lockout(apogee_lockout);
		dest.set_radio_calibration(radio_calibration);
		dest.set_radio_frequency(frequency());
		dest.set_telemetry_rate(telemetry_rate);
		boolean max_enabled = true;

		if (log_space() == 0)
			max_enabled = false;

		if (log_fixed != AltosLib.MISSING)
			max_enabled = false;

		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TINY:
			max_enabled = false;
			break;
		default:
			if (flights != null)
				max_enabled = false;
			break;
		}

		dest.set_flight_log_max_enabled(max_enabled);
		dest.set_radio_enable(radio_enable);
		dest.set_flight_log_max_limit(log_space() >> 10, storage_erase_unit >> 10);
		dest.set_flight_log_max(flight_log_max);
		dest.set_ignite_mode(ignite_mode);
		dest.set_pad_orientation(pad_orientation);
		dest.set_accel_cal(accel_cal_plus(AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP),
				   accel_cal_minus(AltosLib.AO_PAD_ORIENTATION_ANTENNA_UP));
		dest.set_callsign(callsign);
		if (npyro != AltosLib.MISSING)
			dest.set_pyros(pyros);
		else
			dest.set_pyros(null);
		dest.set_pyro_firing_time(pyro_firing_time);
		dest.set_aprs_interval(aprs_interval);
		dest.set_aprs_ssid(aprs_ssid);
		dest.set_aprs_format(aprs_format);
		dest.set_aprs_offset(aprs_offset);
		dest.set_beep(beep);
		dest.set_radio_10mw(radio_10mw);
		dest.set_tracker_motion(tracker_motion);
		dest.set_tracker_interval(tracker_interval);
	}

	public boolean log_has_state() {
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_TELEGPS:
			return false;
		}
		return true;
	}

	public void save(AltosLink link, boolean remote) throws InterruptedException, TimeoutException {

		/* HAS_FLIGHT */
		if (main_deploy != AltosLib.MISSING)
			link.printf("c m %d\n", main_deploy);
		if (apogee_delay != AltosLib.MISSING)
			link.printf("c d %d\n", apogee_delay);
		if (apogee_lockout != AltosLib.MISSING)
			link.printf("c L %d\n", apogee_lockout);

		/* HAS_RADIO */
		if (has_frequency()) {
			boolean has_frequency = radio_frequency != AltosLib.MISSING;
			boolean has_setting = radio_setting != AltosLib.MISSING;
			double frequency = frequency();
			link.set_radio_frequency(frequency,
							has_frequency,
							has_setting,
							radio_calibration);
			/* When remote, reset the dongle frequency at the same time */
			if (remote && frequency != link.frequency) {
				link.stop_remote();
				link.set_radio_frequency(frequency);
				link.start_remote();
			}
		}

		if (telemetry_rate != AltosLib.MISSING) {
			link.printf("c T %d\n", telemetry_rate);
			if (remote && telemetry_rate != link.telemetry_rate) {
				link.stop_remote();
				link.set_telemetry_rate(telemetry_rate);
				link.start_remote();
			}
		}

		if (callsign != null) {
			link.printf("c c %s\n", callsign);
			if (remote && !callsign.equals(link.callsign)) {
				System.out.printf("changing link callsign from %s to %s\n", link.callsign, callsign);
				link.stop_remote();
				link.set_callsign(callsign);
				link.start_remote();
			}
		}

		if (radio_enable != AltosLib.MISSING)
			link.printf("c e %d\n", radio_enable);

		/* HAS_ACCEL */
		/* set orientation first so that we know how to set the accel cal */
		if (pad_orientation != AltosLib.MISSING)
			link.printf("c o %d\n", pad_orientation);
		int plus = accel_cal_plus(pad_orientation);
		int minus = accel_cal_minus(pad_orientation);
		if (plus != AltosLib.MISSING && minus != AltosLib.MISSING) {
			if (plus < 0)
				plus = 65536 + plus;
			if (minus < 0)
				minus = 65536 + minus;
			if (accel_zero_along != AltosLib.MISSING &&
			    accel_zero_across != AltosLib.MISSING &&
			    accel_zero_through != AltosLib.MISSING)
				link.printf("c a %d %d %d %d %d\n",
 					    plus, minus,
					    accel_zero_along,
					    accel_zero_across,
					    accel_zero_through);
			else
				link.printf("c a %d %d\n", plus, minus);
		}

		/* HAS_LOG */
		if (flight_log_max != 0 && flight_log_max != AltosLib.MISSING)
			link.printf("c l %d\n", flight_log_max);

		/* HAS_IGNITE */
		if (ignite_mode != AltosLib.MISSING)
			link.printf("c i %d\n", ignite_mode);

		/* HAS_AES */
		/* UI doesn't support AES key config */

		/* AO_PYRO_NUM */
		if (npyro != AltosLib.MISSING) {
			for (int p = 0; p < pyros.length; p++) {
				link.printf("c P %s\n",
						   pyros[p].toString());
			}
		}
		if (pyro_firing_time != AltosLib.MISSING)
			link.printf("c I %d\n", (int) (pyro_firing_time * 100.0 + 0.5));

		/* HAS_APRS */
		if (aprs_interval != AltosLib.MISSING)
			link.printf("c A %d\n", aprs_interval);
		if (aprs_ssid != AltosLib.MISSING)
			link.printf("c S %d\n", aprs_ssid);
		if (aprs_format != AltosLib.MISSING)
			link.printf("c C %d\n", aprs_format);
		if (aprs_offset != AltosLib.MISSING)
			link.printf("c O %d\n", aprs_offset);

		/* HAS_BEEP */
		if (beep != AltosLib.MISSING)
			link.printf("c b %d\n", beep);

		/* HAS_RADIO_10MW */
		if (radio_10mw != AltosLib.MISSING)
			link.printf("c p %d\n", radio_10mw);

		/* HAS_TRACKER */
		if (tracker_motion != AltosLib.MISSING && tracker_interval != AltosLib.MISSING)
			link.printf("c t %d %d\n", tracker_motion, tracker_interval);

		/* HAS_GYRO */
		/* UI doesn't support accel cal */

		link.printf("c w\n");
		read_link(link, "Saved");
	}

	public AltosConfigData(AltosLink link) throws InterruptedException, TimeoutException {
		reset();
		link.printf("c s\nf\nv\n");
		read_link(link, "software-version");
		switch (log_format) {
		case AltosLib.AO_LOG_FORMAT_UNKNOWN:
		case AltosLib.AO_LOG_FORMAT_NONE:
			break;
		default:
			link.printf("l\n");
			read_link(link, "done");
			break;
		}
		adjust_accel_cal();
	}
}
