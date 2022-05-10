/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _AO_VGA_H_
#define _AO_VGA_H_

#include "ao_draw.h"

void
ao_vga_init(void);

void
ao_vga_enable(int active);

/* Active frame buffer */
#define AO_VGA_WIDTH		320
#define AO_VGA_HEIGHT		240

/* Pad on the right so that there are zeros on the output after the line */
#define AO_VGA_HPAD		32

#define AO_VGA_STRIDE		((AO_VGA_WIDTH + AO_VGA_HPAD) >> AO_SHIFT)

extern uint32_t	ao_vga_fb[AO_VGA_STRIDE * AO_VGA_HEIGHT];

extern const struct ao_bitmap ao_vga_bitmap;

#endif /* _AO_VGA_H_ */
