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
#include "ao_font.h"

const struct ao_font ao_font = {
	.width = GLYPH_WIDTH,
	.height = GLYPH_HEIGHT,
	.ascent = GLYPH_ASCENT,
	.descent = GLYPH_HEIGHT - GLYPH_ASCENT,
};

void
ao_text(const struct ao_bitmap	*dst,
	int16_t			x,
	int16_t			y,
	char			*string,
	uint32_t		fill,
	uint8_t			rop)
{
	uint32_t	src[GLYPH_HEIGHT];
	char		c;
	int		h;

	struct ao_bitmap	src_bitmap = {
		.base = src,
		.stride = 1,
		.width = GLYPH_WIDTH,
		.height = GLYPH_HEIGHT
	};

	y -= GLYPH_ASCENT;

	rop = (rop & 3) | 0x4;

	if ((fill&1) == 0)
		rop ^= 3;

	while ((c = *string++)) {
		const uint8_t	*bytes = &glyph_bytes[glyph_pos[(uint8_t) c]];

		for (h = 0; h < GLYPH_HEIGHT; h++)
			src[h] = bytes[h];

		ao_copy(dst,
			x, y, GLYPH_WIDTH, GLYPH_HEIGHT,
			&src_bitmap,
			0, 0, rop);
		x += GLYPH_WIDTH;
	}
}
