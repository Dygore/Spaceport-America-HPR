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

package altosmap;

import java.net.*;
import java.io.*;
import java.util.*;
import java.text.*;

import org.altusmetrum.altoslib_14.*;

public class AltosMap {

	public final static int port = 16717;

	public final static String protocol_version = "1.0.0";

	String	query_string;
	String	remote_addr;

	public String reason_string(int code) {
		switch (code) {
		case 200:
			return "OK";
		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 408:
			return "Request Timeout";
		default:
			return "Failure";
		}
	}

	public void write_status(int status) {
		System.out.printf("Status: %d %s\n", status, reason_string(status));
	}

	public void write_type(String type) {
		System.out.printf("Content-Type: %s\n", type);
	}

	public void fail(int status, String reason) {
		write_status(status);
		write_type("text/html");
		System.out.printf("\n");
		System.out.printf("<html>\n");
		System.out.printf("<head><title>Map Fetch Failure</title></head>\n");
		System.out.printf("<body>%s</body>\n", reason);
		System.out.printf("</html>\n");
		System.exit(1);
	}

	public void process() {
		query_string = System.getenv("QUERY_STRING");

		if (query_string == null)
			fail(400, "Missing query string");

		remote_addr = System.getenv("REMOTE_ADDR");

		if (remote_addr == null)
			fail(400, "Missing remote address");

		String[] queries = query_string.split("&");

		double	lon = AltosLib.MISSING;
		double	lat = AltosLib.MISSING;
		int	zoom = AltosLib.MISSING;
		String	version = null;

		try {
			for (String query : queries) {
				String[] q = query.split("=");
				if (q.length >= 2) {
					String name = q[0];
					String value = q[1];
					if (name.equals("lon"))
						lon = AltosParse.parse_double_net(value);
					else if (name.equals("lat"))
						lat = AltosParse.parse_double_net(value);
					else if (name.equals("zoom"))
						zoom = AltosParse.parse_int(value);
					else if (name.equals("version"))
						version = value;
					else
						fail(400, String.format("Extra query param \"%s\"", query));
				}
			}
		} catch (ParseException pe) {
			fail(400, String.format("Invalid query: %s", pe.toString()));
		}

		if (version != null) {
			System.out.printf("Content-Type: text/plain\n");
			System.out.printf("\n");
			System.out.printf("%s\n", protocol_version);
		} else {
			if (lon == AltosLib.MISSING)
				fail(400, "Missing longitude");
			if (lat == AltosLib.MISSING)
				fail(400, "Missing latitude");
			if (zoom == AltosLib.MISSING)
				fail(400, "Missing zoom");

			try {
				Socket	socket = null;
				int tries = 0;

				while (tries < 10 && socket == null) {
					try {
						socket = new Socket(InetAddress.getLoopbackAddress(), port);
					} catch (IOException ie) {
						Thread.sleep(100);
						tries++;
					}
				}

				AltosJson	request = new AltosJson();

				request.put("lat", lat);
				request.put("lon", lon);
				request.put("zoom", zoom);
				request.put("remote_addr", remote_addr);

				Writer writer = new PrintWriter(socket.getOutputStream());
				request.write(writer);
				writer.flush();

				AltosJson	reply = AltosJson.fromInputStream(socket.getInputStream());

				int status = reply.get_int("status", 400);

				if (status != 200)
					fail(status, "Bad cache status");

				String filename = reply.get_string("filename", null);
				try {
					File file = new File(filename);
					long length = file.length();
					FileInputStream in = new FileInputStream(file);
					String content_type = reply.get_string("content_type", null);
					System.out.printf("Content-Type: %s\n", content_type);
					System.out.printf("Content-Length: %d\n", file.length());
					System.out.printf("\n");
					byte[] buf = new byte[4096];
					int bytes_read;
					while ((bytes_read = in.read(buf)) > 0)
						System.out.write(buf);
				} catch (IOException ie) {
					fail(404, String.format("IO Exception: %s", ie.toString()));
				}
			} catch (Exception e) {
				fail(404, String.format("Exception %s", e.toString()));
			}
		}
	}

	public AltosMap() {
	}

	public static void main(final String[] args) {

		new AltosMap().process();

	}
}
