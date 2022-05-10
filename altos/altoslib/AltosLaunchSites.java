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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.lang.*;
import java.util.*;
import java.util.concurrent.*;
import java.net.*;
import java.text.*;

public class AltosLaunchSites extends Thread {
	URL				url;
	LinkedList<AltosLaunchSite>	sites;
	AltosLaunchSiteListener		listener;

	public static String		launch_sites_url;

	void notify_complete() {
		listener.notify_launch_sites(sites);
	}

	void add(AltosLaunchSite site) {
		sites.add(site);
	}

	void add(String line) {
		try {
			add(new AltosLaunchSite(line));
		} catch (ParseException pe) {
			System.out.printf("parse exception %s\n", pe.toString());
		}
	}

	public void run() {
		try {
			String	path;

			if (launch_sites_url != null)
				path = launch_sites_url;
			else {
				path = System.getenv(AltosLib.launch_sites_env);
				if (path == null)
					path = AltosLib.launch_sites_url;
			}
			url = new URL(path);
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

	public AltosLaunchSites(AltosLaunchSiteListener listener) {
		sites = new LinkedList<AltosLaunchSite>();
		this.listener = listener;
		start();
	}
}
