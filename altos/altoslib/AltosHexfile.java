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

package org.altusmetrum.altoslib_14;

import java.io.*;
import java.util.LinkedList;
import java.util.Arrays;

class HexFileInputStream extends PushbackInputStream {
	public int line;

	public HexFileInputStream(FileInputStream o) {
		super(new BufferedInputStream(o));
		line = 1;
	}

	public int read() throws IOException {
		int	c = super.read();
		if (c == '\n')
			line++;
		return c;
	}

	public void unread(int c) throws IOException {
		if (c == '\n')
			line--;
		if (c != -1)
			super.unread(c);
	}
}

class HexRecord implements Comparable<Object> {
	public long	address;
	public int	type;
	public byte	checksum;
	public byte[]	data;

	static final int NORMAL = 0;
	static final int EOF = 1;
	static final int EXTENDED_ADDRESS = 2;

	enum read_state {
		marker,
		length,
		address,
		type,
		data,
		checksum,
		newline,
		white,
		done,
	}

	boolean ishex(int c) {
		if ('0' <= c && c <= '9')
			return true;
		if ('a' <= c && c <= 'f')
			return true;
		if ('A' <= c && c <= 'F')
			return true;
		return false;
	}

	boolean isspace(int c) {
		switch (c) {
		case ' ':
		case '\t':
			return true;
		}
		return false;
	}

	int fromhex(int c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	public byte checksum() {
		byte	got = 0;

		got += data.length;
		got += (address >> 8) & 0xff;
		got += (address     ) & 0xff;
		got += type;
		for (int i = 0; i < data.length; i++)
			got += data[i];
		return (byte) (-got);
	}

	public int compareTo(Object other) {
		HexRecord	o = (HexRecord) other;

		long diff = address - o.address;

		if (diff > 0)
			return 1;
		if (diff < 0)
			return -1;
		return 0;
	}

	public String toString() {
		return String.format("%04x: %02x (%d)", address, type, data.length);
	}

	public HexRecord(HexFileInputStream input) throws IOException, EOFException {
		read_state	state = read_state.marker;
		long		nhexbytes = 0;
		long		hex = 0;
		int		ndata = 0;
		byte		got_checksum;

		while (state != read_state.done) {
			int c = input.read();
			if (c < 0 && state != read_state.white && state != read_state.marker)
				throw new IOException(String.format("%d: Unexpected EOF", input.line));
			if (c == ' ')
				continue;
			switch (state) {
			case marker:
				if (c == EOF || c == -1)
					throw new EOFException();
				if (c != ':')
					throw new IOException(String.format ("Missing ':' (got %x)", c));
				state = read_state.length;
				nhexbytes = 2;
				hex = 0;
				break;
			case length:
			case address:
			case type:
			case data:
			case checksum:
				if(!ishex(c))
					throw new IOException(String.format("Non-hex char '%c'", c));
				hex = hex << 4 | fromhex(c);
				--nhexbytes;
				if (nhexbytes != 0)
					break;

				switch (state) {
				case length:
					data = new byte[(int) hex];
					state = read_state.address;
					nhexbytes = 4;
					break;
				case address:
					address = hex;
					state = read_state.type;
					nhexbytes = 2;
					break;
				case type:
					type = (int) hex;
					if (data.length > 0)
						state = read_state.data;
					else
						state = read_state.checksum;
					nhexbytes = 2;
					ndata = 0;
					break;
				case data:
					data[ndata] = (byte) hex;
					ndata++;
					nhexbytes = 2;
					if (ndata == data.length)
						state = read_state.checksum;
					break;
				case checksum:
					checksum = (byte) hex;
					state = read_state.newline;
					break;
				default:
					break;
				}
				hex = 0;
				break;
			case newline:
				if (c != '\n' && c != '\r')
					throw new IOException("Missing newline");
				state = read_state.white;
				break;
			case white:
				if (!isspace(c)) {
					input.unread(c);
					state = read_state.done;
				}
				break;
			case done:
				break;
			}
		}
		got_checksum = checksum();
		if (got_checksum != checksum)
			throw new IOException(String.format("Invalid checksum (read 0x%02x computed 0x%02x)\n",
							    checksum, got_checksum));
	}
}

public class AltosHexfile {
	public long		address;
	public long		max_address;
	public byte[]		data;
	LinkedList<AltosHexsym>	symlist = new LinkedList<AltosHexsym>();

	public byte get_byte(long a) {
		return data[(int) (a - address)];
	}

	public int get_u8(long a) {
		return ((int) get_byte(a)) & 0xff;
	}

	public int get_u16(long a) {
		return get_u8(a) | (get_u8(a+1) << 8);
	}

	/* CC1111-based products have the romconfig stuff located
	 * at a fixed address; when the file we load has no symbols,
	 * assume it is one of those and set the symbols appropriately
	 */
	final static int ao_romconfig_version_addr = 0xa0;
	final static int ao_romconfig_check_addr = 0xa2;
	final static int ao_serial_number_addr = 0xa4;
	final static int ao_radio_cal_addr = 0xa6;
	final static int ao_usb_descriptors_addr = 0xaa;

	static AltosHexsym[] cc_symbols = {
		new AltosHexsym("ao_romconfig_version", ao_romconfig_version_addr),
		new AltosHexsym("ao_romconfig_check", ao_romconfig_check_addr),
		new AltosHexsym("ao_serial_number", ao_serial_number_addr),
		new AltosHexsym("ao_radio_cal", ao_radio_cal_addr),
		new AltosHexsym("ao_usb_descriptors", ao_usb_descriptors_addr)
	};

	static final int AO_USB_DESC_DEVICE		= 1;
	static final int AO_USB_DESC_STRING		= 3;

	static final int AO_ROMCONFIG_VERSION_INDEX	= 0;
	static final int AO_ROMCONFIG_CHECK_INDEX	= 1;
	static final int AO_SERIAL_NUMBER_INDEX		= 2;
	static final int AO_RADIO_CAL_INDEX		= 3;
	static final int AO_USB_DESCRIPTORS_INDEX	= 4;

	private void add_cc_symbols() {
		for (int i = 0; i < cc_symbols.length; i++)
			symlist.add(cc_symbols[i]);
	}

	public void add_symbol(AltosHexsym symbol) {
		symlist.add(symbol);
	}

	/* Take symbols from another hexfile and duplicate them here */
	public void add_symbols(AltosHexfile other) {
		for (AltosHexsym symbol : other.symlist)
			symlist.add(symbol);
	}

	public AltosHexsym lookup_symbol(String name) {
		if (symlist.isEmpty())
			add_cc_symbols();

		for (AltosHexsym symbol : symlist)
			if (name.equals(symbol.name))
				return symbol;
		return null;
	}

	private static final int look_around[] = { 0, -2, 2, -4, 4 };

	private long find_usb_descriptors() {
		AltosHexsym	usb_descriptors = lookup_symbol("ao_usb_descriptors");
		long		a;

		if (usb_descriptors == null)
			return -1;

		/* The address of this has moved depending on padding
		 * in the linker script and romconfig symbols. Look
		 * forward and backwards two and four bytes to see if
		 * we can find it
		 */
		a = usb_descriptors.address;

		for (int look : look_around) {
			try {
				if (get_u8(a + look) == 0x12 && get_u8(a + look + 1) == AO_USB_DESC_DEVICE)
					return a;
			} catch (ArrayIndexOutOfBoundsException ae) {
				continue;
			}
		}
		return -1;
	}

	public AltosUsbId find_usb_id() {
		long a = find_usb_descriptors();

		if (a == -1)
			return null;

		/* Walk the descriptors looking for the device */
		while (get_u8(a+1) != AO_USB_DESC_DEVICE) {
			int delta = get_u8(a);
			a += delta;
			if (delta == 0 || a >= max_address)
				return null;
		}

		return new AltosUsbId(get_u16(a + 8),
				      get_u16(a + 10));
	}

	public String find_usb_product() {
		long		a = find_usb_descriptors();
		int		num_strings;
		int		product_string;

		if (a == -1)
			return null;

		product_string = get_u8(a+15);

		/* Walk the descriptors looking for the device */
		num_strings = 0;
		for (;;) {
			if (get_u8(a+1) == AO_USB_DESC_STRING) {
				++num_strings;
				if (num_strings == product_string + 1)
					break;
			}

			int delta = get_u8(a);
			a += delta;
			if (delta == 0 || a >= max_address)
				return null;
		}

		int product_len = get_u8(a);

		if (product_len <= 0)
			return null;

		String product = "";

		for (int i = 0; i < product_len - 2; i += 2) {
			int	c = get_u16(a + 2 + i);

			product += Character.toString((char) c);
		}

		if (AltosLink.debug)
			System.out.printf("product %s\n", product);

		return product;
	}

	private String make_string(byte[] data, int start, int length) {
		String s = "";
		for (int i = 0; i < length; i++)
			s += (char) data[start + i];
		return s;
	}

	public AltosHexfile(byte[] bytes, long offset) {
		data = bytes;
		address = offset;
		max_address = address + bytes.length;
	}

	public AltosHexfile(FileInputStream file) throws IOException {
		HexFileInputStream	input = new HexFileInputStream(file);
		LinkedList<HexRecord>	record_list = new LinkedList<HexRecord>();
		boolean			done = false;

		while (!done) {
			try {
				HexRecord	record = new HexRecord(input);

				record_list.add(record);
			} catch (EOFException eof) {
				done = true;
			}
		}

		long	extended_addr = 0;
		long	base = 0;
		long	bound = 0;
		boolean	set = false;
		for (HexRecord record : record_list) {
			long addr;
			switch (record.type) {
			case 0:
				addr = extended_addr + record.address;
				long r_bound = addr + record.data.length;
				if (!set || addr < base)
					base = addr;
				if (!set || r_bound > bound)
					bound = r_bound;
				set = true;
				break;
			case 1:
				break;
			case 2:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 4;
				break;
			case 4:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 16;
				break;
			case 0xfe:
				String name = make_string(record.data, 0, record.data.length);
				addr = extended_addr + record.address;
				AltosHexsym s = new AltosHexsym(name, addr);
				symlist.add(s);
				break;
			default:
				throw new IOException ("invalid hex record type");
			}
		}

		if (!set || base >= bound)
			throw new IOException("invalid hex file");

		if (bound - base > 4 * 1024 * 1024)
			throw new IOException("hex file too large");

		data = new byte[(int) (bound - base)];
		address = base;
		max_address = bound;
		Arrays.fill(data, (byte) 0xff);

		/* Paint the records into the new array */
		for (HexRecord record : record_list) {
			switch (record.type) {
			case 0:
				long addr = extended_addr + record.address;
				long r_bound = addr + record.data.length;
				for (int j = 0; j < record.data.length; j++)
					data[(int) (addr - base) + j] = record.data[j];
				break;
			case 1:
				break;
			case 2:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 4;
				break;
			case 4:
				if (record.data.length != 2)
					throw new IOException("invalid extended segment address record");
				extended_addr = ((record.data[0] << 8) + (record.data[1])) << 16;
				break;
			case 0xfe:
				break;
			default:
				throw new IOException ("invalid hex record type");
			}
		}
	}
}
