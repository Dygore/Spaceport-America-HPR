/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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

import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;

import android.content.Context;
import android.hardware.usb.*;
import android.app.*;
import android.os.Handler;

import org.altusmetrum.altoslib_14.*;

public class AltosUsb extends AltosDroidLink {

	private Thread           input_thread   = null;

	private Handler          handler;

	private UsbManager		manager;
	private UsbDevice		device;
	private UsbDeviceConnection	connection;
	private UsbInterface 		iface;
	private UsbEndpoint		in, out;

	private InputStream      input;
	private OutputStream     output;

	// Constructor
	public AltosUsb(Context context, UsbDevice device, Handler handler) {
		super(handler);
//		set_debug(D);
		this.handler = handler;

		iface = null;
		in = null;
		out = null;

		int	niface = device.getInterfaceCount();

		for (int i = 0; i < niface; i++) {

			iface = device.getInterface(i);

			in = null;
			out = null;

			int nendpoints = iface.getEndpointCount();

			for (int e = 0; e < nendpoints; e++) {
				UsbEndpoint	endpoint = iface.getEndpoint(e);

				if (endpoint.getType() == UsbConstants.USB_ENDPOINT_XFER_BULK) {
					switch (endpoint.getDirection()) {
					case UsbConstants.USB_DIR_OUT:
						out = endpoint;
						break;
					case UsbConstants.USB_DIR_IN:
						in = endpoint;
						break;
					}
				}
			}

			if (in != null && out != null)
				break;
		}

		if (in != null && out != null) {
			AltosDebug.debug("\tin %s out %s\n", in.toString(), out.toString());

			manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);

			if (manager == null) {
				AltosDebug.debug("USB_SERVICE failed");
				return;
			}

			connection = manager.openDevice(device);

			if (connection == null) {
				AltosDebug.debug("openDevice failed");
				return;
			}

			connection.claimInterface(iface, true);

			input_thread = new Thread(this);
			input_thread.start();

			// Configure the newly connected device for telemetry
			print("~\nE 0\n");
			set_monitor(false);
		}
	}

	static private boolean isAltusMetrum(UsbDevice device) {
		if (device.getVendorId() != AltosLib.vendor_altusmetrum)
			return false;
		if (device.getProductId() < AltosLib.product_altusmetrum_min)
			return false;
		if (device.getProductId() > AltosLib.product_altusmetrum_max)
			return false;
		return true;
	}

	static boolean matchProduct(int want_product, UsbDevice device) {

		if (!isAltusMetrum(device))
			return false;

		if (want_product == AltosLib.product_any)
			return true;

		int have_product = device.getProductId();

		if (want_product == AltosLib.product_basestation)
			return have_product == AltosLib.product_teledongle ||
				have_product == AltosLib.product_telebt ||
				have_product == AltosLib.product_megadongle;

		if (want_product == AltosLib.product_altimeter)
			return have_product == AltosLib.product_telemetrum ||
				have_product == AltosLib.product_telemega ||
				have_product == AltosLib.product_easymega ||
				have_product == AltosLib.product_telegps ||
				have_product == AltosLib.product_easymini ||
				have_product == AltosLib.product_telemini ||
				have_product == AltosLib.product_easytimer;

		if (have_product == AltosLib.product_altusmetrum)	/* old devices match any request */
			return true;

		if (want_product == have_product)
			return true;

		return false;
	}

	static public boolean request_permission(Context context, UsbDevice device, PendingIntent pi) {
		UsbManager	manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);

//		if (manager.hasPermission(device))
//			return true;

		AltosDebug.debug("request permission for USB device " + device.toString());

		manager.requestPermission(device, pi);
		return false;
	}

	static public UsbDevice find_device(Context context, int match_product) {
		UsbManager	manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);

		HashMap<String,UsbDevice>	devices = manager.getDeviceList();

		for (UsbDevice	device : devices.values()) {
			int	vendor = device.getVendorId();
			int	product = device.getProductId();

			if (matchProduct(match_product, device)) {
				AltosDebug.debug("found USB device " + device.toString());
				return device;
			}
		}

		return null;
	}

	private void disconnected() {
		if (closed()) {
			AltosDebug.debug("disconnected after closed");
			return;
		}

		AltosDebug.debug("Sending disconnected message");
		handler.obtainMessage(TelemetryService.MSG_DISCONNECTED, this).sendToTarget();
	}

	void close_device() {
		UsbDeviceConnection	tmp_connection;

		synchronized(this) {
			tmp_connection = connection;
			connection = null;
		}

		if (tmp_connection != null) {
			AltosDebug.debug("Closing USB device");
			tmp_connection.close();
		}
	}

	int read(byte[] buffer, int len) {
		if (connection == null)
			return 0;
		int ret = connection.bulkTransfer(in, buffer, len, -1);
		AltosDebug.debug("read(%d) = %d\n", len, ret);
		return ret;
	}

	int write(byte[] buffer, int len) {
		if (connection == null)
			return 0;
		int ret = connection.bulkTransfer(out, buffer, len, -1);
		AltosDebug.debug("write(%d) = %d\n", len, ret);
		return ret;
	}

	// Stubs of required methods when extending AltosLink
	public boolean can_cancel_reply()   { return false; }
	public boolean show_reply_timeout() { return true; }
	public void hide_reply_timeout()    { }

}
