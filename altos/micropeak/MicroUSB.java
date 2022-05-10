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

package org.altusmetrum.micropeak;

import java.util.*;
import libaltosJNI.*;
import org.altusmetrum.altoslib_14.*;
import org.altusmetrum.altosuilib_14.*;

public class MicroUSB extends altos_device implements AltosDevice {

	static boolean	initialized = false;
	static boolean	loaded_library = false;

	public static boolean load_library() {
		if (!initialized) {
			try {
				System.loadLibrary("altos");
				libaltos.altos_init();
				loaded_library = true;
			} catch (UnsatisfiedLinkError e) {
				try {
					System.loadLibrary("altos64");
					libaltos.altos_init();
					loaded_library = true;
				} catch (UnsatisfiedLinkError e2) {
					loaded_library = false;
				}
			}
			initialized = true;
		}
		return loaded_library;
	}

	public String toString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%-24.24s %s",
				     name, getPath());
	}

	public String toShortString() {
		String	name = getName();
		if (name == null)
			name = "Altus Metrum";
		return String.format("%s %s",
				     name, getPath());

	}

	public String getErrorString() {
		altos_error	error = new altos_error();

		libaltos.altos_get_last_error(error);
		return String.format("%s (%d)", error.getString(), error.getCode());
	}

	public SWIGTYPE_p_altos_file open() {
		return libaltos.altos_open(this);
	}

	private boolean isFTDI() {
		int vid = getVendor();
		int pid = getProduct();
		if (vid == 0x0403 && pid == 0x6015)
			return true;
		return false;
	}

	private boolean isMicro() {
		int vid = getVendor();
		int pid = getProduct();
		if (vid == AltosLib.vendor_altusmetrum &&
		    pid == AltosLib.product_mpusb)
			return true;
		return false;
	}

	public boolean matchProduct(int product) {
		return isFTDI() || isMicro();
	}

	public int hashCode() {
		return getVendor() ^ getProduct() ^ getSerial() ^ getPath().hashCode();
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;

		if (!(o instanceof MicroUSB))
			return false;

		MicroUSB other = (MicroUSB) o;

		return getVendor() == other.getVendor() &&
			getProduct() == other.getProduct() &&
			getSerial() == other.getSerial() &&
			getPath().equals(other.getPath());
	}

	static java.util.List<MicroUSB> list() {
		if (!load_library())
			return null;

		ArrayList<MicroUSB> device_list = new ArrayList<MicroUSB>();

		SWIGTYPE_p_altos_list list;

		list = libaltos.altos_ftdi_list_start();

		if (list != null) {
			for (;;) {
				MicroUSB device = new MicroUSB();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				if (device.isFTDI())
					device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		list = libaltos.altos_list_start();

		if (list != null) {
			for (;;) {
				MicroUSB device = new MicroUSB();
				if (libaltos.altos_list_next(list, device) == 0)
					break;
				if (device.isMicro())
					device_list.add(device);
			}
			libaltos.altos_list_finish(list);
		}

		return device_list;
	}
}
