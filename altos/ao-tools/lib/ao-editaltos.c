/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#include <string.h>
#include <stdlib.h>
#include "ao-editaltos.h"

struct ao_sym ao_symbols[] = {
	[AO_ROMCONFIG_VERSION_INDEX] = {
		.name = "ao_romconfig_version",
		.required = 1
	},
	[AO_ROMCONFIG_CHECK_INDEX] = {
		.name = "ao_romconfig_check",
		.required = 1
	},
	[AO_SERIAL_NUMBER_INDEX] = {
		.name = "ao_serial_number",
		.required = 1
	},
	[AO_RADIO_CAL_INDEX] = {
		.name = "ao_radio_cal",
		.required = 0
	},
	[AO_USB_DESCRIPTORS_INDEX] = {
		.name = "ao_usb_descriptors",
		.required = 0
	},
};

#define NUM_SYMBOLS		5

int ao_num_symbols = NUM_SYMBOLS;

/*
 * Edit the to-be-written memory block
 */
static bool
rewrite(struct ao_hex_image *load, unsigned address, uint8_t *data, int length)
{
	if (address < load->address || load->address + load->length < address + length)
		return false;

	memcpy(&load->data[address - load->address], data, length);
	return true;
}

/*
 * Find the symbols needed to correctly load the program
 */

bool
ao_editaltos_find_symbols(struct ao_sym *file_symbols, int num_file_symbols,
			  struct ao_sym *symbols, int num_symbols)
{
	int	f, s;

	for (f = 0; f < num_file_symbols; f++) {
		for (s = 0; s < num_symbols; s++) {
			if (strcmp(symbols[s].name, file_symbols[f].name) == 0) {
				symbols[s].addr = file_symbols[f].addr;
				symbols[s].found = true;
			}
		}
	}
	for (s = 0; s < num_symbols; s++)
		if (!symbols[s].found && symbols[s].required)
			return false;
	return true;
}

bool
ao_editaltos(struct ao_hex_image *image,
	     uint16_t serial,
	     uint32_t cal)
{
	uint8_t			*serial_ucs2;
	int			serial_ucs2_len;
	uint8_t			serial_int[2];
	unsigned int		s;
	int			i;
	int			string_num;
	uint8_t			cal_int[4];

	/* Write the config values into the flash image
	 */

	serial_int[0] = serial & 0xff;
	serial_int[1] = (serial >> 8) & 0xff;

	if (!rewrite(image, AO_SERIAL_NUMBER, serial_int, sizeof (serial_int))) {
		fprintf(stderr, "Cannot rewrite serial integer at %08x\n",
			AO_SERIAL_NUMBER);
		return false;
	}

	if (AO_USB_DESCRIPTORS) {
		uint32_t	usb_descriptors = AO_USB_DESCRIPTORS - image->address;
		string_num = 0;

		while (image->data[usb_descriptors] != 0 && usb_descriptors < image->length) {
			if (image->data[usb_descriptors+1] == AO_USB_DESC_STRING) {
				++string_num;
				if (string_num == 4)
					break;
			}
			usb_descriptors += image->data[usb_descriptors];
		}
		if (usb_descriptors >= image->length || image->data[usb_descriptors] == 0 ) {
			fprintf(stderr, "Cannot rewrite serial string at %08x\n", AO_USB_DESCRIPTORS);
			return false;
		}

		serial_ucs2_len = image->data[usb_descriptors] - 2;
		serial_ucs2 = malloc(serial_ucs2_len);
		if (!serial_ucs2) {
			fprintf(stderr, "Malloc(%d) failed\n", serial_ucs2_len);
			return false;
		}
		s = serial;
		for (i = serial_ucs2_len / 2; i; i--) {
			serial_ucs2[i * 2 - 1] = 0;
			serial_ucs2[i * 2 - 2] = (s % 10) + '0';
			s /= 10;
		}
		if (!rewrite(image, usb_descriptors + 2 + image->address, serial_ucs2, serial_ucs2_len)) {
			fprintf (stderr, "Cannot rewrite USB descriptor at %08x\n", AO_USB_DESCRIPTORS);
			return false;
		}
	}

	if (cal && AO_RADIO_CAL) {
		cal_int[0] = cal & 0xff;
		cal_int[1] = (cal >> 8) & 0xff;
		cal_int[2] = (cal >> 16) & 0xff;
		cal_int[3] = (cal >> 24) & 0xff;

		if (!rewrite(image, AO_RADIO_CAL, cal_int, sizeof (cal_int))) {
			fprintf(stderr, "Cannot rewrite radio calibration at %08x\n", AO_RADIO_CAL);
			return false;
		}
	}
	return true;
}

static uint16_t
read_le16(uint8_t *src)
{
	return (uint16_t) src[0] | ((uint16_t) src[1] << 8);
}

bool
ao_heximage_usb_id(struct ao_hex_image *image, struct ao_usb_id *id)
{
	uint32_t	usb_descriptors;

	if (!AO_USB_DESCRIPTORS)
		return false;
	usb_descriptors = AO_USB_DESCRIPTORS - image->address;

	while (image->data[usb_descriptors] != 0 && usb_descriptors < image->length) {
		if (image->data[usb_descriptors+1] == AO_USB_DESC_DEVICE) {
			break;
		}
		usb_descriptors += image->data[usb_descriptors];
	}

	/*
	 * check to make sure there's at least 0x12 (size of a USB
	 * device descriptor) available
	 */
	if (usb_descriptors >= image->length || image->data[usb_descriptors] != 0x12)
		return false;

	id->vid = read_le16(image->data + usb_descriptors + 8);
	id->pid = read_le16(image->data + usb_descriptors + 10);

	return true;
}

uint16_t *
ao_heximage_usb_product(struct ao_hex_image *image)
{
	uint32_t	usb_descriptors;
	int		string_num;
	uint16_t	*product;
	uint8_t		product_len;

	if (!AO_USB_DESCRIPTORS)
		return NULL;
	usb_descriptors = AO_USB_DESCRIPTORS - image->address;

	string_num = 0;
	while (image->data[usb_descriptors] != 0 && usb_descriptors < image->length) {
		if (image->data[usb_descriptors+1] == AO_USB_DESC_STRING) {
			++string_num;
			if (string_num == 3)
				break;
		}
		usb_descriptors += image->data[usb_descriptors];
	}

	/*
	 * check to make sure there's at least 0x12 (size of a USB
	 * device descriptor) available
	 */
	if (usb_descriptors >= image->length || image->data[usb_descriptors] == 0)
		return NULL;

	product_len = image->data[usb_descriptors] - 2;

	if (usb_descriptors < product_len + 2)
		return NULL;

	product = malloc (product_len + 2);
	if (!product)
		return NULL;

	memcpy(product, image->data + usb_descriptors + 2, product_len);
	product[product_len/2] = 0;
	return product;
}
