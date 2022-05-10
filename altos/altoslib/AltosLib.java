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

import java.util.*;
import java.io.*;
import java.nio.charset.Charset;

public class AltosLib {
	/* EEProm command letters */
	public static final int AO_LOG_FLIGHT = 'F';
	public static final int AO_LOG_SENSOR = 'A';
	public static final int AO_LOG_TEMP_VOLT = 'T';
	public static final int AO_LOG_DEPLOY = 'D';
	public static final int AO_LOG_STATE = 'S';
	public static final int AO_LOG_GPS_POS = 'P';
	public static final int AO_LOG_GPS_TIME = 'G';
	public static final int AO_LOG_GPS_LAT = 'N';
	public static final int AO_LOG_GPS_LON = 'W';
	public static final int AO_LOG_GPS_ALT = 'H';
	public static final int AO_LOG_GPS_SAT = 'V';
	public static final int AO_LOG_GPS_DATE = 'Y';
	public static final int AO_LOG_PRESSURE = 'P';

	public static boolean is_gps_cmd(int cmd) {
		switch (cmd) {
		case AltosLib.AO_LOG_GPS_POS:
		case AltosLib.AO_LOG_GPS_TIME:
		case AltosLib.AO_LOG_GPS_LAT:
		case AltosLib.AO_LOG_GPS_LON:
		case AltosLib.AO_LOG_GPS_ALT:
		case AltosLib.AO_LOG_GPS_SAT:
		case AltosLib.AO_LOG_GPS_DATE:
			return true;
		}
		return false;
	}

	/* Added for header fields in eeprom files */
	public static final int AO_LOG_CONFIG_VERSION = 1000;
	public static final int AO_LOG_MAIN_DEPLOY = 1001;
	public static final int AO_LOG_APOGEE_DELAY = 1002;
	public static final int AO_LOG_RADIO_CHANNEL = 1003;
	public static final int AO_LOG_CALLSIGN = 1004;
	public static final int AO_LOG_ACCEL_CAL = 1005;
	public static final int AO_LOG_RADIO_CAL = 1006;
	public static final int AO_LOG_MAX_FLIGHT_LOG = 1007;
	public static final int AO_LOG_MANUFACTURER = 2000;
	public static final int AO_LOG_PRODUCT = 2001;
	public static final int AO_LOG_SERIAL_NUMBER = 2002;
	public static final int AO_LOG_LOG_FORMAT = 2003;

	public static final int AO_LOG_FREQUENCY = 2004;
	public static final int AO_LOG_APOGEE_LOCKOUT = 2005;
	public static final int AO_LOG_RADIO_RATE = 2006;
	public static final int AO_LOG_IGNITE_MODE = 2007;
	public static final int AO_LOG_PAD_ORIENTATION = 2008;
	public static final int AO_LOG_RADIO_ENABLE = 2009;
	public static final int AO_LOG_AES_KEY = 2010;
	public static final int AO_LOG_APRS = 2011;
	public static final int AO_LOG_BEEP_SETTING = 2012;
	public static final int AO_LOG_TRACKER_SETTING = 2013;
	public static final int AO_LOG_PYRO_TIME = 2014;
	public static final int AO_LOG_APRS_ID = 2015;
	public static final int AO_LOG_ALTITUDE_32 = 2016;

	/* Added for header fields in telemega files */
	public static final int AO_LOG_BARO_RESERVED = 3000;
	public static final int AO_LOG_BARO_SENS = 3001;
	public static final int AO_LOG_BARO_OFF = 3002;
	public static final int AO_LOG_BARO_TCS = 3004;
	public static final int AO_LOG_BARO_TCO = 3005;
	public static final int AO_LOG_BARO_TREF = 3006;
	public static final int AO_LOG_BARO_TEMPSENS = 3007;
	public static final int AO_LOG_BARO_CRC = 3008;
	public static final int AO_LOG_IMU_CAL = 3009;

	public static final int AO_LOG_SOFTWARE_VERSION = 9999;

	public final static int	MISSING = 0x7fffffff;

	/* Added to flag invalid records */
	public static final int AO_LOG_INVALID = -1;

	/* Flight state numbers and names */
	public static final int ao_flight_startup = 0;
	public static final int ao_flight_idle = 1;
	public static final int ao_flight_pad = 2;
	public static final int ao_flight_boost = 3;
	public static final int ao_flight_fast = 4;
	public static final int ao_flight_coast = 5;
	public static final int ao_flight_drogue = 6;
	public static final int ao_flight_main = 7;
	public static final int ao_flight_landed = 8;
	public static final int ao_flight_invalid = 9;
	public static final int ao_flight_stateless = 10;

	/* USB product IDs */
	public final static int vendor_altusmetrum = 0xfffe;

	public final static int product_altusmetrum = 0x000a;
	public final static int product_telemetrum = 0x000b;
	public final static int product_teledongle = 0x000c;
	public final static int product_easytimer = 0x000d;
	public final static int product_telebt = 0x000e;
	public final static int product_telelaunch = 0x000f;
	public final static int product_telelco = 0x0010;
	public final static int product_telescience = 0x0011;
	public final static int product_telepyro =0x0012;
	public final static int product_telemega = 0x0023;
	public final static int product_megadongle = 0x0024;
	public final static int product_telegps = 0x0025;
	public final static int product_easymini = 0x0026;
	public final static int product_telemini = 0x0027;
	public final static int product_easymega = 0x0028;
	public final static int product_usbtrng = 0x0029;
	public final static int product_usbrelay = 0x002a;
	public final static int product_mpusb = 0x002b;
	public final static int product_easymotor = 0x002c;
	public final static int product_altusmetrum_min = 0x000a;
	public final static int product_altusmetrum_max = 0x002c;

	public final static int product_any = 0x10000;
	public final static int product_basestation = 0x10000 + 1;
	public final static int product_altimeter = 0x10000 + 2;

	private static class Product {
		final String	name;
		final int	product;

		Product (String name, int product) {
			this.name = name;
			this.product = product;
		}
	}

	private static Product[] products = {
		new Product("telemetrum", product_telemetrum),
		new Product("teleballoon", product_telemetrum),
		new Product("teledongle", product_teledongle),
		new Product("easytimer", product_easytimer),
		new Product("telebt", product_telebt),
		new Product("telelaunch", product_telelaunch),
		new Product("telelco", product_telelco),
		new Product("telescience", product_telescience),
		new Product("telepyro", product_telepyro),
		new Product("telemega", product_telemega),
		new Product("megadongle", product_megadongle),
		new Product("telegps", product_telegps),
		new Product("easymini", product_easymini),
		new Product("telemini", product_telemini),
		new Product("easymega", product_easymega),
		new Product("easymotor", product_easymotor)
	};

	public static int name_to_product(String name) {
		String low = name.toLowerCase();

		for (int i = 0; i < products.length; i++)
			if (low.startsWith(products[i].name))
				return products[i].product;
		return product_any;
	}

	public static boolean has_9dof(int device_type) {
		return device_type == product_telemega || device_type == product_easymega;
	}

	public static boolean has_radio(int device_type) {
		return device_type != product_easymini && device_type != product_easymega;
	}

	public static boolean has_gps(int device_type) {
		return device_type == product_telemetrum ||
			device_type == product_telemega ||
			device_type == product_telegps;
	}

	/* Bluetooth "identifier" (bluetooth sucks) */
	public final static String bt_product_telebt = "TeleBT";

	/* "good" voltages */

	public final static double ao_battery_good = 3.8;
	public final static double ao_igniter_good = 3.5;

	/* Telemetry modes */
	public static final int ao_telemetry_off = 0;
	public static final int ao_telemetry_min = 1;
	public static final int ao_telemetry_standard = 1;
	public static final int ao_telemetry_0_9 = 2;
	public static final int ao_telemetry_0_8 = 3;
	public static final int ao_telemetry_max = 3;

	private static final String[] ao_telemetry_name = {
		"Off", "Standard Telemetry", "TeleMetrum v0.9", "TeleMetrum v0.8"
	};

	public static final int ao_telemetry_rate_38400 = 0;
	public static final int ao_telemetry_rate_9600 = 1;
	public static final int ao_telemetry_rate_2400 = 2;
	public static final int ao_telemetry_rate_max = 2;

	public static final Integer[] ao_telemetry_rate_values = {
		38400, 9600, 2400
	};

	public static final int ao_aprs_format_compressed = 0;
	public static final int ao_aprs_format_uncompressed = 1;

	public static final String[] ao_aprs_format_name = {
		"Compressed", "Uncompressed"
	};

	public static final String launch_sites_url = "https://maps.altusmetrum.org/launch-sites.txt";
	public static final String launch_sites_env = "LAUNCH_SITES";
//	public static final String launch_sites_url = "file:///home/keithp/misc/text/altusmetrum/AltOS/launch-sites.txt";

	public static final String unit_info_url = "https://altusmetrum.org/cgi-bin/unitinfo.cgi?sn=%d";
	public static final String unit_info_env = "UNIT_INFO";

	public static final int ao_telemetry_standard_len = 32;
	public static final int ao_telemetry_0_9_len = 95;
	public static final int ao_telemetry_0_8_len = 94;

	private static final int[] ao_telemetry_len = {
		0, 32, 95, 94
	};

	private static HashMap<String,Integer>	string_to_state = new HashMap<String,Integer>();

	private static boolean map_initialized = false;

	public static void initialize_map()
	{
		string_to_state.put("startup", ao_flight_startup);
		string_to_state.put("idle", ao_flight_idle);
		string_to_state.put("pad", ao_flight_pad);
		string_to_state.put("boost", ao_flight_boost);
		string_to_state.put("fast", ao_flight_fast);
		string_to_state.put("coast", ao_flight_coast);
		string_to_state.put("drogue", ao_flight_drogue);
		string_to_state.put("apogee", ao_flight_coast);
		string_to_state.put("main", ao_flight_main);
		string_to_state.put("landed", ao_flight_landed);
		string_to_state.put("invalid", ao_flight_invalid);
		string_to_state.put("stateless", ao_flight_stateless);
		map_initialized = true;
	}

	public static int telemetry_len(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_len[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
	}

	public static String telemetry_name(int telemetry) {
		if (telemetry <= ao_telemetry_max)
			return ao_telemetry_name[telemetry];
		throw new IllegalArgumentException(String.format("Invalid telemetry %d",
								 telemetry));
	}

	private static int[] split_version(String version) {
		String[] tokens = version.split("\\.");
		int[] ret = new int[tokens.length];
		for (int i = 0; i < tokens.length; i++)
			ret[i] = Integer.parseInt(tokens[i]);
		return ret;
	}

	public static int compare_version(String version_a, String version_b) {
		int[] a = split_version(version_a);
		int[] b = split_version(version_b);

		for (int i = 0; i < Math.min(a.length, b.length); i++) {
			if (a[i] < b[i])
				return -1;
			if (a[i] > b[i])
				return 1;
		}
		if (a.length < b.length)
			return -1;
		if (a.length > b.length)
			return 1;
		return 0;
	}

	private static String[] state_to_string = {
		"startup",
		"idle",
		"pad",
		"boost",
		"fast",
		"coast",
		"drogue",
		"main",
		"landed",
		"invalid",
		"stateless",
	};

	private static String[] state_to_string_capital = {
		"Startup",
		"Idle",
		"Pad",
		"Boost",
		"Fast",
		"Coast",
		"Drogue",
		"Main",
		"Landed",
		"Invalid",
		"Stateless",
	};

	public static int state(String state) {
		if (!map_initialized)
			initialize_map();
		if (string_to_state.containsKey(state))
			return string_to_state.get(state);
		return ao_flight_invalid;
	}

	public static String state_name(int state) {
		if (state < 0 || state_to_string.length <= state)
			return "invalid";
		return state_to_string[state];
	}

	public static String state_name_capital(int state) {
		if (state < 0 || state_to_string.length <= state)
			return "Invalid";
		return state_to_string_capital[state];
	}

	public static final int AO_GPS_VALID = (1 << 4);
	public static final int AO_GPS_RUNNING = (1 << 5);
	public static final int AO_GPS_DATE_VALID = (1 << 6);
	public static final int AO_GPS_NUM_SAT_SHIFT = 0;
	public static final int AO_GPS_NUM_SAT_MASK = 0xf;

	public static final int AO_PAD_ORIENTATION_ANTENNA_UP = 0;
	public static final int AO_PAD_ORIENTATION_ANTENNA_DOWN = 1;
	public static final int AO_PAD_ORIENTATION_WORDS_UPRIGHT = 2;
	public static final int AO_PAD_ORIENTATION_WORDS_UPSIDEDOWN = 3;
	public static final int AO_PAD_ORIENTATION_BIG_PARTS_UP = 4;
	public static final int AO_PAD_ORIENTATION_BIG_PARTS_DOWN = 5;

	public static final int AO_LOG_FORMAT_UNKNOWN = 0;
	public static final int AO_LOG_FORMAT_FULL = 1;
	public static final int AO_LOG_FORMAT_TINY = 2;
	public static final int AO_LOG_FORMAT_TELEMETRY = 3;
	public static final int AO_LOG_FORMAT_TELESCIENCE = 4;
	public static final int AO_LOG_FORMAT_TELEMEGA_OLD = 5;
	public static final int AO_LOG_FORMAT_EASYMINI1 = 6;
	public static final int AO_LOG_FORMAT_TELEMETRUM = 7;
	public static final int AO_LOG_FORMAT_TELEMINI2 = 8;
	public static final int AO_LOG_FORMAT_TELEGPS = 9;
	public static final int AO_LOG_FORMAT_TELEMEGA = 10;
	public static final int AO_LOG_FORMAT_DETHERM = 11;
	public static final int AO_LOG_FORMAT_TELEMINI3 = 12;
	public static final int AO_LOG_FORMAT_TELEFIRETWO = 13;
	public static final int AO_LOG_FORMAT_EASYMINI2 = 14;
	public static final int AO_LOG_FORMAT_TELEMEGA_3 = 15;
	public static final int AO_LOG_FORMAT_EASYMEGA_2 = 16;
	public static final int AO_LOG_FORMAT_TELESTATIC = 17;
	public static final int AO_LOG_FORMAT_MICROPEAK2 = 18;
	public static final int AO_LOG_FORMAT_TELEMEGA_4 = 19;
	public static final int AO_LOG_FORMAT_EASYMOTOR = 20;
	public static final int AO_LOG_FORMAT_TELEMEGA_5 = 21;
	public static final int AO_LOG_FORMAT_NONE = 127;

	public static final int	model_mpu6000 = 0;
	public static final int	model_mpu9250 = 1;
	public static final int	model_adxl375 = 2;
	public static final int	model_bmx160 = 3;
	public static final int model_hmc5883 = 4;
	public static final int model_mmc5983 = 5;

	public static boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	public static final boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	public static final boolean ishex(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!ishex(s.charAt(i)))
				return false;
		return true;
	}

	public static int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	public static int fromhex(String s) throws NumberFormatException {
		int c, v = 0;
		for (int i = 0; i < s.length(); i++) {
			c = s.charAt(i);
			if (!ishex(c)) {
				if (i == 0)
					throw new NumberFormatException(String.format("invalid hex \"%s\"", s));
				return v;
			}
			v = v * 16 + fromhex(c);
		}
		return v;
	}

	public static boolean isdec(int c) {
		if ('0' <= c && c <= '9')
			return true;
		return false;
	}

	public static boolean isdec(String s) {
		for (int i = 0; i < s.length(); i++)
			if (!isdec(s.charAt(i)))
				return false;
		return true;
	}

	public static int fromdec(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		return -1;
	}

	public static int int8(int[] bytes, int i) {
		return (int) (byte) bytes[i];
	}

	public static int uint8(int[] bytes, int i) {
		return bytes[i];
	}

	public static int int16(int[] bytes, int i) {
		return (int) (short) (bytes[i] + (bytes[i+1] << 8));
	}

	public static int uint16(int[] bytes, int i) {
		return bytes[i] + (bytes[i+1] << 8);
	}

	public static int uint32(int[] bytes, int i) {
		return bytes[i] +
			(bytes[i+1] << 8) +
			(bytes[i+2] << 16) +
			(bytes[i+3] << 24);
	}

	public static int int32(int[] bytes, int i) {
		return (int) uint32(bytes, i);
	}

	public static final Charset	unicode_set = Charset.forName("UTF-8");

	public static String string(int[] bytes, int s, int l) {
		if (s + l > bytes.length) {
			if (s > bytes.length) {
				s = bytes.length;
				l = 0;
			} else {
				l = bytes.length - s;
			}
		}

		int i;
		for (i = l - 1; i >= 0; i--)
			if (bytes[s+i] != 0)
				break;

		l = i + 1;
		byte[]	b = new byte[l];

		for (i = 0; i < l; i++)
			b[i] = (byte) bytes[s+i];
		String n = new String(b, unicode_set);
		return n;
	}

	public static int hexbyte(String s, int i) {
		int c0, c1;

		if (s.length() < i + 2)
			throw new NumberFormatException(String.format("invalid hex \"%s\"", s));
		c0 = s.charAt(i);
		if (!ishex(c0))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c0));
		c1 = s.charAt(i+1);
		if (!ishex(c1))
			throw new NumberFormatException(String.format("invalid hex \"%c\"", c1));
		return fromhex(c0) * 16 + fromhex(c1);
	}

	public static int[] hexbytes(String s) {
		int	n;
		int[]	r;
		int	i;

		if ((s.length() & 1) != 0)
			throw new NumberFormatException(String.format("invalid line \"%s\"", s));
		byte[] bytes = s.getBytes(unicode_set);
		n = bytes.length / 2;
		r = new int[n];
		for (i = 0; i < n; i++) {
			int h = fromhex(bytes[(i << 1)]);
			int l = fromhex(bytes[(i << 1) + 1]);
			if (h < 0 || l < 0)
				throw new NumberFormatException(String.format("invalid hex \"%c%c\"",
									      bytes[(i<<1)], bytes[(i<<1) + 1]));
			r[i] = (h << 4) + l;
		}
		return r;
	}

	public static long fromdec(String s) throws NumberFormatException {
		int c;
		long v = 0;
		long sign = 1;
		for (int i = 0; i < s.length(); i++) {
			c = s.charAt(i);
			if (i == 0 && c == '-') {
				sign = -1;
			} else if (!isdec(c)) {
				if (i == 0)
					throw new NumberFormatException(String.format("invalid number \"%s\"", s));
				return v;
			} else
				v = v * 10 + fromdec(c);
		}
		return v * sign;
	}

	public static String gets(FileInputStream s) throws IOException {
		int c;
		String	line = "";

		while ((c = s.read()) != -1) {
			if (c == '\r')
				continue;
			if (c == '\n') {
				return line;
			}
			line = line + (char) c;
		}
		return null;
	}

	public static String replace_extension(String input, String extension) {
		int dot = input.lastIndexOf(".");
		if (dot > 0)
			input = input.substring(0,dot);
		return input.concat(extension);
	}

	public static File replace_extension(File input, String extension) {
		return new File(replace_extension(input.getPath(), extension));
	}

	public static String product_name(int product_id) {
		switch (product_id) {
		case product_altusmetrum: return "AltusMetrum";
		case product_telemetrum: return "TeleMetrum";
		case product_teledongle: return "TeleDongle";
		case product_easytimer: return "EasyTimer";
		case product_telebt: return "TeleBT";
		case product_telelaunch: return "TeleLaunch";
		case product_telelco: return "TeleLco";
		case product_telescience: return "Telescience";
		case product_telepyro: return "TelePyro";
		case product_telemega: return "TeleMega";
		case product_megadongle: return "MegaDongle";
		case product_telegps: return "TeleGPS";
		case product_easymini: return "EasyMini";
		case product_telemini: return "TeleMini";
		case product_easymotor: return "EasyMotor";
		default: return "unknown";
		}
	}

	public static String igniter_name(int i) {
		return String.format("Igniter %c", 'A' + i);
	}

	public static String igniter_short_name(int i) {
		return String.format("igniter_%c", 'a' + i);
	}

	public static AltosRecordSet record_set(File file) throws FileNotFoundException, IOException {
		FileInputStream in;
		in = new FileInputStream(file);
		if (file.getName().endsWith("telem")) {
			return new AltosTelemetryFile(in);
		} else if (file.getName().endsWith("eeprom")) {
			return new AltosEepromFile(in);
		} else {
			String	name = file.getName();
			int	dot = name.lastIndexOf('.');
			String	extension;

			if (dot == -1)
				throw new IOException(String.format("%s (Missing extension)", file.toString()));
			else {
				extension = name.substring(dot);
				throw new IOException(String.format("%s (Invalid extension '%s')",
								    file.toString(),
								    extension));
			}
		}
	}

}
