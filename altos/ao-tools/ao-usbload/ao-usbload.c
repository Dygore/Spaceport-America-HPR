/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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
#include "ao-usbload.h"
#include "ao-selfload.h"
#include "ao-verbose.h"
#include "ao-editaltos.h"

static uint16_t
get_uint16(struct cc_usb *cc, uint32_t addr)
{
	uint16_t	result;
	result = ao_self_get_uint16(cc, addr);
	return result;
}

/*
 * Read a 32-bit value from the target device with arbitrary
 * alignment
 */
static uint32_t
get_uint32(struct cc_usb *cc, uint32_t addr)
{
	uint32_t	result;

	result = ao_self_get_uint32(cc, addr);
	return result;
}

/*
 * Check to see if the target device has been
 * flashed with a similar firmware image before
 *
 * This is done by looking for the same romconfig version,
 * which should be at the same location as the linker script
 * places this at 0x100 from the start of the rom section
 */
static int
check_flashed(struct cc_usb *cc)
{
	uint16_t	romconfig_version = get_uint16(cc, AO_ROMCONFIG_VERSION);
	uint16_t	romconfig_check = get_uint16(cc, AO_ROMCONFIG_CHECK);

	if (romconfig_version != (uint16_t) ~romconfig_check) {
		fprintf (stderr, "Device has not been flashed before\n");
		return 0;
	}
	return 1;
}

static const struct option options[] = {
	{ .name = "tty", .has_arg = 1, .val = 'T' },
	{ .name = "device", .has_arg = 1, .val = 'D' },
	{ .name = "raw", .has_arg = 0, .val = 'r' },
	{ .name = "cal", .has_arg = 1, .val = 'c' },
	{ .name = "serial", .has_arg = 1, .val = 's' },
	{ .name = "verbose", .has_arg = 1, .val = 'v' },
	{ .name = "wait", .has_arg = 0, .val = 'w' },
	{ .name = "force", .has_arg = 0, .val = 'f' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--raw] [--verbose=<verbose>] [--device=<device>] [-tty=<tty>] [--cal=<radio-cal>] [--serial=<serial>] [--wait] [--force] file.{elf,ihx}\n", program);
	exit(1);
}

static void
done(struct cc_usb *cc, int code)
{
/*	cc_usb_printf(cc, "a\n"); */
	cc_usb_close(cc);
	exit (code);
}

static int
ends_with(char *whole, char *suffix)
{
	int whole_len = strlen(whole);
	int suffix_len = strlen(suffix);

	if (suffix_len > whole_len)
		return 0;
	return strcmp(whole + whole_len - suffix_len, suffix) == 0;
}

static int
ucs2len(uint16_t *ucs2)
{
	int	len = 0;
	while (*ucs2++)
		len++;
	return len;
}

static int
putucs4(uint32_t c, FILE *file)
{
	char d;
	int	bits;

	     if (c <       0x80) { d = c;                         bits= -6; }
	else if (c <      0x800) { d= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
	else if (c <    0x10000) { d= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
	else if (c <   0x200000) { d= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }
	else if (c <  0x4000000) { d= ((c >> 24) & 0x03) | 0xF8;  bits= 18; }
	else if (c < 0x80000000) { d= ((c >> 30) & 0x01) | 0xFC;  bits= 24; }
	else return EOF;

	if (putc (d, file) < 0)
		return EOF;

	for ( ; bits >= 0; bits-= 6)
		if (putc (((c >> bits) & 0x3F) | 0x80, file) < 0)
			return EOF;

	return 0;
}

static void
putucs2str(uint16_t *ucs2str, FILE *file)
{
	uint16_t	ucs2;

	while ((ucs2 = *ucs2str++) != 0)
		putucs4(ucs2, file);
}

int
main (int argc, char **argv)
{
	char			*device = NULL;
	char			*filename;
	int			raw = 0;
	char			*serial_end;
	unsigned int		serial = 0;
	uint32_t		cal = 0;
	char			*cal_end;
	int			c;
	int			was_flashed = 0;
	struct ao_hex_image	*load;
	struct cc_usb		*cc = NULL;
	char			*tty = NULL;
	int			success;
	int			verbose = 0;
	struct ao_sym		*file_symbols;
	int			num_file_symbols;
	uint32_t		flash_base, flash_bound;
	int			has_flash_size = 0;
	int			force = 0;

	while ((c = getopt_long(argc, argv, "wrfT:D:c:s:v:", options, NULL)) != -1) {
		switch (c) {
		case 'T':
			tty = optarg;
			break;
		case 'D':
			device = optarg;
			break;
		case 'r':
			raw = 1;
			break;
		case 'w':
			cc_default_timeout = -1;
			break;
		case 'c':
			cal = strtoul(optarg, &cal_end, 10);
			if (cal_end == optarg || *cal_end != '\0')
				usage(argv[0]);
			break;
		case 's':
			serial = strtoul(optarg, &serial_end, 10);
			if (serial_end == optarg || *serial_end != '\0')
				usage(argv[0]);
			break;
		case 'v':
			verbose++;
			break;
		case 'f':
			force = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	ao_verbose = verbose;

	if (verbose > 1)
		ccdbg_add_debug(CC_DEBUG_BITBANG);

	filename = argv[optind];
	if (filename == NULL)
		usage(argv[0]);

	if (ends_with (filename, ".elf")) {
		load = ao_load_elf(filename, &file_symbols, &num_file_symbols);
	} else if (ends_with (filename, ".ihx")) {
		load = ao_hex_load(filename, &file_symbols, &num_file_symbols);
	} else
		usage(argv[0]);

	if (!raw) {
		if (!ao_editaltos_find_symbols(file_symbols, num_file_symbols, ao_symbols, ao_num_symbols)) {
			fprintf(stderr, "Cannot find required symbols\n");
			usage(argv[0]);
		}
	}

	{
		int	is_loader;
		int	tries;

		for (tries = 0; tries < 3; tries++) {
			char	*this_tty = tty;
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "AltosFlash");
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "TeleMega");
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "TeleMetrum");
			if (!this_tty)
				this_tty = cc_usbdevs_find_by_arg(device, "TeleGPS");
			if (!this_tty)
				this_tty = getenv("ALTOS_TTY");
			if (!this_tty)
				this_tty="/dev/ttyACM0";

			cc = cc_usb_open(this_tty);

			if (!cc)
				exit(1);
			cc_usb_printf(cc, "v\n");
			is_loader = 0;
			for (;;) {
				char	line[256];
				cc_usb_getline(cc, line, sizeof(line));
				if (!strncmp(line, "altos-loader", 12))
					is_loader = 1;
				if (!strncmp(line, "flash-range", 11)) {
					int i;
					for (i = 11; i < strlen(line); i++)
						if (line[i] != ' ')
							break;
					if (sscanf(line + i, "%x %x", &flash_base, &flash_bound) == 2)
						has_flash_size = 1;
				}
				if (!strncmp(line, "software-version", 16))
					break;
			}
			if (is_loader)
				break;
			printf ("rebooting to loader\n");
			cc_usb_printf(cc, "X\n");
			cc_usb_close(cc);
			sleep(1);
			cc = NULL;
		}
		if (!is_loader) {
			fprintf(stderr, "Cannot switch to boot loader\n");
			exit(1);
		}
#if 0
		{
			uint8_t	check[256];
			int	i = 0;

			ao_self_block_read(cc, AO_BOOT_APPLICATION_BASE, check);
			for (;;) {
				uint8_t block[256];
				putchar ('.');
				if (++i == 40) {
					putchar('\n');
					i = 0;
				}
				fflush(stdout);
				ao_self_block_write(cc, AO_BOOT_APPLICATION_BASE, block);
				ao_self_block_read(cc, AO_BOOT_APPLICATION_BASE, block);
				if (memcmp(block, check, 256) != 0) {
					fprintf (stderr, "read differed\n");
					exit(1);
				}
			}
		}
#endif
	}

	/* If the device can tell us the size of flash, make sure
	 * the image fits in that
	 */
	if (has_flash_size) {
		if (load->address < flash_base ||
		    load->address + load->length > flash_bound)
		{
			fprintf(stderr, "Image does not fit on device.\n");
			fprintf(stderr, "  Image base:  %08x bounds %08x\n",
				load->address, load->address + load->length);
			fprintf(stderr, "  Device base: %08x bounds %08x\n",
				flash_base, flash_bound);
			done(cc, 1);
		}
	}

	if (!raw) {
		/* Go fetch existing config values
		 * if available
		 */
		was_flashed = check_flashed(cc);

		if (!serial) {
			if (!was_flashed) {
				fprintf (stderr, "Must provide serial number\n");
				done(cc, 1);
			}
			serial = get_uint16(cc, AO_SERIAL_NUMBER);
			if (!serial || serial == 0xffff) {
				fprintf (stderr, "Invalid existing serial %d\n", serial);
				done(cc, 1);
			}
		}

		if (!cal && AO_RADIO_CAL && was_flashed) {
			cal = get_uint32(cc, AO_RADIO_CAL);
			if (!cal || cal == 0xffffffff) {
				fprintf (stderr, "Invalid existing rf cal %d\n", cal);
				done(cc, 1);
			}
		}

		if (!force && was_flashed) {
			struct ao_usb_id	new_id, old_id;
			uint16_t		*new_product, *old_product;
			int			new_len, old_len;

			if (!ao_heximage_usb_id(load, &new_id)) {
				fprintf(stderr, "Can't get new USB id\n");
				done(cc, 1);
			}

			if (!ao_self_get_usb_id(cc, &old_id)) {
				fprintf(stderr, "Can't get old USB id\n");
				done(cc, 1);
			}
			if (new_id.vid != old_id.vid || new_id.pid != old_id.pid) {
				fprintf(stderr, "USB ID mismatch (device is %04x/%04x image is %04x/%04x)\n",
					old_id.vid, old_id.pid, new_id.vid, new_id.pid);
				done(cc, 1);
			}

			new_product = ao_heximage_usb_product(load);
			if (!new_product) {
				fprintf(stderr, "Can't get new USB product name\n");
				done(cc, 1);
			}
			old_product = ao_self_get_usb_product(cc);
			if (!old_product) {
				fprintf(stderr, "Can't get existing USB product name\n");
				done(cc, 1);
			}
			new_len = ucs2len(new_product);
			old_len = ucs2len(old_product);
			if (new_len != old_len || memcmp(new_product, old_product, new_len * 2) != 0) {
				fprintf(stderr, "USB product mismatch (device is ");
				putucs2str(old_product, stderr);
				fprintf(stderr, ", image is ");
				putucs2str(new_product, stderr);
				fprintf(stderr, ")\n");
				done(cc, 1);
			}
		}

		if (!ao_editaltos(load, serial, cal))
			done(cc, 1);
	}

	/* And flash the resulting image to the device
	 */
	success = ao_self_write(cc, load);

	if (!success) {
		fprintf (stderr, "\"%s\": Write failed\n", filename);
		done(cc, 1);
	}

	done(cc, 0);
}
