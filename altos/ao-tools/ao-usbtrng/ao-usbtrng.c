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

#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include "ao-elf.h"
#include "ccdbg.h"
#include "cc-usb.h"
#include "cc.h"
#include "ao-verbose.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "verbose", .has_arg = 0, .val = 'v' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--verbose] [--device=<AltOS-device>] [-tty=<tty>] [<kbytes>]\n", program);
	exit(1);
}

static void
done(struct cc_usb *cc, int code)
{
	cc_usb_close(cc);
	exit (code);
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	int			i;
	int			c;
	struct cc_usb		*cc = NULL;
	char			*tty = NULL;
	int			verbose = 0;
	int			ret = 0;
	int			kbytes = 0; /* 0 == continuous */
	int			written;
	uint8_t			bits[1024];

	while ((c = getopt_long(argc, argv, "vT:D:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	if (optind < argc)
		kbytes = atoi(argv[optind]);

	ao_verbose = verbose;

	if (verbose > 1)
		ccdbg_add_debug(CC_DEBUG_BITBANG);

	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "usbtrng");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	cc = cc_usb_open(tty);

	if (!cc)
		exit(1);

	if (kbytes) {
		cc_usb_printf(cc, "f %d\n", kbytes);

		while (kbytes--) {
			for (i = 0; i < 1024; i++)
				bits[i] = cc_usb_getchar(cc);
			write(1, bits, 1024);
		}
	} else { /* 0 == continuous */
		written = 0;
		while (written >= 0) {
			cc_usb_printf(cc, "f 1\n");
			for (i = 0; i < 1024; i++)
				bits[i] = cc_usb_getchar(cc);
			written = write(1, bits, 1024);
		}
	}

	done(cc, ret);
}
