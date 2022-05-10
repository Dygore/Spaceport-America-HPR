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

#ifndef _AO_DRAW_INT_H_
#define _AO_DRAW_INT_H_

static inline uint32_t
ao_expand(uint32_t bits)
{
	return ~((bits & 1)-1);
}

static inline uint32_t
ao_xor(uint8_t rop, uint32_t fg)
{
	fg = ao_expand(fg);

	return (fg & ao_expand(rop >> 1)) |
		(~fg & ao_expand(rop >> 3));
}

static inline uint32_t
ao_and(uint8_t rop, uint32_t fg)
{
	fg = ao_expand(fg);

	return (fg & ao_expand(rop ^ (rop >> 1))) |
		(~fg & ao_expand((rop>>2) ^ (rop>>3)));
}

static inline uint32_t
ao_left(uint32_t bits, int16_t shift) {
	return bits >> shift;
}

static inline uint32_t
ao_right(uint32_t bits, int16_t shift) {
	return bits << shift;
}

static inline uint32_t
ao_right_mask(int16_t x) {
	if ((AO_UNIT - x) & AO_MASK)
		return ao_left(AO_ALLONES,(AO_UNIT - x) & AO_MASK);
	else
		return 0;
}

static inline uint32_t
ao_left_mask(int16_t x) {
	if (x & AO_MASK)
		return ao_right(AO_ALLONES, x & AO_MASK);
	else
		return 0;
}

static inline uint32_t
ao_bits_mask(int16_t x, int16_t w) {
	return ao_right(AO_ALLONES, x & AO_MASK) &
		ao_left(AO_ALLONES,(AO_UNIT - (x + w)) & AO_MASK);
}

#define ao_mask_bits(x,w,l,n,r) { \
    n = (w); \
    r = ao_right_mask((x)+n); \
    l = ao_left_mask(x); \
    if (l) { \
	n -= AO_UNIT - ((x) & AO_MASK); \
	if (n < 0) { \
	    n = 0; \
	    l &= r; \
	    r = 0; \
	} \
    } \
    n >>= AO_SHIFT; \
}

#define ao_clip(val,min,max) do {		\
		if (val < min) {		\
			val = min;		\
		} else if (val > max) {		\
			val = max;		\
		}				\
	} while (0)

static inline uint32_t
ao_do_mask_rrop(uint32_t dst, uint32_t and, uint32_t xor, uint32_t mask) {
	return (dst & (and | ~mask)) ^ (xor & mask);
}

static inline uint32_t
ao_do_rrop(uint32_t dst, uint32_t and, uint32_t xor) {
	return (dst & and) ^ xor;
}

void
ao_blt(uint32_t		*src_line,
       int16_t		src_stride,
       int16_t		src_x,
       uint32_t		*dst_line,
       int16_t		dst_stride,
       int16_t		dst_x,
       int16_t		width,
       int16_t		height,
       uint8_t		rop,
       uint8_t		reverse,
       uint8_t		upsidedown);

void
ao_solid(uint32_t	and,
	 uint32_t	xor,
	 uint32_t	*dst,
	 int16_t	dst_stride,
	 int16_t	dst_x,
	 int16_t	width,
	 int16_t	height);

int16_t
ao_glyph(const struct ao_bitmap	*dst,
	 int16_t		x,
	 int16_t		y,
	 uint8_t		c,
	 uint8_t		rop);

#endif /* _AO_DRAW_INT_H_ */
