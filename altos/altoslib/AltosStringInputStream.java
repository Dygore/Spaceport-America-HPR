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

public class AltosStringInputStream extends InputStream {

	String	s;
	int	at;
	int	mark;

	public int available() {
		return s.length() - at;
	}

	public void mark(int read_limit) {
		mark = at;
	}

	public boolean markSupported() {
		return true;
	}

	public int read() {
		if (at == s.length())
			return -1;
		return (int) s.charAt(at++);
	}

	public void reset() {
		at = mark;
	}

	public long skip(long n) throws IOException {
		if (n < 0) n = 0;

		if (at + n > s.length())
			n = s.length() - at;
		at += n;
		return n;
	}

	public AltosStringInputStream(String s) {
		this.s = s;
		this.at = 0;
	}
}
