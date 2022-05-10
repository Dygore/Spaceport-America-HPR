/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

package altosmapd;

import java.io.*;

import org.altusmetrum.altoslib_14.*;

public class AltosMapdPreferences extends AltosPreferencesBackend {

	public String  getString(String key, String def) {
		return def;
	}
	public void    putString(String key, String value) {
	}

	public int     getInt(String key, int def) {
		return def;
	}

	public void    putInt(String key, int value) {
	}

	public double  getDouble(String key, double def) {
		return def;
	}

	public void    putDouble(String key, double value) {
	}

	public boolean getBoolean(String key, boolean def) {
		return def;
	}

	public void    putBoolean(String key, boolean value) {
	}

	public byte[]  getBytes(String key, byte[] def) {
		return def;
	}

	public void    putBytes(String key, byte[] value) {
	}

	public boolean nodeExists(String key) {
		return false;
	}

	public AltosPreferencesBackend node(String key) {
		return this;
	}

	public String[] keys() {
		return null;
	}

	public void    remove(String key) {
	}

	public void    flush() {
	}

	public File homeDirectory() {
		return new File (".");
	}

	public void debug(String format, Object ... arguments) {
		System.out.printf(format, arguments);
	}

	public AltosMapdPreferences() {
	}
}
