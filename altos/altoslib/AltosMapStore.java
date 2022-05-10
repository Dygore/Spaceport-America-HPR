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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.net.*;
import java.util.*;

public class AltosMapStore {
	String					url;
	public File				file;
	LinkedList<AltosMapStoreListener>	listeners = new LinkedList<AltosMapStoreListener>();

	int					status;

	private static File map_file(AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		double lat = center.lat;
		double lon = center.lon;
		char chlat = lat < 0 ? 'S' : 'N';
		char chlon = lon < 0 ? 'W' : 'E';

		if (lat < 0) lat = -lat;
		if (lon < 0) lon = -lon;
		String maptype_string = String.format("%s-", AltosMap.maptype_names[maptype]);
		String format_string;
		if (maptype == AltosMap.maptype_hybrid || maptype == AltosMap.maptype_satellite || maptype == AltosMap.maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png";
		return new File(AltosPreferences.mapdir(),
				String.format("map-%c%.6f,%c%.6f-%s%d%s.%s",
					      chlat, lat, chlon, lon, maptype_string, zoom, scale == 1 ? "" : String.format("-%d", scale), format_string));
	}

	public static String google_maps_api_key = null;

	private static String google_map_url(AltosLatLon center, int zoom, int maptype, int px_size, int scale, String format_string) {
		return String.format("http://maps.google.com/maps/api/staticmap?center=%.6f,%.6f&zoom=%d&size=%dx%d&scale=%d&sensor=false&maptype=%s&format=%s&key=%s",
				     center.lat, center.lon, zoom, px_size, px_size, scale,
				     AltosMap.maptype_names[maptype], format_string, google_maps_api_key);
	}

	private static String altos_map_url(AltosLatLon center, int zoom, int maptype, int px_size, int scale, String format_string) {
		return String.format("https://maps.altusmetrum.org/cgi-bin/altos-map?lat=%.6f&lon=%.6f&zoom=%d",
				     center.lat, center.lon, zoom);
	}

	private static String map_url(AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		String format_string;

		if (maptype == AltosMap.maptype_hybrid || maptype == AltosMap.maptype_satellite || maptype == AltosMap.maptype_terrain)
			format_string = "jpg";
		else
			format_string = "png32";

		for (int s = 1; s < scale; s <<= 1)
			zoom--;

		px_size /= scale;

		if (google_maps_api_key != null)
			return google_map_url(center, zoom, maptype, px_size, scale, format_string);
		else
			return altos_map_url(center, zoom, maptype, px_size, scale, format_string);
	}

	public synchronized int status() {
		return status;
	}

	public synchronized void add_listener(AltosMapStoreListener listener) {
		if (!listeners.contains(listener))
			listeners.add(listener);
		listener.notify_store(this, status);
	}

	public synchronized void remove_listener(AltosMapStoreListener listener) {
		listeners.remove(listener);
	}

	private synchronized void notify_listeners(int status) {
		this.status = status;
		for (AltosMapStoreListener listener : listeners)
			listener.notify_store(this, status);
	}

	private int fetch_url() {
		URL u;

		try {
			u = new URL(url);
		} catch (java.net.MalformedURLException e) {
			return AltosMapTile.bad_request;
		}

		byte[] data = null;
		URLConnection uc = null;

		int status = AltosMapTile.failed;
		int tries = 0;

		while (tries < 10 && status != AltosMapTile.fetched) {
			try {
				uc = u.openConnection();
				String type = uc.getContentType();
				int contentLength = uc.getContentLength();
				if (uc instanceof HttpURLConnection) {
					int response = ((HttpURLConnection) uc).getResponseCode();
					switch (response) {
					case HttpURLConnection.HTTP_FORBIDDEN:
					case HttpURLConnection.HTTP_PAYMENT_REQUIRED:
					case HttpURLConnection.HTTP_UNAUTHORIZED:
						return AltosMapTile.forbidden;
					}
				}
				InputStream in = new BufferedInputStream(uc.getInputStream());
				int bytesRead = 0;
				int offset = 0;
				data = new byte[contentLength];
				while (offset < contentLength) {
					bytesRead = in.read(data, offset, data.length - offset);
					if (bytesRead == -1)
						break;
					offset += bytesRead;
				}
				in.close();

				if (offset == contentLength)
					status = AltosMapTile.fetched;
				else
					status = AltosMapTile.failed;

			} catch (IOException e) {
				status = AltosMapTile.failed;
			}

			if (status != AltosMapTile.fetched) {
				try {
					Thread.sleep(100);
				} catch (InterruptedException ie) {
				}
				tries++;
				System.out.printf("Fetch failed, retrying %d\n", tries);
			}
		}

		if (status != AltosMapTile.fetched)
			return status;

		try {
			FileOutputStream out = new FileOutputStream(file);
			if (data != null)
				out.write(data);
			out.flush();
			out.close();
		} catch (FileNotFoundException e) {
			return AltosMapTile.bad_request;
		} catch (IOException e) {
			if (file.exists())
				file.delete();
			return AltosMapTile.bad_request;
		}
		return AltosMapTile.fetched;
	}

	static Object	fetch_lock = new Object();

	static Object	fetcher_lock = new Object();

	static LinkedList<AltosMapStore> waiting = new LinkedList<AltosMapStore>();
	static LinkedList<AltosMapStore> running = new LinkedList<AltosMapStore>();

	static int concurrent_fetchers() {
		if (google_maps_api_key == null)
			return 16;
		return 128;
	}

	static void start_fetchers() {
		while (!waiting.isEmpty() && running.size() < concurrent_fetchers()) {
			AltosMapStore 	s = waiting.remove();
			running.add(s);
			Thread lt = s.make_fetcher_thread();
			lt.start();
		}
	}

	void finish_fetcher() {
		synchronized(fetcher_lock) {
			running.remove(this);
			start_fetchers();
		}
	}

	void add_fetcher() {
		synchronized(fetcher_lock) {
			waiting.add(this);
			start_fetchers();
		}
	}

	class fetcher implements Runnable {

		public void run() {
			try {
				if (file.exists()) {
					notify_listeners(AltosMapTile.fetched);
					return;
				}

				int new_status;

				new_status = fetch_url();

				notify_listeners(new_status);
			} finally {
				finish_fetcher();
			}
		}
	}

	private Thread make_fetcher_thread() {
		return new Thread(new fetcher());
	}

	private void fetch() {
		add_fetcher();
	}

	private AltosMapStore (String url, File file) {
		this.url = url;
		this.file = file;

		if (file.exists())
			status = AltosMapTile.fetched;
		else {
			status = AltosMapTile.fetching;
			fetch();
		}
	}

	public int hashCode() {
		return url.hashCode();
	}

	public boolean equals(Object o) {
		if (o == null)
			return false;

		if (!(o instanceof AltosMapStore))
			return false;

		AltosMapStore other = (AltosMapStore) o;
		return url.equals(other.url);
	}

	static HashMap<String,AltosMapStore> stores = new HashMap<String,AltosMapStore>();

	public static AltosMapStore get(AltosLatLon center, int zoom, int maptype, int px_size, int scale) {
		String url = map_url(center, zoom, maptype, px_size, scale);

		AltosMapStore	store;
		synchronized(stores) {
			if (stores.containsKey(url)) {
				store = stores.get(url);
			} else {
				store = new AltosMapStore(url, map_file(center, zoom, maptype, px_size, scale));
				stores.put(url, store);
			}
		}
		return store;
	}

}
