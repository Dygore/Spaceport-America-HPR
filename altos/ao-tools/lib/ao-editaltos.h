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

#ifndef _AO_EDITALTOS_H_
#define _AO_EDITALTOS_H_

#include <stdint.h>
#include <stdbool.h>
#include "ao-hex.h"

extern struct ao_sym ao_symbols[];
extern int ao_num_symbols;

#define AO_USB_DESC_DEVICE		1
#define AO_USB_DESC_STRING		3

#define AO_ROMCONFIG_VERSION_INDEX	0
#define AO_ROMCONFIG_CHECK_INDEX	1
#define AO_SERIAL_NUMBER_INDEX		2
#define AO_RADIO_CAL_INDEX		3
#define AO_USB_DESCRIPTORS_INDEX	4

#define AO_ROMCONFIG_VERSION	(ao_symbols[AO_ROMCONFIG_VERSION_INDEX].addr)
#define AO_ROMCONFIG_CHECK	(ao_symbols[AO_ROMCONFIG_CHECK_INDEX].addr)
#define AO_SERIAL_NUMBER	(ao_symbols[AO_SERIAL_NUMBER_INDEX].addr)
#define AO_RADIO_CAL		(ao_symbols[AO_RADIO_CAL_INDEX].addr)
#define AO_USB_DESCRIPTORS	(ao_symbols[AO_USB_DESCRIPTORS_INDEX].addr)

struct ao_editaltos_funcs {
	uint16_t	(*get_uint16)(void *closure, uint32_t addr);
	uint32_t	(*get_uint32)(void *closure, uint32_t addr);
};

struct ao_usb_id {
	uint16_t	vid;
	uint16_t	pid;
};

bool
ao_editaltos_find_symbols(struct ao_sym *file_symbols, int num_file_symbols,
			  struct ao_sym *symbols, int num_symbols);

bool
ao_editaltos(struct ao_hex_image *image,
	     uint16_t serial,
	     uint32_t radio_cal);

bool
ao_heximage_usb_id(struct ao_hex_image *image, struct ao_usb_id *id);

uint16_t *
ao_heximage_usb_product(struct ao_hex_image *image);

#endif /* _AO_EDITALTOS_H_ */
