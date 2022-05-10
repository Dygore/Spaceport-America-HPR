/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "cc-usb.h"
#include "cc.h"

#define NUM_BLOCK	512

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "wait", .has_arg = 0, .val = 'w' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--tty <tty-name>] [--device <device-name>] [--wait]\n", program);
	exit(1);
}

static int get_nonwhite(struct cc_usb *cc, int timeout)
{
	int	c;

	for (;;) {
		c = cc_usb_getchar_timeout(cc, timeout);
		putchar(c);
		if (!isspace(c))
			return c;
	}
}

static const uint8_t test_data[] = {
	0xfc, 0xfd, 0xfe, 0xff, 0xf8, 0xf9, 0xfa, 0xfb, 0x40, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x17, 0x08,
};

static bool test_failed;
static int test_pos;

static void
check_test(uint8_t b)
{
	if (test_pos >= sizeof (test_data) || test_data[test_pos++] != b)
		test_failed = true;
}

static uint8_t
get_hexc(struct cc_usb *cc)
{
	int	c = get_nonwhite(cc, 1000);

	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'F')
		return c - 'A' + 10;
	fprintf(stderr, "Non-hex char '%c'\n", c);
	exit(1);
}

static int file_crc;

static const int POLY = 0x8408;

static int
log_crc(int crc, int b)
{
	int	i;

	for (i = 0; i < 8; i++) {
		if (((crc & 0x0001) ^ (b & 0x0001)) != 0)
			crc = (crc >> 1) ^ POLY;
		else
			crc = crc >> 1;
		b >>= 1;
	}
	return crc & 0xffff;
}

static uint8_t
get_hex(struct cc_usb *cc)
{
	int	a = get_hexc(cc);
	int	b = get_hexc(cc);
	int	h = (a << 4) + b;

	file_crc = log_crc(file_crc, h);
	check_test(h);
	return h;
}

static int get_32(struct cc_usb *cc)
{
	int	v = 0;
	int	i;
	for (i = 0; i < 4; i++) {
		v += get_hex(cc) << (i * 8);
	}
	return v;
}

static int get_16(struct cc_usb *cc)
{
	int	v = 0;
	int	i;
	for (i = 0; i < 2; i++) {
		v += get_hex(cc) << (i * 8);
	}
	return v;
}

static int swap16(int i)
{
	return ((i << 8) & 0xff00) | ((i >> 8) & 0xff);
}

static int find_header(struct cc_usb *cc)
{
	for (;;) {
		if (get_nonwhite(cc, -1) == 'M' && get_nonwhite(cc, 1000) == 'P')
			return 1;
	}
}

int
main (int argc, char **argv)
{
	struct cc_usb	*cc;
	char		*tty = NULL;
	char		*device = NULL;
	int		c;
	int		nsamples;
	int		i;
	int		crc;
	int		current_crc;
	int		wait = 0;

	while ((c = getopt_long(argc, argv, "wT:D:", options, NULL)) != -1) {
		switch (c) {
		case 'w':
			wait = 1;
			break;
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if (!tty) {
		for (;;) {
			tty = cc_usbdevs_find_by_arg(device, "FT230X Basic UART");
			if (tty) {
				if (wait) {
					printf("tty is %s\n", tty);
					sleep(1);
				}
				break;
			}
			if (!wait)
				break;
			sleep(1);
		}
	}
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyUSB0";
	cc = cc_usb_open(tty);
	if (!cc)
		exit(1);
	find_header(cc);
	file_crc = 0xffff;
	get_32(cc);	/* ground pressure */
	get_32(cc);	/* min pressure */
	nsamples = get_16(cc);	/* nsamples */
	for (i = 0; i < nsamples; i++)
		get_16(cc);	/* sample i */
	current_crc = swap16(~file_crc & 0xffff);
	crc = get_16(cc);	/* crc */
	putchar ('\n');
	if (crc == current_crc) {
		if (!test_failed)
			printf("\033[32mValid MicroTest Data\033[39m\n");
		else
			printf("CRC valid\n");
	}
	else
		printf("CRC invalid\n");
	cc_usb_close(cc);
	exit (0);
}
