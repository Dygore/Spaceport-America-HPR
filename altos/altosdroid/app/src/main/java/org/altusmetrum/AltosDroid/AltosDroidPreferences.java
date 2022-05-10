/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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

import java.util.*;

import android.content.Context;
import org.altusmetrum.altoslib_14.*;

public class AltosDroidPreferences extends AltosPreferences {

	/* Active device preference name */
	final static String activeDeviceAddressPreference = "ACTIVE-DEVICE-ADDRESS";
	final static String activeDeviceNamePreference = "ACTIVE-DEVICE-NAME";

	public static final int font_size_small = 0;
	public static final int font_size_medium = 1;
	public static final int font_size_large = 2;
	public static final int font_size_extra = 3;

	final static String fontSizePreference = "FONT-SIZE";

	static int font_size = font_size_medium;

	static DeviceAddress	active_device_address;

	/* Map source preference name */
	final static String mapSourcePreference = "MAP-SOURCE";

	static final int	MAP_SOURCE_OFFLINE = 0;
	static final int	MAP_SOURCE_ONLINE = 1;

	static int	map_source;

	/* Tracker sort selection */
	final static String trackerSortPreference = "TRACKER-SORT";

	static int	tracker_sort;

	public static void init(Context context) {
		if (backend != null)
			return;

		AltosPreferences.init(new AltosDroidPreferencesBackend(context));

		font_size = backend.getInt(fontSizePreference, font_size_medium);

		String address = backend.getString(activeDeviceAddressPreference, null);
		String name = backend.getString(activeDeviceNamePreference, null);

		if (address != null && name != null)
			active_device_address = new DeviceAddress (address, name);

		map_source = backend.getInt(mapSourcePreference, MAP_SOURCE_ONLINE);

		tracker_sort = backend.getInt(trackerSortPreference, 0);
	}

	public static void set_active_device(DeviceAddress address) {
		if (backend == null)
			return;
		synchronized(backend) {
			active_device_address = address;
			if (active_device_address != null) {
				backend.putString(activeDeviceAddressPreference, active_device_address.address);
				backend.putString(activeDeviceNamePreference, active_device_address.name);
			} else {
				backend.remove(activeDeviceAddressPreference);
				backend.remove(activeDeviceNamePreference);
			}
			flush_preferences();
		}
	}

	public static DeviceAddress active_device() {
		if (backend == null)
			return null;

		synchronized(backend) {
			return active_device_address;
		}
	}

	static LinkedList<AltosDroidMapSourceListener> map_source_listeners;

	public static void set_map_source(int map_source) {
		if (backend == null)
			return;
		synchronized(backend) {
			AltosDroidPreferences.map_source = map_source;
			backend.putInt(mapSourcePreference, map_source);
			flush_preferences();
		}
		if (map_source_listeners != null) {
			for (AltosDroidMapSourceListener l : map_source_listeners) {
				l.map_source_changed(map_source);
			}
		}
	}

	public static int map_source() {
		if (backend == null)
			return MAP_SOURCE_ONLINE;
		synchronized(backend) {
			return map_source;
		}
	}

	public static void register_map_source_listener(AltosDroidMapSourceListener l) {
		if (backend == null)
			return;

		synchronized(backend) {
			if (map_source_listeners == null)
				map_source_listeners = new LinkedList<AltosDroidMapSourceListener>();
			map_source_listeners.add(l);
		}
	}

	public static void unregister_map_source_listener(AltosDroidMapSourceListener l) {
		if (backend == null)
			return;
		synchronized(backend) {
			map_source_listeners.remove(l);
		}
	}

	public static int font_size() {
		if (backend == null)
			return font_size_medium;

		synchronized (backend) {
			return font_size;
		}
	}

	public static void set_font_size(int new_font_size) {
		if (backend == null)
			return;
		synchronized (backend) {
			if (font_size != new_font_size) {
				font_size = new_font_size;
				backend.putInt(fontSizePreference, font_size);
				flush_preferences();
			}
		}
	}


	public static int tracker_sort() {
		if (backend == null)
			return 0;

		synchronized(backend) {
			return tracker_sort;
		}
	}

	public static void set_tracker_sort(int new_tracker_sort) {
		if (backend == null)
			return;

		synchronized(backend) {
			if (tracker_sort != new_tracker_sort) {
				tracker_sort = new_tracker_sort;
				backend.putInt(trackerSortPreference, tracker_sort);
				flush_preferences();
			}
		}
	}
}
