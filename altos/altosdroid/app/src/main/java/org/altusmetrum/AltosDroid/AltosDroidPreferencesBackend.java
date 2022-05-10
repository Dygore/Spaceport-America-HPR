/*
 * Copyright © 2012 Mike Beattie <mike@ethernal.org>
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

package org.altusmetrum.AltosDroid;

import java.io.File;
import java.util.Map;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;
import android.util.*;

import org.altusmetrum.altoslib_14.*;

public class AltosDroidPreferencesBackend extends AltosPreferencesBackend {
	public final static String        NAME    = "org.altusmetrum.AltosDroid";
	private Context                   context = null;
	private SharedPreferences         prefs   = null;
	private SharedPreferences.Editor  editor  = null;

	public AltosDroidPreferencesBackend(Context in_context) {
		this(in_context, NAME);
	}

	public AltosDroidPreferencesBackend(Context in_context, String in_prefs) {
		context = in_context;
		prefs   = context.getSharedPreferences(in_prefs, 0);
		editor  = prefs.edit();
	}

	public String[] keys() {
		Map<String, ?> all = prefs.getAll();
		Object[] ao = all.keySet().toArray();

		String[] as = new String[ao.length];
		for (int i = 0; i < ao.length; i++)
			as[i] = (String) ao[i];
		return as;
	}

	public AltosPreferencesBackend node(String key) {
		if (!nodeExists(key))
			putBoolean(key, true);
		return new AltosDroidPreferencesBackend(context, key);
	}

	public boolean nodeExists(String key) {
		return prefs.contains(key);
	}

	public boolean getBoolean(String key, boolean def) {
		return prefs.getBoolean(key, def);
	}

	public double getDouble(String key, double def) {
		Float f = Float.valueOf(prefs.getFloat(key, (float)def));
		return f.doubleValue();
	}

	public int getInt(String key, int def) {
		return prefs.getInt(key, def);
	}

	public String getString(String key, String def) {
		String	ret;
		if (key.equals(AltosPreferences.logdirPreference))
			ret = null;
		else
			ret = prefs.getString(key, def);
		AltosDebug.debug("AltosDroidPreferencesBackend get string %s:\n", key);
		if (ret == null)
			AltosDebug.debug("      (null)\n");
		else {
			String[] lines = ret.split("\n");
			for (String l : lines)
				AltosDebug.debug("        %s\n", l);
		}
		return ret;
	}

	public byte[] getBytes(String key, byte[] def) {
		String save = prefs.getString(key, null);

		if (save == null)
			return def;

		byte[] bytes = Base64.decode(save, Base64.DEFAULT);
		return bytes;
	}

	public void putBoolean(String key, boolean value) {
		editor.putBoolean(key, value);
	}

	public void putDouble(String key, double value) {
		editor.putFloat(key, (float)value);
	}

	public void putInt(String key, int value) {
		editor.putInt(key, value);
	}

	public void putString(String key, String value) {
//		AltosDebug.debug("AltosDroidPreferencesBackend put string %s:\n", key);
//		String[] lines = value.split("\n");
//		for (String l : lines)
//			AltosDebug.debug("        %s\n", l);
		editor.putString(key, value);
	}

	public void putBytes(String key, byte[] bytes) {
		String save = Base64.encodeToString(bytes, Base64.DEFAULT);
		editor.putString(key, save);
	}

	public void remove(String key) {
		AltosDebug.debug("remove preference %s\n", key);
		editor.remove(key);
	}

	public void flush() {
		editor.apply();
	}

	public File homeDirectory() {
		return context.getExternalMediaDirs()[0];
	}

	public void debug(String format, Object ... arguments) {
		AltosDebug.debug(format, arguments);
	}
}
