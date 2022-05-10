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

public class AltosPreferences {
	public static AltosPreferencesBackend backend = null;

	/* logdir preference name */
	public final static String logdirPreference = "LOGDIR";

	/* channel preference name */
	public final static String channelPreferenceFormat = "CHANNEL-%d";

	/* frequency preference name */
	public final static String frequencyPreferenceFormat = "FREQUENCY-%d";

	/* telemetry format preference name */
	public final static String telemetryPreferenceFormat = "TELEMETRY-%d";

	/* telemetry rate format preference name */
	public final static String telemetryRatePreferenceFormat = "RATE-%d";

	/* log file format preference name */
	public final static String logfilePreferenceFormat = "LOGFILE-%d";

	/* state preference name */
	public final static String statePreferenceHead = "STATE-";
	public final static String statePreferenceFormat = "STATE-%d";
	public final static String statePreferenceLatest = "STATE-LATEST";

	/* voice preference name */
	public final static String voicePreference = "VOICE";

	/* callsign preference name */
	public final static String callsignPreference = "CALLSIGN";

	/* firmware directory preference name */
	public final static String firmwaredirPreference = "FIRMWARE";

	/* serial debug preference name */
	public final static String serialDebugPreference = "SERIAL-DEBUG";

	/* scanning telemetry preferences name */
	public final static String scanningTelemetryPreference = "SCANNING-TELEMETRY";

	/* scanning telemetry rate preferences name */
	public final static String scanningTelemetryRatePreference = "SCANNING-RATE";

	/* Launcher serial preference name */
	public final static String launcherSerialPreference = "LAUNCHER-SERIAL";

	/* Launcher channel preference name */
	public final static String launcherChannelPreference = "LAUNCHER-CHANNEL";

	/* Default logdir is ~/AltusMetrum */
	public final static String logdirName = "AltusMetrum";

	/* Log directory */
	public static File logdir;

	/* Last log directory - use this next time we open or save something */
	public static File last_logdir;

	/* Map directory -- hangs of logdir */
	public static File mapdir;

	/* Frequency (map serial to frequency) */
	public static Hashtable<Integer, Double> frequencies;

	/* Telemetry (map serial to telemetry format) */
	public static Hashtable<Integer, Integer> telemetries;

	/* Telemetry rate (map serial to telemetry format) */
	public static Hashtable<Integer, Integer> telemetry_rates;

	/* Log file (map serial to logfile name) */
	public static Hashtable<Integer, File> logfiles;

	/* Voice preference */
	public static boolean voice;

	/* Callsign preference */
	public static String callsign;

	/* Firmware directory */
	public static File firmwaredir;

	/* Scanning telemetry */
	public static int scanning_telemetry;

	public static int scanning_telemetry_rate;

	/* List of frequencies */
	public final static String common_frequencies_node_name = "COMMON-FREQUENCIES";
	public static AltosFrequency[] common_frequencies;

	public final static String	frequency_count = "COUNT";
	public final static String	frequency_format = "FREQUENCY-%d";
	public final static String	description_format = "DESCRIPTION-%d";
	public final static String	frequenciesPreference = "FREQUENCIES-1";

	/* Units preference */

	public final static String	unitsPreference = "IMPERIAL-UNITS";

	/* Maps cache size preference name */
	final static String mapCachePreference = "MAP-CACHE";

	static LinkedList<AltosMapCacheListener> map_cache_listeners;

	public static int map_cache = 9;

	final static String mapTypePreference = "MAP-TYPE";
	static int	map_type;

	public static AltosFrequency[] load_common_frequencies() {
		AltosFrequency[] frequencies = null;

		try {
			AltosJson json = AltosJson.fromString(backend.getString(frequenciesPreference,
										null));
			frequencies = (AltosFrequency[]) json.make((new AltosFrequency[1]).getClass());
		} catch (Exception e) {
		}

		if (frequencies == null) {
			if (backend.nodeExists(common_frequencies_node_name)) {
				AltosPreferencesBackend	node = backend.node(common_frequencies_node_name);
				int		count = node.getInt(frequency_count, 0);

				if (count > 0) {
					frequencies = new AltosFrequency[count];
					for (int i = 0; i < count; i++) {
						double	frequency;
						String	description;

						frequency = node.getDouble(String.format(frequency_format, i), 0.0);
						description = node.getString(String.format(description_format, i), null);
						frequencies[i] = new AltosFrequency(frequency, description);
					}
				}
			}
		}

		if (frequencies == null) {
			frequencies = new AltosFrequency[10];
			for (int i = 0; i < 10; i++) {
				frequencies[i] = new AltosFrequency(434.550 + i * .1,
									   String.format("Channel %d", i));
			}
		}
		return frequencies;
	}

	public static void save_common_frequencies() {
		AltosJson	json = new AltosJson(common_frequencies);
		backend.putString(frequenciesPreference, json.toString());
		flush_preferences();
	}

	public static int launcher_serial;

	public static int launcher_channel;

	public static void init(AltosPreferencesBackend in_backend) {

		if (backend != null)
			return;

		backend = in_backend;

		/* Initialize logdir from preferences */
		String logdir_string = backend.getString(logdirPreference, null);
		if (logdir_string != null)
			logdir = new File(logdir_string);
		else {
			logdir = new File(backend.homeDirectory(), logdirName);
			if (!logdir.exists())
				logdir.mkdirs();
		}
		mapdir = new File(logdir, "maps");
		if (!mapdir.exists())
			mapdir.mkdirs();

		frequencies = new Hashtable<Integer, Double>();

		telemetries = new Hashtable<Integer,Integer>();

		telemetry_rates = new Hashtable<Integer,Integer>();

		logfiles = new Hashtable<Integer,File>();

		voice = backend.getBoolean(voicePreference, true);

		callsign = backend.getString(callsignPreference,"N0CALL");

		scanning_telemetry = backend.getInt(scanningTelemetryPreference,(1 << AltosLib.ao_telemetry_standard));

		scanning_telemetry_rate = backend.getInt(scanningTelemetryRatePreference,(1 << AltosLib.ao_telemetry_rate_38400));

		launcher_serial = backend.getInt(launcherSerialPreference, 0);

		launcher_channel = backend.getInt(launcherChannelPreference, 0);

		String firmwaredir_string = backend.getString(firmwaredirPreference, null);
		if (firmwaredir_string != null)
			firmwaredir = new File(firmwaredir_string);
		else
			firmwaredir = null;

		common_frequencies = load_common_frequencies();

		AltosConvert.imperial_units = backend.getBoolean(unitsPreference, false);

		map_cache = backend.getInt(mapCachePreference, 9);
		map_cache_listeners = new LinkedList<AltosMapCacheListener>();
		map_type = backend.getInt(mapTypePreference, AltosMap.maptype_hybrid);
	}

	public static void flush_preferences() {
		backend.flush();
	}

	public static void set_logdir(File new_logdir) {
		synchronized (backend) {
			logdir = new_logdir;
			mapdir = new File(logdir, "maps");
			if (!mapdir.exists())
				mapdir.mkdirs();
			backend.putString(logdirPreference, logdir.getPath());
			flush_preferences();
		}
	}

	public static File logdir() {
		synchronized (backend) {
			return logdir;
		}
	}

	public static File last_logdir() {
		synchronized (backend) {
			if (last_logdir == null)
				last_logdir = logdir;
			return last_logdir;
		}
	}

	public static void set_last_logdir(File file) {
		synchronized(backend) {
			if (file != null && !file.isDirectory())
				file = file.getParentFile();
			if (file == null)
				file = new File(".");
			last_logdir = file;
		}
	}

	public static File mapdir() {
		synchronized (backend) {
			return mapdir;
		}
	}

	public static void set_frequency(int serial, double new_frequency) {
		synchronized (backend) {
			frequencies.put(serial, new_frequency);
			backend.putDouble(String.format(frequencyPreferenceFormat, serial), new_frequency);
			flush_preferences();
		}
	}

	public static double frequency(int serial) {
		synchronized (backend) {
			if (frequencies.containsKey(serial))
				return frequencies.get(serial);
			double frequency = backend.getDouble(String.format(frequencyPreferenceFormat, serial), 0);
			if (frequency == 0.0) {
				int channel = backend.getInt(String.format(channelPreferenceFormat, serial), 0);
				frequency = AltosConvert.radio_channel_to_frequency(channel);
			}
			frequencies.put(serial, frequency);
			return frequency;
		}
	}

	public static void set_telemetry(int serial, int new_telemetry) {
		synchronized (backend) {
			telemetries.put(serial, new_telemetry);
			backend.putInt(String.format(telemetryPreferenceFormat, serial), new_telemetry);
			flush_preferences();
		}
	}

	public static int telemetry(int serial) {
		synchronized (backend) {
			if (telemetries.containsKey(serial))
				return telemetries.get(serial);
			int telemetry = backend.getInt(String.format(telemetryPreferenceFormat, serial),
						   AltosLib.ao_telemetry_standard);
			telemetries.put(serial, telemetry);
			return telemetry;
		}
	}

	public static void set_telemetry_rate(int serial, int new_telemetry_rate) {
		synchronized (backend) {
			telemetry_rates.put(serial, new_telemetry_rate);
			backend.putInt(String.format(telemetryRatePreferenceFormat, serial), new_telemetry_rate);
			flush_preferences();
		}
	}

	public static int telemetry_rate(int serial) {
		synchronized (backend) {
			if (telemetry_rates.containsKey(serial))
				return telemetry_rates.get(serial);
			int telemetry_rate = backend.getInt(String.format(telemetryRatePreferenceFormat, serial),
							    AltosLib.ao_telemetry_rate_38400);
			telemetry_rates.put(serial, telemetry_rate);
			return telemetry_rate;
		}
	}

	public static void set_logfile(int serial, File new_logfile) {
		synchronized(backend) {
			logfiles.put(serial, new_logfile);
			backend.putString(String.format(logfilePreferenceFormat, serial), new_logfile.getPath());
			flush_preferences();
		}
	}

	public static File logfile(int serial) {
		synchronized(backend) {
			if (logfiles.containsKey(serial))
				return logfiles.get(serial);
			String logfile_string = backend.getString(String.format(logfilePreferenceFormat, serial), null);
			if (logfile_string == null)
				return null;
			File logfile = new File(logfile_string);
			logfiles.put(serial, logfile);
			return logfile;
		}
	}

	public static void set_state(AltosState state, int serial) {

		synchronized(backend) {
			backend.putJson(String.format(statePreferenceFormat, serial), new AltosJson(state));
			backend.putInt(statePreferenceLatest, serial);
			flush_preferences();
		}
	}

	public static ArrayList<Integer> list_states() {
		String[]		keys = backend.keys();
		ArrayList<Integer>	states = new ArrayList<Integer>();

		for (String key : keys) {
			if (key.startsWith(statePreferenceHead)) {
				try {
					int serial = AltosParse.parse_int(key.substring(statePreferenceHead.length()));
					states.add(serial);
				} catch (ParseException pe) {
				}
			}
		}
		return states;
	}

	public static void remove_state(int serial) {
		synchronized(backend) {
			backend.remove(String.format(statePreferenceFormat, serial));
			flush_preferences();
		}
	}

	public static int latest_state() {
		int	latest = 0;
		synchronized (backend) {
			latest = backend.getInt(statePreferenceLatest, 0);
		}
		return latest;
	}

	public static AltosState state(int serial) {
		synchronized(backend) {
			try {
				AltosJson json = backend.getJson(String.format(statePreferenceFormat, serial));
				if (json != null)
					return (AltosState) (json.make(AltosState.class));
			} catch (Exception e) {
			}
			return null;
		}
	}

	public static void set_scanning_telemetry(int new_scanning_telemetry) {
		synchronized (backend) {
			scanning_telemetry = new_scanning_telemetry;
			backend.putInt(scanningTelemetryPreference, scanning_telemetry);
			flush_preferences();
		}
	}

	public static int scanning_telemetry() {
		synchronized (backend) {
			return scanning_telemetry;
		}
	}

	public static void set_scanning_telemetry_rate(int new_scanning_telemetry_rate) {
		synchronized (backend) {
			scanning_telemetry_rate = new_scanning_telemetry_rate;
			backend.putInt(scanningTelemetryRatePreference, scanning_telemetry_rate);
			flush_preferences();
		}
	}

	public static int scanning_telemetry_rate() {
		synchronized(backend) {
			return scanning_telemetry_rate;
		}
	}

	public static void set_voice(boolean new_voice) {
		synchronized (backend) {
			voice = new_voice;
			backend.putBoolean(voicePreference, voice);
			flush_preferences();
		}
	}

	public static boolean voice() {
		synchronized (backend) {
			return voice;
		}
	}

	public static void set_callsign(String new_callsign) {
		synchronized(backend) {
			callsign = new_callsign;
			backend.putString(callsignPreference, callsign);
			flush_preferences();
		}
	}

	public static String callsign() {
		synchronized(backend) {
			return callsign;
		}
	}

	public static void set_firmwaredir(File new_firmwaredir) {
		synchronized (backend) {
			firmwaredir = new_firmwaredir;
			backend.putString(firmwaredirPreference, firmwaredir.getPath());
			flush_preferences();
		}
	}

	public static File firmwaredir() {
		synchronized (backend) {
			return firmwaredir;
		}
	}

	public static void set_launcher_serial(int new_launcher_serial) {
		synchronized (backend) {
			launcher_serial = new_launcher_serial;
			backend.putInt(launcherSerialPreference, launcher_serial);
			flush_preferences();
		}
	}

	public static int launcher_serial() {
		synchronized (backend) {
			return launcher_serial;
		}
	}

	public static void set_launcher_channel(int new_launcher_channel) {
		synchronized (backend) {
			launcher_channel = new_launcher_channel;
			backend.putInt(launcherChannelPreference, launcher_channel);
			flush_preferences();
		}
	}

	public static int launcher_channel() {
		synchronized (backend) {
			return launcher_channel;
		}
	}

	public static AltosPreferencesBackend bt_devices() {
		synchronized (backend) {
			return backend.node("bt_devices");
		}
	}

	public static AltosFrequency[] common_frequencies() {
		synchronized (backend) {
			return common_frequencies;
		}
	}

	public static void set_common_frequencies(AltosFrequency[] frequencies) {
		synchronized(backend) {
			common_frequencies = frequencies;

			save_common_frequencies();
		}
	}

	public static void add_common_frequency(AltosFrequency frequency) {
		AltosFrequency[]	old_frequencies = common_frequencies();
		AltosFrequency[]	new_frequencies = new AltosFrequency[old_frequencies.length + 1];
		int			i;

		for (i = 0; i < old_frequencies.length; i++) {
			if (frequency.frequency == old_frequencies[i].frequency)
				return;
			if (frequency.frequency < old_frequencies[i].frequency)
				break;
			new_frequencies[i] = old_frequencies[i];
		}
		new_frequencies[i] = frequency;
		for (; i < old_frequencies.length; i++)
			new_frequencies[i+1] = old_frequencies[i];
		set_common_frequencies(new_frequencies);
	}

	static LinkedList<AltosUnitsListener> units_listeners;

	public static boolean imperial_units() {
		synchronized(backend) {
			return AltosConvert.imperial_units;
		}
	}

	public static void set_imperial_units(boolean imperial_units) {
		synchronized (backend) {
			AltosConvert.imperial_units = imperial_units;
			backend.putBoolean(unitsPreference, imperial_units);
			flush_preferences();
		}
		if (units_listeners != null) {
			for (AltosUnitsListener l : units_listeners) {
				l.units_changed(imperial_units);
			}
		}
	}

	public static void register_units_listener(AltosUnitsListener l) {
		synchronized(backend) {
			if (units_listeners == null)
				units_listeners = new LinkedList<AltosUnitsListener>();
			units_listeners.add(l);
		}
	}

	public static void unregister_units_listener(AltosUnitsListener l) {
		synchronized(backend) {
			units_listeners.remove(l);
		}
	}


	public static void register_map_cache_listener(AltosMapCacheListener l) {
		synchronized(backend) {
			map_cache_listeners.add(l);
		}
	}

	public static void unregister_map_cache_listener(AltosMapCacheListener l) {
		synchronized (backend) {
			map_cache_listeners.remove(l);
		}
	}

	public static void set_map_cache(int new_map_cache) {
		synchronized(backend) {
			map_cache = new_map_cache;
			backend.putInt(mapCachePreference, map_cache);
			flush_preferences();
			for (AltosMapCacheListener l: map_cache_listeners)
				l.map_cache_changed(map_cache);
		}
	}

	public static int map_cache() {
		synchronized(backend) {
			return map_cache;
		}
	}

	static LinkedList<AltosMapTypeListener> map_type_listeners;

	public static void set_map_type(int map_type) {
		synchronized(backend) {
			AltosPreferences.map_type = map_type;
			backend.putInt(mapTypePreference, map_type);
			flush_preferences();
		}
		if (map_type_listeners != null) {
			for (AltosMapTypeListener l : map_type_listeners) {
				l.map_type_changed(map_type);
			}
		}
	}

	public static int map_type() {
		synchronized(backend) {
			return map_type;
		}
	}

	public static void register_map_type_listener(AltosMapTypeListener l) {
		synchronized(backend) {
			if (map_type_listeners == null)
				map_type_listeners = new LinkedList<AltosMapTypeListener>();
			map_type_listeners.add(l);
		}
	}

	public static void unregister_map_type_listener(AltosMapTypeListener l) {
		synchronized(backend) {
			map_type_listeners.remove(l);
		}
	}
}
