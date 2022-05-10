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

#ifndef _AO_DRAW_H_
#define _AO_DRAW_H_

struct ao_bitmap {
	uint32_t	*base;
	int16_t		stride;	/* in units */
	int16_t		width;	/* in pixels */
	int16_t		height;	/* in pixels */
};

struct ao_pattern {
	uint8_t		pattern[8];
};

void
ao_copy(const struct ao_bitmap	*dst,
	int16_t			dst_x,
	int16_t			dst_y,
	int16_t			width,
	int16_t			height,
	const struct ao_bitmap	*src,
	int16_t			src_x,
	int16_t			src_y,
	uint8_t			rop);

void
ao_rect(const struct ao_bitmap	*dst,
	int16_t			x,
	int16_t			y,
	int16_t			width,
	int16_t			height,
	uint32_t		fill,
	uint8_t			rop);

void
ao_pattern(const struct ao_bitmap	*dst,
	   int16_t			x,
	   int16_t			y,
	   int16_t			width,
	   int16_t			height,
	   const struct ao_pattern	*pattern,
	   int16_t			pat_x,
	   int16_t			pat_y,
	   uint8_t			rop);

void
ao_line(const struct ao_bitmap	*dst,
	int16_t			x1,
	int16_t			y1,
	int16_t			x2,
	int16_t			y2,
	uint32_t		fill,
	uint8_t			rop);

void
ao_text(const struct ao_bitmap	*dst,
	int16_t			x,
	int16_t			y,
	char			*string,
	uint32_t		fill,
	uint8_t			rop);

struct ao_font {
	int	width;
	int	height;
	int	ascent;
	int	descent;
};

extern const struct ao_font ao_font;

#define AO_SHIFT	5
#define AO_UNIT		(1 << AO_SHIFT)
#define AO_MASK		(AO_UNIT - 1)
#define AO_ALLONES	((uint32_t) -1)

/*
 *	    dst
 *	   0   1
 *
 *	0  a   b
 *  src
 *	1  c   d
 *
 *	ROP = abcd
 */

#define AO_CLEAR         0x0	/* 0 */
#define AO_AND           0x1	/* src AND dst */
#define AO_AND_REVERSE   0x2	/* src AND NOT dst */
#define AO_COPY          0x3	/* src */
#define AO_AND_INVERTED  0x4	/* NOT src AND dst */
#define AO_NOOP          0x5	/* dst */
#define AO_XOR           0x6	/* src XOR dst */
#define AO_OR            0x7	/* src OR dst */
#define AO_NOR           0x8	/* NOT src AND NOT dst */
#define AO_EQUIV         0x9	/* NOT src XOR dst */
#define AO_INVERT        0xa	/* NOT dst */
#define AO_OR_REVERSE    0xb	/* src OR NOT dst */
#define AO_COPY_INVERTED 0xc	/* NOT src */
#define AO_OR_INVERTED   0xd	/* NOT src OR dst */
#define AO_NAND          0xe	/* NOT src OR NOT dst */
#define AO_SET           0xf	/* 1 */

#endif /* _AO_DRAW_H_ */
