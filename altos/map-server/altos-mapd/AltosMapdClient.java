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
import java.util.*;
import java.util.concurrent.*;
import java.io.*;

import org.altusmetrum.altoslib_14.*;

public class AltosMapdClient extends Thread implements AltosMapStoreListener {
	private Socket		socket;
	private AltosJson	request;
	private AltosJson	reply;

	private int		http_status;

	private void set_status(int status) {
		http_status = status;
		reply.put("status", status);
	}

	private void set_filename(String filename) {
		reply.put("filename", filename);

	}

	private void set_content_type(String content_type) {
		reply.put("content_type", content_type);
	}

	private String content_type(File file) {
		String content_type = "application/octet-stream";
		String basename = file.getName();
		if (basename.endsWith(".jpg"))
			content_type = "image/jpeg";
		if (basename.endsWith(".png"))
			content_type = "image/png";
		return content_type;
	}

	private void set_file(File file) {
		set_filename(file.getAbsolutePath());
		set_content_type(content_type(file));
	}

	private Semaphore store_ready;

	public void notify_store(AltosMapStore map_store, int status) {
		if (status != AltosMapTile.fetching)
			store_ready.release();
	}

	public void run() {
		reply = new AltosJson();
		try {
			request = AltosJson.fromInputStream(socket.getInputStream());

			if (request == null) {
				set_status(400);
				System.out.printf("client failed %d\n", http_status);
			} else {

				double	lat = request.get_double("lat", AltosLib.MISSING);
				double	lon = request.get_double("lon", AltosLib.MISSING);
				int	zoom = request.get_int("zoom", AltosLib.MISSING);
				String	addr = request.get_string("remote_addr", null);

				if (lat == AltosLib.MISSING ||
				    lon == AltosLib.MISSING ||
				    zoom == AltosLib.MISSING ||
				    addr == null)
				{
					set_status(400);
				} else if (!AltosMapd.check_lat_lon(lat, lon, zoom)) {
					set_status(403);	/* Forbidden */
				} else {

					store_ready = new Semaphore(0);

					AltosMapStore	map_store = AltosMapStore.get(new AltosLatLon(lat, lon),
										      zoom,
										      AltosMapd.maptype,
										      AltosMapd.px_size,
										      AltosMapd.scale);
					int status;

					if (map_store == null) {
						status = AltosMapTile.failed;
					} else {
						map_store.add_listener(this);

						try {
							store_ready.acquire();
						} catch (Exception ie) {
						}

						status = map_store.status();
					}

					if (status == AltosMapTile.fetched || status == AltosMapTile.loaded) {
						set_status(200);
						set_file(map_store.file);
					} else if (status == AltosMapTile.failed) {
						set_status(404);
					} else if (status == AltosMapTile.fetching) {
						set_status(408);
					} else if (status == AltosMapTile.bad_request) {
						set_status(400);
					} else if (status == AltosMapTile.forbidden) {
						set_status(403);
					} else {
						set_status(400);
					}
				}
				System.out.printf("%s: %.6f %.6f %d status %d\n",
						  addr, lat, lon, zoom, http_status);
			}
		} catch (Exception e) {
			System.out.printf("client exception %s\n", e.toString());
			e.printStackTrace(System.out);
			set_status(400);

		} finally {
			try {
				Writer writer = new PrintWriter(socket.getOutputStream());
				reply.write(writer);
				writer.write('\n');
				writer.flush();
			} catch (IOException ie) {
			}
			try {
				socket.close();
			} catch (IOException ie) {
			}
		}
	}

	public AltosMapdClient(Socket socket) {
		this.socket = socket;
		start();
	}
}
