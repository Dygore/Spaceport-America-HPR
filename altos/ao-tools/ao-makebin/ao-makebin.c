/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ao-hex.h"
#include "ao-elf.h"
#include "ao-dfu.h"

static const struct option options[] = {
	{ .name = "verbose", .has_arg = 0, .val = 'v' },
	{ .name = "output", .has_arg = 1, .val = 'o' },
	{ .name = "base", .has_arg = 1, .val = 'b' },
	{ .name = "align", .has_arg = 1, .val = 'a' },
	{ .name = "dfu", .has_arg = 0, .val = 'd' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--verbose=<level>] [--output=<output.bin>] [--base=<base-address>] [--align=<align>] [--dfu] <input.elf> ...\n", program);
	exit(1);
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

static struct ao_dfu_info dfu_info = {
	.bcdDevice = 0x0000,
	.idProduct = 0xdf11,
	.idVendor = 0x0483,
};

int
main (int argc, char **argv)
{
	char			*output = NULL;
	struct ao_hex_image	*image = NULL;
	struct ao_sym		*file_symbols;
	int			num_file_symbols;
	FILE			*file;
	int			c;
	uint32_t		base = 0xffffffff;
	uint32_t		align = 0;
	uint32_t		length;
	int			verbose = 0;
	int			dfu = 0;

	while ((c = getopt_long(argc, argv, "dvo:b:a:", options, NULL)) != -1) {
		switch (c) {
		case 'o':
			output = optarg;
			break;
		case 'v':
			verbose++;
			break;
		case 'b':
			base = strtoul(optarg, NULL, 0);
			break;
		case 'a':
			align = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			dfu = 1;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	while (argv[optind]) {
		char			*input = argv[optind];
		struct ao_hex_image	*tmp;

		if (ends_with (input, ".ihx"))
			tmp = ao_hex_load(input, &file_symbols, &num_file_symbols);
		else
			tmp = ao_load_elf(input, &file_symbols, &num_file_symbols);

		if (!tmp)
			usage(argv[0]);

		if (verbose)
			fprintf(stderr, "%s: 0x%x %d\n", input, tmp->address, tmp->length);

		if (image) {
			image = ao_hex_image_cat(image, tmp);
			if (!image)
				usage(argv[0]);
		} else
			image = tmp;
		optind++;
	}

	if (base != 0xffffffff && base > image->address) {
		fprintf(stderr, "requested base 0x%x is after image address 0x%x\n",
			base, image->address);
		usage(argv[0]);
	}

	if (verbose)
		fprintf(stderr, "%s: base 0x%x length %d\n", output ? output : "<stdout>", image->address, image->length);

	if (!output)
		file = stdout;
	else {
		file = fopen(output, "w");
		if (!file) {
			perror(output);
			exit(1);
		}
	}

	if (dfu) {
		if (!ao_dfu_write(file, &dfu_info, 1, image)) {
			fprintf(stderr, "%s: dfu_write failed: %s\n", output, strerror(errno));
			if (output)
				unlink(output);
			exit(1);
		}
	} else {
		while (base < image->address) {
			fputc(0xff, file);
			base++;
		}

		if (fwrite(image->data, 1, image->length, file) != image->length) {
			fprintf(stderr, "%s: failed to write bin file\n", output ? output : "<stdout>");
			if (output)
				unlink(output);
			exit(1);
		}

		if (align) {
			length = image->length;

			while (length % align) {
				fputc(0xff, file);
				length++;
			}
		}
		fflush(file);
	}

	exit(0);
}
