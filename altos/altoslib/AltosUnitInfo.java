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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.lang.*;
import java.util.*;
import java.util.concurrent.*;
import java.net.*;
import java.text.*;

public class AltosUnitInfo extends Thread {
	int			sn;
	int			rfcal;
	AltosUnitInfoListener	listener;
	String			json_string;

	public int sn() {
		return sn;
	}

	public int rfcal() {
		return rfcal;
	}

	void add(String line) {
		if (json_string == null) {
			json_string = line;
		} else {
			json_string = json_string + "\n" + line;
		}
	}

	void notify_complete() {
		rfcal = AltosLib.MISSING;

		if (json_string != null) {
			System.out.printf("json_string: %s\n", json_string);
			AltosJson json = AltosJson.fromString(json_string);
			System.out.printf("json: %s\n", json);
			String rfcal_string = null;
			try {
				AltosJson unitinfo = json.get("unitinfo");
				rfcal_string = unitinfo.get_string("rfcal", null);
				if (rfcal_string != null)
					rfcal = Integer.parseInt(rfcal_string);
			} catch (NumberFormatException ne) {
				System.out.printf("mal-formed integer %s\n", rfcal_string);
			} catch (IllegalArgumentException ie) {
				System.out.printf("mal-formed json\n");
			}
		}
		listener.notify_unit_info(this);
	}

	public void run() {
		try {
			String	format;

			format = System.getenv(AltosLib.unit_info_env);
			if (format == null)
				format = AltosLib.unit_info_url;

			String path = String.format(format, sn);

			URL url = new URL(path);

			System.out.printf("URL: %s\n", path);

			URLConnection uc = url.openConnection();

			InputStreamReader in_stream = new InputStreamReader(uc.getInputStream(), AltosLib.unicode_set);
			BufferedReader in = new BufferedReader(in_stream);

			for (;;) {
				String line = in.readLine();
				if (line == null)
					break;
				add(line);
			}
		} catch (Exception e) {
			System.out.printf("file exception %s\n", e.toString());
		} finally {
			notify_complete();
		}
	}

	public AltosUnitInfo(int sn, AltosUnitInfoListener listener) {
		this.listener = listener;
		this.sn = sn;
		this.rfcal = AltosLib.MISSING;
		start();
	}
}
