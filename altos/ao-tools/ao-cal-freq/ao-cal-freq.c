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
#include <math.h>
#include "ao-elf.h"
#include "ccdbg.h"
#include "cc-usb.h"
#include "cc.h"
#include "ao-verbose.h"

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "verbose", .has_arg = 0, .val = 'v' },
	{ .name = "output", .has_arg = 1, .val = 'o' },
	{ .name = "nosave", .has_arg = 0, .val = 'n' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--verbose] [--nosave] [--device=<device>] [-tty=<tty>] [--output=<cal-value-file>]\n", program);
	exit(1);
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

static int
do_save(struct cc_usb *usb)
{
	int ret = 0;

	printf("Saving calibration to device\n");
	cc_usb_printf(usb, "c w\nv\n");
	for (;;) {
		char	line[512];

		cc_usb_getline(usb, line, sizeof (line));
		if (strstr(line, "Nothing to save"))
			ret = 1;
		if (strstr(line, "Saved"))
			ret = 1;
		if (strstr(line, "software-version"))
			break;
	}
	if (!ret) {
		printf("Calibration save failed\n");
	}
	return ret;
}

static int
do_output(char *output, int cur_cal)
{
	printf ("Saving calibration value to file \"%s\"\n", output);

	FILE	*out = fopen(output, "w");
	int	ret = 1;

	if (!out) {
		perror(output);
		return 0;
	}

	if (fprintf(out, "%d\n", cur_cal) < 0) {
		perror("fprintf");
		ret = 0;
	}
	if (fflush(out) != 0) {
		perror("fflush");
		ret = 0;
	}
	if (fclose(out) != 0) {
		perror("fclose");
		ret = 0;
	}
	return ret;
}

static int
do_cal(char *tty, int save, char *output)
{
	struct cc_usb *usb = NULL;
	struct flash	*b;
	char	line[1024];
	double	measured_freq;
	char	**cur_freq_words;
	char	**cur_cal_words;
	char	*line_end;
	int	cur_freq;
	int	cur_cal;
	int	new_cal;
	int	ret = 1;
	int	changed = 0;

	for(;;) {
		usb = cc_usb_open(tty);

		if (!usb) {
			fprintf(stderr, "failed to open device\n");
			ret = 0;
			break;
		}

		cc_usb_printf(usb, "E 0\n");

		b = flash(usb);

		cur_cal_words = find_flash(b, "Radio cal:");
		cur_freq_words = find_flash(b, "Frequency:");

		if (!cur_cal_words || !cur_freq_words) {
			fprintf(stderr, "no response\n");
			ret = 0;
			break;
		}

		cur_cal = atoi(cur_cal_words[2]);
		cur_freq = atoi(cur_freq_words[1]);

		printf ("Current radio calibration %d\n", cur_cal);
		printf ("Current radio frequency: %7.3f\n", cur_freq / 1000.0);

		cc_usb_sync(usb);
		cc_usb_printf(usb, "C 1\n");
		cc_usb_sync(usb);

		printf("Generating RF carrier. Please enter measured frequency [enter for done]: ");
		fflush(stdout);
		fgets(line, sizeof (line) - 1, stdin);
		cc_usb_printf(usb, "C 0\n");
		cc_usb_sync(usb);

		measured_freq = strtod(line, &line_end);
		if (line_end == line)
			break;

		new_cal = floor ((((double) cur_freq / 1000.0) / measured_freq) * cur_cal + 0.5);

		if (new_cal == cur_cal) {
			printf("Calibration value %d unchanged\n", cur_cal);
		} else {
			printf ("Setting cal value %d\n", new_cal);

			cc_usb_printf (usb, "c f %d\n", new_cal);
			changed = 1;
			cc_usb_sync(usb);
		}
		cc_usb_close(usb);
	}
	if (usb) {
		if (ret && save) {
			if (changed) {
				if (!do_save(usb))
					ret = 0;
			} else {
				printf("Calibration unchanged, not saving\n");
			}
		}
		if (ret && output) {
			if (!do_output(output, cur_cal))
				ret = 0;
		}
		cc_usb_close(usb);
	}
	return ret;
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	int			c;
	char			*tty = NULL;
	int			verbose = 0;
	int			save = 1;
	int			ret = 0;
	char			*output = NULL;

	while ((c = getopt_long(argc, argv, "vnT:D:o:", options, NULL)) != -1) {
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
		case 'n':
			save = 0;
			break;
		case 'o':
			output = optarg;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	ao_verbose = verbose;
	if (verbose)
		ccdbg_add_debug(CC_DEBUG_BITBANG);

	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "AltosFlash");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TeleMega");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	if (!do_cal(tty, save, output))
		ret = 1;
	return ret;
}
