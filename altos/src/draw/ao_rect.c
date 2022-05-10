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

#include "ao.h"
#include "ao_draw.h"
#include "ao_draw_int.h"

void
ao_rect(const struct ao_bitmap	*dst,
	int16_t			x,
	int16_t			y,
	int16_t			width,
	int16_t			height,
	uint32_t		fill,
	uint8_t			rop)
{
	int16_t	x2 = x + width;
	int16_t y2 = y + height;

	ao_clip(x, 0, dst->width);
	ao_clip(x2, 0, dst->width);
	ao_clip(y, 0, dst->height);
	ao_clip(y2, 0, dst->height);

	if (x < x2 && y < y2) {
		ao_solid(ao_and(rop, fill),
			 ao_xor(rop, fill),
			 dst->base + y * dst->stride,
			 dst->stride,
			 x,
			 x2 - x,
			 y2 - y);
	}
}

