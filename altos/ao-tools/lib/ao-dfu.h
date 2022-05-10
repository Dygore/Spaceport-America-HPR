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

#ifndef _AO_DFU_H_
#define _AO_DFU_H_

struct ao_dfu_info {
	uint16_t	bcdDevice;
	uint16_t	idProduct;
	uint16_t	idVendor;
};

#define DFU_SIGNATURE		"DfuSe"
#define DFU_SPEC_VERSION	0x011a

#define DFU_TARGET_SIGNATURE	"Target"

int
ao_dfu_write(FILE *file, struct ao_dfu_info *info, int num_image, struct ao_hex_image images[]);

#endif /* _AO_DFU_H_ */
