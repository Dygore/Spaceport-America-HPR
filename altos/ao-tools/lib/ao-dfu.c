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

#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "ao-hex.h"
#include "ao-dfu.h"

static uint32_t	dfu_crc;
static FILE	*dfu_file;
static int	dfu_failed;
static int	dfu_error;

static uint32_t update_crc(uint32_t crc, uint8_t byte)
{
	int j;
	uint32_t mask;

	crc = crc ^ byte;
	for (j = 0; j < 8; j++) {
		mask = -(crc & 1);
		crc = (crc >> 1) ^ (0xEDB88320 & mask);
	}
	return crc;
}

static void dfu_init(FILE *file)
{
	dfu_crc = 0xffffffff;
	dfu_file = file;
	dfu_failed = 0;
	dfu_error = 0;
}

static int dfu_fini(void)
{
	if (fflush(dfu_file) == EOF) {
		if (!dfu_failed) {
			dfu_failed = 1;
			dfu_error = errno;
		}
	}
	if (dfu_failed)
		errno = dfu_error;
	return !dfu_failed;
}

static void dfu_8(uint8_t byte) {
	if (putc(byte, dfu_file) == EOF) {
		if (!dfu_failed) {
			dfu_failed = 1;
			dfu_error = errno;
		}
	}
	dfu_crc = update_crc(dfu_crc, byte);
}

static void dfu_pad(int len) {
	while (len--)
		dfu_8(0);
}

static void dfu_string(char *string) {
	char	c;

	while ((c = *string++))
		dfu_8((uint8_t) c);
}

static void dfu_string_pad(char *string, int len) {
	char	c;

	while ((c = *string++)) {
		dfu_8((uint8_t) c);
		len--;
	}
	dfu_pad(len);
}

static void dfu_block(uint8_t *bytes, int len) {
	while (len--)
		dfu_8(*bytes++);
}

static void dfu_lsb16(uint16_t value) {
	dfu_8(value);
	dfu_8(value>>8);
}

static void dfu_lsb32(uint32_t value) {
	dfu_8(value);
	dfu_8(value >> 8);
	dfu_8(value >> 16);
	dfu_8(value >> 24);
}

static uint32_t dfu_image_size(struct ao_hex_image *image) {
	return 8 + image->length;
}

static uint32_t dfu_images_size(int num_image, struct ao_hex_image images[])
{
	uint32_t	size = 0;
	int		i;

	for (i = 0; i < num_image; i++)
		size += dfu_image_size(&images[i]);
	return size;
}

static void dfu_image(struct ao_hex_image *image)
{
	dfu_lsb32(image->address);
	dfu_lsb32(image->length);
	dfu_block(image->data, image->length);
}

static void dfu_target(char *name, int num_image, struct ao_hex_image images[])
{
	uint32_t	images_size = dfu_images_size(num_image, images);
	int		i;

	dfu_string("Target");
	dfu_8(0);
	if (name) {
		dfu_8(1);
		dfu_pad(3);
		dfu_string_pad(name, 255);
	} else {
		dfu_8(0);
		dfu_pad(3);
		dfu_pad(255);
	}
	dfu_lsb32(images_size);
	dfu_lsb32(num_image);
	for (i = 0; i < num_image; i++)
		dfu_image(&images[i]);
}

static uint32_t dfu_target_size(int num_image, struct ao_hex_image images[])
{
	return 274 + dfu_images_size(num_image, images);
}

static uint32_t
dfu_size(int num_image, struct ao_hex_image images[])
{
	uint32_t	size = 0;
	size += 11;	/* DFU Prefix */

	size += dfu_target_size(num_image, images);

	return size;
}

int
ao_dfu_write(FILE *file, struct ao_dfu_info *info, int num_image, struct ao_hex_image images[])
{
	uint32_t	total_size;

	total_size = dfu_size(num_image, images);

	dfu_init(file);
	/* DFU Prefix */
	dfu_string(DFU_SIGNATURE);
	dfu_8(0x01);
	dfu_lsb32(total_size);
	dfu_8(0x01);

	dfu_target("ST...", num_image, images);

	/* DFU Suffix */
	dfu_lsb16(info->bcdDevice);
	dfu_lsb16(info->idProduct);
	dfu_lsb16(info->idVendor);
	dfu_lsb16(DFU_SPEC_VERSION);
	dfu_string("UFD");
	dfu_8(16);
	dfu_lsb32(dfu_crc);
	return dfu_fini();
}

