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

import java.net.*;
import java.io.*;
import java.text.*;
import java.util.*;
import java.util.concurrent.*;

import org.altusmetrum.altoslib_14.*;

public class AltosMapd implements AltosLaunchSiteListener {

	public static int port = 16717;

	public final static int maptype = AltosMap.maptype_hybrid;

	public final static int px_size = 512;

	public final static int scale = 1;

	public static int max_zoom = 17;

	public static double valid_radius = 17000;	/* 17km */

	public String map_dir = null;
	public String launch_sites_file = null;
	public String key_file = null;

	public void usage() {
		System.out.printf("usage: altos-mapd [--mapdir <map-directory] [--launch-sites <launch-sites-file>]\n" +
				  "                  [--radius <valid-radius-m> [--port <port>] [--key <key-file>]\n" +
				  "                  [--max-zoom <max-zoom-level>\n");
		System.exit(1);
	}

	private static Semaphore launch_sites_ready;

	private static List<AltosLaunchSite> launch_sites;

	public void notify_launch_sites(List<AltosLaunchSite> sites) {
		synchronized (launch_sites_ready) {
			if (sites != null) {
				launch_sites = sites;
				launch_sites_ready.release();
			}
		}
	}

	private static boolean west_of(double a, double b) {
		double	diff = b - a;

		while (diff >= 360.0)
			diff -= 360.0;
		while (diff <= -360.0)
			diff += 360.0;

		return diff >= 0;
	}

	public static boolean check_lat_lon(double lat, double lon, int zoom) {

		if (zoom > max_zoom)
			return false;

		AltosMapTransform	transform = new AltosMapTransform(px_size, px_size, zoom, new AltosLatLon(lat, lon));

		AltosLatLon		upper_left = transform.screen_lat_lon(new AltosPointInt(0, 0));
		AltosLatLon		lower_right = transform.screen_lat_lon(new AltosPointInt(px_size, px_size));

		synchronized (launch_sites_ready) {
			if (launch_sites == null) {
				try {
					launch_sites_ready.acquire();
				} catch (InterruptedException ie) {
					return false;
				}
			}
		}
		if (launch_sites == null) {
			System.out.printf("No launch site data available, refusing all requests\n");
			return false;
		}

		for (AltosLaunchSite site : launch_sites) {

			/* Figure out which point in the tile to
			 * measure to the site That's one of the edges
			 * or the site location depend on where the
			 * site is in relation to the tile
			 */

			double		check_lon;

			if (west_of(site.longitude, upper_left.lon))
				check_lon = upper_left.lon;
			else if (west_of(lower_right.lon, site.longitude))
				check_lon = lower_right.lon;
			else
				check_lon = site.longitude;

			double		check_lat;

			if (site.latitude < lower_right.lat)
				check_lat = lower_right.lat;
			else if (upper_left.lat < site.latitude)
				check_lat = upper_left.lat;
			else
				check_lat = site.latitude;

			AltosGreatCircle gc = new AltosGreatCircle(site.latitude, site.longitude,
								   check_lat, check_lon);

			if (gc.distance <= valid_radius)
				return true;
		}

		return false;
	}

	AltosMapdServer	server;

	public void process(String[] args) {

		AltosPreferences.init(new AltosMapdPreferences());

		int skip = 1;
		for (int i = 0; i < args.length; i += skip) {
			skip = 1;
			if (args[i].equals("--mapdir") && i < args.length-1) {
				map_dir = args[i+1];
				skip = 2;
			} else if (args[i].equals("--launch-sites") && i < args.length-1) {
				launch_sites_file = args[i+1];
				skip = 2;
			} else if (args[i].equals("--radius") && i < args.length-1) {
				try {
					valid_radius = AltosParse.parse_double_locale(args[i+1]);
				} catch (ParseException pe) {
					usage();
				}
				skip = 2;
			} else if (args[i].equals("--port") && i < args.length-1) {
				try {
					port = AltosParse.parse_int(args[i+1]);
				} catch (ParseException pe) {
					usage();
				}
				skip = 2;
			} else if (args[i].equals("--key") && i < args.length-1) {
				key_file = args[i+1];
				skip = 2;
			} else if (args[i].equals("--max-zoom") && i < args.length-1) {
				try {
					max_zoom = AltosParse.parse_int(args[i+1]);
				} catch (ParseException pe) {
					usage();
				}
				skip = 2;
			} else {
				usage();
			}
		}

		if (map_dir == null)
			usage();

		if (key_file != null) {
			try {
				BufferedReader key_reader = new BufferedReader(new FileReader(key_file));

				String line = key_reader.readLine();
				if (line == null || line.length() != 39) {
					System.out.printf("%s: invalid contents %d \"%s\"\n",
							  key_file, line.length(), line);
					usage();
				}
				key_reader.close();
				AltosMapStore.google_maps_api_key = line;
			} catch (Exception e) {
				System.out.printf("%s: %s\n", key_file, e.toString());
				usage();
			}
		}

		AltosPreferences.mapdir = new File(map_dir);

		if (launch_sites_file != null)
			AltosLaunchSites.launch_sites_url = "file://" + launch_sites_file;

		launch_sites_ready = new Semaphore(0);

		new AltosLaunchSites(this);

		try {
			server = new AltosMapdServer(port);
		} catch (IOException ie) {
			System.out.printf("Cannot bind to port %d: %s\n", port, ie.toString());
			usage();
		}

		for (;;) {
			try {
				Socket client = server.accept();
				if (client == null) {
					System.out.printf("accept failed\n");
					continue;
				}
				new AltosMapdClient(client);
			} catch (Exception e) {
				System.out.printf("Exception %s\n", e.toString());
			}
		}
	}

	public void AltosMapd() {
	}

	public static void main(final String[] args) {
		new AltosMapd().process(args);
	}
}
