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
#include <ctype.h>
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
	{ .name = "rplus", .has_arg = 1, .val = 'a' },
	{ .name = "rminus", .has_arg = 1, .val = 'b' },
	{ .name = "adcmax", .has_arg = 1, .val = 'm' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--verbose=<verbose>] [--device=<device>] [--tty=<tty>] [--rplus=val] [--rminus=val] [--adcmax=val] main|drogue\n", program);
	exit(1);
}

static void
done(struct cc_usb *cc, int code)
{
/*	cc_usb_printf(cc, "a\n"); */
	cc_usb_close(cc);
	exit (code);
}

struct igniter {
	struct igniter	*next;
	char		name[512];
	char		status[512];
	int		adc;
};

static bool
map_igniter_name(char *adc_name, char *igniter_name)
{
	char	*colon = strchr(adc_name, ':');
	if (!colon)
		return false;
	*colon = '\0';
	if (strlen(adc_name) == 1 && isupper(adc_name[0])) {
		igniter_name[0] = '0' + adc_name[0] - 'A';
		igniter_name[1] = '\0';
	} else {
		strcpy(igniter_name, adc_name);
	}
	return true;
}

static const char *
other_igniter_name(const char *name)
{
	if (!strcmp(name, "drogue"))
		return "apogee";
	if (!strcmp(name, "apogee"))
		return "drogue";
	return name;
}

static struct igniter *
igniters(struct cc_usb *usb)
{
	struct igniter	*head = NULL, **tail = &head;
	cc_usb_printf(usb, "t\na\nv\n");
	for (;;) {
		char	line[512];
		char	name[512];
		char	status[512];
		char	adc_name[512];
		char	igniter_name[512];

		cc_usb_getline(usb, line, sizeof (line));
		if (strstr(line, "software-version"))
			break;
		if (sscanf(line, "Igniter: %s Status: %s", name, status) == 2) {
			struct igniter	*i = calloc (1, sizeof (struct igniter));
			strcpy(i->name, name);
			strcpy(i->status, status);
			i->next = NULL;
			*tail = i;
			tail = &i->next;
		}
		if (strncmp(line, "tick:", 5) == 0) {
			char 	*tok;
			char	*l = line;
			bool	found_igniter = false;

			while ((tok = strtok(l, " ")) != NULL) {
				l = NULL;
				if (found_igniter) {
					struct igniter *i;
					for (i = head; i; i = i->next)
						if (!strcmp(i->name, igniter_name) ||
						    !strcmp(i->name, other_igniter_name(igniter_name)))
						{
							i->adc = atoi(tok);
							break;
						}
					found_igniter = false;
				} else {
					strcpy(adc_name, tok);
					found_igniter = map_igniter_name(adc_name, igniter_name);
				}
			}
		}
	}
	return head;
}

static void
free_igniters(struct igniter *i) {
	struct igniter *n;

	while (i) {
		n = i->next;
		free(i);
		i = n;
	}
}

static struct igniter *
find_igniter(struct igniter *i, char *name)
{
	for (; i; i = i->next)
		if (strcmp(i->name, name) == 0) {
			printf("igniter %s adc %d\n", i->name, i->adc);
			return i;
		}
	return NULL;
}

static const double ref_volts = 3.3;

static double
compute_voltage(int adc, double rplus, double rminus, int adc_max)
{
	return (double) adc / (double) adc_max * ref_volts * (rplus + rminus) / rminus;
}

static int
do_igniter(struct cc_usb *usb, char *name, double rplus, double rminus, int adc_max)
{
	struct igniter	*all = igniters(usb);
	struct igniter	*this = find_igniter(all, name);
	double volts = -1;
	if (!this) {
		struct igniter	*i;
		printf("no igniter %s found in", name);
		for (i = all; i; i = i->next)
			printf(" %s", i->name);
		printf("\n");
		free_igniters(all);
		return 0;
	}
	if (rplus && rminus && adc_max) {
		volts = compute_voltage(this->adc, rplus, rminus, adc_max);
		if (volts < 1 || volts > 4) {
			printf("igniter %s voltage is %f, not in range of 1-4 volts\n", this->name, volts);
			free_igniters(all);
			return 0;
		}
	}
	if (strcmp(this->status, "ready") != 0) {
		printf("igniter %s status is %s\n", this->name, this->status);
		free_igniters(all);
		return 0;
	}
	cc_usb_printf(usb, "i DoIt %s\n", this->name);
	cc_usb_sync(usb);
	free_igniters(all);
	usleep(200000);
	return 1;
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
	double			rplus = 0.0;
	double			rminus = 0.0;
	int			adcmax = 0;

	while ((c = getopt_long(argc, argv, "rT:D:c:s:v:a:b:m:", options, NULL)) != -1) {
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
		case 'a':
			rplus = strtod(optarg, NULL);
			break;
		case 'b':
			rminus = strtod(optarg, NULL);
			break;
		case 'm':
			adcmax = strtol(optarg, NULL, 0);
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
		tty = cc_usbdevs_find_by_arg(device, "TeleMega-v1.0");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TeleMetrum-v2.0");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "TeleMini-v2.0");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "EasyMega-v1.0");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "EasyMetrum-v1.0");
	if (!tty)
		tty = cc_usbdevs_find_by_arg(device, "EasyMini-v1.0");
	if (!tty)
		tty = getenv("ALTOS_TTY");
	if (!tty)
		tty="/dev/ttyACM0";

	cc = cc_usb_open(tty);

	if (!cc)
		exit(1);

	for (i = optind; i < argc; i++) {
		char	*name = argv[i];

		if (!do_igniter(cc, name, rplus, rminus, adcmax))
			ret++;
	}
	done(cc, ret);
}
