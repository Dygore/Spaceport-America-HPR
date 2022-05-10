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
#include <termios.h>
#include "ao-elf.h"
#include "ccdbg.h"
#include "cc-usb.h"
#include "cc.h"
#include "ao-verbose.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "raw", .has_arg = 0, .val = 'r' },
	{ .name = "verbose", .has_arg = 1, .val = 'v' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--verbose=<verbose>] [--device=<device>] [-tty=<tty>]\n", program);
	exit(1);
}

static void
done(struct cc_usb *cc, int code)
{
	cc_usb_close(cc);
	exit (code);
}

static char **
tok(char *line) {
	char	**strs = malloc (sizeof (char *)), *str;
	int	n = 0;

	while ((str = strtok(line, " \t"))) {
		line = NULL;
		strs = realloc(strs, (n + 2) * sizeof (char *));
		strs[n] = strdup(str);
		n++;
	}
	strs[n] = '\0';
	return strs;
}

struct flash {
	struct flash	*next;
	char		line[512];
	char		**strs;
};

static struct flash *
flash(struct cc_usb *usb)
{
	struct flash	*head = NULL, **tail = &head;
	cc_usb_printf(usb, "c s\nv\n");
	for (;;) {
		char	line[512];
		struct flash	*b;

		cc_usb_getline(usb, line, sizeof (line));
		b = malloc (sizeof (struct flash));
		strcpy(b->line, line);
		b->strs = tok(line);
		b->next = NULL;
		*tail = b;
		tail = &b->next;
		if (strstr(line, "software-version"))
			break;
	}
	return head;
}

static char **
find_flash(struct flash *b, char *word0) {
	for (;b; b = b->next) {
		if (strstr(b->line, word0))
			return b->strs;
	}
	return NULL;
}

static void
await_key(void)
{
	struct termios	termios, termios_save;
	char	buf[512];

	tcgetattr(0, &termios);
	termios_save = termios;
	cfmakeraw(&termios);
	tcsetattr(0, TCSAFLUSH, &termios);
	read(0, buf, sizeof (buf));
	tcsetattr(0, TCSAFLUSH, &termios_save);
}

static int
do_cal(struct cc_usb *usb) {
	struct flash	*b;
	char	**accel;
	char	line[1024];
	int	l = 0;
	int	running = 0;
	int	worked = 1;

	cc_usb_printf(usb, "E 1\nc a 0\n");

	for (;;) {
		int	c = cc_usb_getchar_timeout(usb, 20*1000);

		if (c == '\n')
			l = 0;
		else if (l < sizeof (line) - 1)
			line[l++] = c;
		line[l] = '\0';
		putchar(c); fflush(stdout);
		if (strstr(line, "press a key...")) {
			await_key();
			cc_usb_printf(usb, " ");
			l = 0;
			running = 1;
		}
		else if (strstr(line, "Invalid"))
			worked = 0;
		if (running && strstr(line, ">")) {
			printf("\n");
			break;
		}
	}
	cc_usb_printf(usb, "E 0\n");

	if (!worked) {
		printf("Calibration failed\n");
		return 0;
	}

	b = flash(usb);

	accel = find_flash(b, "Accel cal");
	if (!accel) {
		printf("no response\n");
		return 0;
	}

	printf ("Accel cal +1g: %s -1g: %s\n",
		accel[3], accel[5]);

	printf ("Saving..."); fflush(stdout);
	cc_usb_printf (usb, "c w\n");
	cc_usb_sync(usb);
	b = flash(usb);
	printf ("done\n");

	return worked;
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	int			c;
	struct cc_usb		*cc = NULL;
	char			*tty = NULL;
	int			verbose = 0;
	int			ret = 0;

	while ((c = getopt_long(argc, argv, "rT:D:c:s:v:", options, NULL)) != -1) {
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

	ao_verbose = verbose;

	if (verbose > 1)
		ccdbg_add_debug(CC_DEBUG_BITBANG);

	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "AltosFlash");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TeleMega");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	cc = cc_usb_open(tty);

	if (!cc)
		exit(1);

	if (!do_cal(cc))
		ret = 1;
	done(cc, ret);
}
