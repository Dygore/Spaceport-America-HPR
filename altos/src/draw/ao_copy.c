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

#define bound(val,max,other) do {		\
		if (val < 0) {			\
			other -= val;		\
			val = 0;		\
		}				\
		if (val > max) {		\
			other -= (val - max);	\
			val = max;		\
		}				\
	} while (0)

#define bound2(a, max_a, b, max_b) do {		\
		bound(a, max_a, b);		\
		bound(b, max_b, a);		\
	} while (0)

void
ao_copy(const struct ao_bitmap	*dst,
	int16_t			dst_x,
	int16_t			dst_y,
	int16_t			width,
	int16_t			height,
	const struct ao_bitmap	*src,
	int16_t			src_x,
	int16_t			src_y,
	uint8_t			rop)
{
	int16_t		dst_x2 = dst_x + width, dst_y2 = dst_y + height;
	int16_t		src_x2 = src_x + width, src_y2 = src_y + height;
	uint8_t		reverse = 0;
	uint8_t		upsidedown = 0;

	bound2(dst_x, dst->width, src_x, src->width);
	bound2(dst_x2, dst->width, src_x2, src->width);
	bound2(dst_y, dst->height, src_y, src->height);
	bound2(dst_y2, dst->height, src_y2, src->height);

	if (dst == src) {
		reverse = (dst_x > src_x);
		upsidedown = (dst_y > src_y);
	}

	if (dst_x < dst_x2 && dst_y < dst_y2) {
		ao_blt(src->base + src_y * src->stride,
		       src->stride,
		       src_x,
		       dst->base + dst_y * dst->stride,
		       dst->stride,
		       dst_x,
		       dst_x2 - dst_x,
		       dst_y2 - dst_y,
		       rop,
		       reverse,
		       upsidedown);
	}
}

