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

#define O 0
#define I AO_ALLONES

struct ao_merge_rop {
	uint32_t	ca1, cx1, ca2, cx2;
};

const struct ao_merge_rop ao_merge_rop[16] = {
    {O, O, O, O},               /* clear         0x0         0 */
    {I, O, O, O},               /* and           0x1         src AND dst */
    {I, O, I, O},               /* andReverse    0x2         src AND NOT dst */
    {O, O, I, O},               /* copy          0x3         src */
    {I, I, O, O},               /* andInverted   0x4         NOT src AND dst */
    {O, I, O, O},               /* noop          0x5         dst */
    {O, I, I, O},               /* xor           0x6         src XOR dst */
    {I, I, I, O},               /* or            0x7         src OR dst */
    {I, I, I, I},               /* nor           0x8         NOT src AND NOT dst */
    {O, I, I, I},               /* equiv         0x9         NOT src XOR dst */
    {O, I, O, I},               /* invert        0xa         NOT dst */
    {I, I, O, I},               /* orReverse     0xb         src OR NOT dst */
    {O, O, I, I},               /* copyInverted  0xc         NOT src */
    {I, O, I, I},               /* orInverted    0xd         NOT src OR dst */
    {I, O, O, I},               /* nand          0xe         NOT src OR NOT dst */
    {O, O, O, I},               /* set           0xf         1 */
};

#define ao_do_merge_rop(src, dst) \
    (((dst) & (((src) & _ca1) ^ _cx1)) ^ (((src) & _ca2) ^ _cx2))

#define ao_do_dst_invarient_merge_rop(src)	(((src) & _ca2) ^ _cx2)

#define ao_do_mask_merge_rop(src, dst, mask) \
    (((dst) & ((((src) & _ca1) ^ _cx1) | ~(mask))) ^ ((((src) & _ca2) ^ _cx2) & (mask)))

#define ao_dst_invarient_merge_rop()   (_ca1 == 0 && _cx1 == 0)

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
       uint8_t		upsidedown)
{
	uint32_t	*src, *dst;
	uint32_t	_ca1, _cx1, _ca2, _cx2;
	uint8_t		dst_invarient;
	uint32_t	startmask, endmask;
	int16_t		nmiddle, n;
	uint32_t	bits1, bits;
	int16_t		left_shift, right_shift;

	_ca1 = ao_merge_rop[rop].ca1;
	_cx1 = ao_merge_rop[rop].cx1;
	_ca2 = ao_merge_rop[rop].ca2;
	_cx2 = ao_merge_rop[rop].cx2;
	dst_invarient = ao_dst_invarient_merge_rop();

	if (upsidedown) {
		src_line += (height - 1) * src_stride;
		dst_line += (height - 1) * dst_stride;
		src_stride = -src_stride;
		dst_stride = -dst_stride;
	}

	ao_mask_bits(dst_x, width, startmask, nmiddle, endmask);
	if (reverse) {
		src_line += ((src_x + width - 1) >> AO_SHIFT) + 1;
		dst_line += ((dst_x + width - 1) >> AO_SHIFT) + 1;
		src_x = (src_x + width - 1) & AO_MASK;
		dst_x = (dst_x + width - 1) & AO_MASK;
	} else {
		src_line += src_x >> AO_SHIFT;
		dst_line += dst_x >> AO_SHIFT;
		src_x &= AO_MASK;
		dst_x &= AO_MASK;
	}
	if (src_x == dst_x) {
		while (height--) {
			src = src_line;
			src_line += src_stride;
			dst = dst_line;
			dst_line += dst_stride;
			if (reverse) {
				if (endmask) {
					bits = *--src;
					--dst;
					*dst = ao_do_mask_merge_rop(bits, *dst, endmask);
				}
				n = nmiddle;
				if (dst_invarient) {
					while (n--)
						*--dst = ao_do_dst_invarient_merge_rop(*--src);
				}
				else {
					while (n--) {
						bits = *--src;
						--dst;
						*dst = ao_do_merge_rop(bits, *dst);
					}
				}
				if (startmask) {
					bits = *--src;
					--dst;
					*dst = ao_do_mask_merge_rop(bits, *dst, startmask);
				}
			}
			else {
				if (startmask) {
					bits = *src++;
					*dst = ao_do_mask_merge_rop(bits, *dst, startmask);
					dst++;
				}
				n = nmiddle;
				if (dst_invarient) {
					while (n--)
						*dst++ = ao_do_dst_invarient_merge_rop(*src++);
				}
				else {
					while (n--) {
						bits = *src++;
						*dst = ao_do_merge_rop(bits, *dst);
						dst++;
					}
				}
				if (endmask) {
					bits = *src;
					*dst = ao_do_mask_merge_rop(bits, *dst, endmask);
				}
			}
		}
	} else {
		if (src_x > dst_x) {
			left_shift = src_x - dst_x;
			right_shift = AO_UNIT - left_shift;
		} else {
			right_shift = dst_x - src_x;
			left_shift = AO_UNIT - right_shift;
		}
		while (height--) {
			src = src_line;
			src_line += src_stride;
			dst = dst_line;
			dst_line += dst_stride;

			bits1 = 0;
			if (reverse) {
				if (src_x < dst_x)
					bits1 = *--src;
				if (endmask) {
					bits = ao_right(bits1, right_shift);
					if (ao_right(endmask, left_shift)) {
						bits1 = *--src;
						bits |= ao_left(bits1, left_shift);
					}
					--dst;
					*dst = ao_do_mask_merge_rop(bits, *dst, endmask);
				}
				n = nmiddle;
				if (dst_invarient) {
					while (n--) {
						bits = ao_right(bits1, right_shift);
						bits1 = *--src;
						bits |= ao_left(bits1, left_shift);
						--dst;
						*dst = ao_do_dst_invarient_merge_rop(bits);
					}
				} else {
					while (n--) {
						bits = ao_right(bits1, right_shift);
						bits1 = *--src;
						bits |= ao_left(bits1, left_shift);
						--dst;
						*dst = ao_do_merge_rop(bits, *dst);
					}
				}
				if (startmask) {
					bits = ao_right(bits1, right_shift);
					if (ao_right(startmask, left_shift)) {
						bits1 = *--src;
						bits |= ao_left(bits1, left_shift);
					}
					--dst;
					*dst = ao_do_mask_merge_rop(bits, *dst, startmask);
				}
			}
			else {
				if (src_x > dst_x)
					bits1 = *src++;
				if (startmask) {
					bits = ao_left(bits1, left_shift);
					if (ao_left(startmask, right_shift)) {
						bits1 = *src++;
						bits |= ao_right(bits1, right_shift);
					}
					*dst = ao_do_mask_merge_rop(bits, *dst, startmask);
					dst++;
				}
				n = nmiddle;
				if (dst_invarient) {
					while (n--) {
						bits = ao_left(bits1, left_shift);
						bits1 = *src++;
						bits |= ao_right(bits1, right_shift);
						*dst = ao_do_dst_invarient_merge_rop(bits);
						dst++;
					}
				}
				else {
					while (n--) {
						bits = ao_left(bits1, left_shift);
						bits1 = *src++;
						bits |= ao_right(bits1, right_shift);
						*dst = ao_do_merge_rop(bits, *dst);
						dst++;
					}
				}
				if (endmask) {
					bits = ao_left(bits1, left_shift);
					if (ao_left(endmask, right_shift)) {
						bits1 = *src;
						bits |= ao_right(bits1, right_shift);
					}
					*dst = ao_do_mask_merge_rop(bits, *dst, endmask);
				}
			}
		}
	}
}

void
ao_solid(uint32_t	and,
	 uint32_t	xor,
	 uint32_t	*dst,
	 int16_t	dst_stride,
	 int16_t	dst_x,
	 int16_t	width,
	 int16_t	height)
{
	uint32_t	startmask, endmask;
	int16_t		nmiddle;
	int16_t		n;

	dst += dst_x >> AO_SHIFT;
	dst_x &= AO_MASK;

	ao_mask_bits(dst_x, width, startmask, nmiddle, endmask);

	if (startmask)
		dst_stride--;

	dst_stride -= nmiddle;
	while (height--) {
		if (startmask) {
			*dst = ao_do_mask_rrop(*dst, and, xor, startmask);
			dst++;
		}
		n = nmiddle;
		if (!and)
			while (n--)
				*dst++ = xor;
		else
			while (n--) {
				*dst = ao_do_rrop(*dst, and, xor);
				dst++;
			}
		if (endmask)
			*dst = ao_do_mask_rrop(*dst, and, xor, endmask);
		dst += dst_stride;
	}
}
