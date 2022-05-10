/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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

import java.util.*;
import java.io.*;

public class AltosEeprom {

	private AltosJson	config;
	ArrayList<Byte>		data;
	private AltosConfigData	config_data;
	int			errors = 0;

	/*
	 * Public accessor APIs
	 */
	public int data8(int offset) {
		return ((int) data.get(offset)) & 0xff;
	}

	public int data16(int offset) {
		return data8(offset) | (data8(offset+1) << 8);
	}

	public int data24(int offset) {
		return (data8(offset) |
			(data8(offset+1) << 8) |
			(data8(offset+2) << 16));
	}

	public int data32(int offset) {
		return (data8(offset) |
			(data8(offset+1) << 8) |
			(data8(offset+2) << 16) |
			(data8(offset+3) << 24));
	}

	public int size() {
		return data.size();
	}

	public AltosConfigData config_data() {
		if (config_data == null) {
			config_data = (AltosConfigData) config.make(AltosConfigData.class);
			if (config_data == null)
				config_data = new AltosConfigData();

			if (config_data.log_format == AltosLib.AO_LOG_FORMAT_UNKNOWN) {
				config_data.log_format = AltosLib.AO_LOG_FORMAT_FULL;
				if (config_data.product != null) {
					if (config_data.product.startsWith("TeleMetrum"))
						config_data.log_format = AltosLib.AO_LOG_FORMAT_FULL;
					else if (config_data.product.startsWith("TeleMini"))
						config_data.log_format = AltosLib.AO_LOG_FORMAT_TINY;
				}
			}
		}
		return config_data;
	}

	private void write_config(Writer w) throws IOException {
		config.write(w, 0, true);
		w.append('\n');
	}

	/*
	 * Private I/O APIs
	 */
	private void write_data(Writer w) throws IOException {
		PrintWriter pw = new PrintWriter(w);

		for (int i = 0; i < data.size(); i++) {
			if (i > 0) {
				if ((i & 0x1f) == 0)
					pw.printf("\n");
				else
					pw.printf(" ");
			}
			pw.printf("%02x", data.get(i));
		}
		w.append('\n');
	}

	private boolean read_config(InputStream stream) throws IOException {
		config = AltosJson.fromInputStream(stream);
		if (config == null)
			return false;
		return true;
	}

	private String read_line(InputStream stream) throws IOException {
		StringBuffer	buffer = null;
		int		c;

		for (;;) {
			c = stream.read();
			if (c == -1 && buffer == null)
				return null;
			if (buffer == null)
				buffer = new StringBuffer();
			if (c == -1 || c == '\n')
				return buffer.toString();
			buffer.append((char) c);
		}
	}

	private boolean read_data(InputStream stream) throws IOException {
		String			s;

		data = new ArrayList<Byte>();
		while ((s = read_line(stream)) != null) {

			String[] tokens = s.split("\\s+");

			for (int i = 0; i < tokens.length; i++) {
				if (tokens[i].length() > 0) {
					try {
						data.add((byte) AltosLib.fromhex(tokens[i]));
					} catch (NumberFormatException e) {
						throw new IOException(e.toString());
					}
				}
			}
		}
		return true;
	}

	private boolean read_old_config(InputStream stream) throws IOException {
		AltosConfigData	cfg = new AltosConfigData();
		for (;;) {
			boolean	done = false;

			/* The data starts with an upper case F character followed by a space */
			stream.mark(2);
			int	first = stream.read();
			if (first == 'F') {
				int second =  stream.read();
				if (second == ' ')
					done = true;
			}
			stream.reset();
			if (done)
				break;

			String line = read_line(stream);
			if (line == null)
				return false;
			cfg.parse_line(line);
		}
		config = new AltosJson(cfg);
		return true;
	}

	private boolean read_old_data(InputStream stream) throws IOException {
		String line;

		data = new ArrayList<Byte>();
		while ((line = read_line(stream)) != null) {
			String[] tokens = line.split("\\s+");

			/* Make sure there's at least a type and time */
			if (tokens.length < 2)
				break;

			/* packet type */
			if (tokens[0].length() != 1)
				break;
			int start = data.size();

			if (config_data().log_format != AltosLib.AO_LOG_FORMAT_TINY) {
				byte cmd = (byte) tokens[0].codePointAt(0);
				data.add(cmd);

				int time = AltosLib.fromhex(tokens[1]);

				data.add((byte) 0);
				data.add((byte) (time & 0xff));
				data.add((byte) (time >> 8));
			}
			if (tokens.length == 4) {
				/* Handle ancient log files */
				if (config_data().log_format == AltosLib.AO_LOG_FORMAT_TINY) {
					/*
					 * Ancient TeleMini log files stored "extra" data to pretend
					 * that it was a TeleMetrum device. Throw that away and
					 * just save the actual log data.
					 */
					int a = AltosLib.fromhex(tokens[2]);
					int b = AltosLib.fromhex(tokens[3]);
					if (a != 0)
						b = 0x8000 | a;
					data.add((byte) (b & 0xff));
					data.add((byte) ((b >> 8)));
				} else {
					for (int i = 2; i < tokens.length; i++) {
						int v = AltosLib.fromhex(tokens[i]);
						data.add((byte) (v & 0xff));
						data.add((byte) ((v >> 8)));
					}
					/* Re-compute the checksum byte */
					data.set(start + 1, (byte) (256 - AltosConvert.checksum(data, start, data.size() - start)));
				}
			} else {
				for (int i = 2; i < tokens.length; i++)
					data.add((byte) AltosLib.fromhex(tokens[i]));
				/* Re-compute the checksum byte */
				data.set(start + 1, (byte) (256 - AltosConvert.checksum(data, start, data.size() - start)));
			}
		}
		return true;
	}

	private void read(InputStream stream) throws IOException {
		BufferedInputStream	bis = new BufferedInputStream(stream);

		bis.mark(1);
		int c = bis.read();
		bis.reset();

		if (c == '{') {
			if (!read_config(bis))
				throw new IOException("failed to read config");
			if (!read_data(bis))
				throw new IOException("failed to read data");
		} else {
			if (!read_old_config(bis))
				throw new IOException("failed to read old config");
			if (!read_old_data(bis))
				throw new IOException("failed to read old data");
		}
	}

	/*
	 * Public APIs for I/O
	 */
	public void write(Writer w) throws IOException {
		write_config(w);
		write_data(w);
	}

	public String toString() {
		try {
			Writer	w = new StringWriter();

			write(w);
			return w.toString();
		} catch (Exception e) {
			return null;
		}
	}

	public void print() throws IOException {
		System.out.printf("%s", toString());
	}

	/*
	 * Constructors
	 */
	public AltosEeprom(InputStream stream) throws IOException {
		read(stream);
	}

	public AltosEeprom(String s) throws IOException {
		read(new AltosStringInputStream(s));
	}

	public AltosEeprom(AltosJson config, ArrayList<Byte> data) {
		this.config = config;
		this.data = data;
	}

	public AltosEeprom(AltosConfigData config_data, ArrayList<Byte> data) {
		this.config = new AltosJson(config_data);
		this.data = data;
	}

	public AltosEeprom() {
	}
}
