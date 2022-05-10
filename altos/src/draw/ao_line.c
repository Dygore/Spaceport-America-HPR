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

#define ao_mask(x,w)	(ao_right(AO_ALLONES,(x) & AO_MASK) & \
			 ao_left(AO_ALLONES,(FB_UNIT - ((x)+(w))) & AO_MASK))


/* out of clip region codes */
#define OUT_LEFT 0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

/* major axis for bresenham's line */
#define X_AXIS	0
#define Y_AXIS	1

/*
 * Line clipping. Clip to the box, bringing the coordinates forward while
 * preserving the actual slope and error
 *
 *
 *	X major line, clip X:
 *
 *	adjust_x = -x;
 *
 *	e += adjust_x * e1;
 *
 *	adjust_y = (e + -e3-1) / -e3;
 *
 *	e -= adjust_y / -e3;
 *
 *	X major line, clip Y:
 *
 *	adjust_y = -y;

 *
 *	e -= adjust_y / -e3;
 *
 *	adjust_x = e / e1;
 */




static void
ao_bres(const struct ao_bitmap	*dst_bitmap,
	int16_t		signdx,
	int16_t		signdy,
	int16_t		axis,
	int16_t		x1,
	int16_t		y1,
	int16_t		e,
	int16_t		e1,
	int16_t		e3,
	int16_t		len,
	uint32_t	and,
	uint32_t	xor)
{
	int16_t		stride = dst_bitmap->stride;
	uint32_t	*dst = dst_bitmap->base;
	uint32_t	mask0, mask;

	mask0 = 1;
	if (signdx < 0)
		mask0 = ao_right(1, AO_UNIT - 1);

	if (signdy < 0)
		stride = -stride;

	dst = dst + y1 * stride + (x1 >> AO_SHIFT);
	mask = ao_right(1, x1 & AO_MASK);

	while (len--) {
		/* clip each point */

		*dst = ao_do_mask_rrop(*dst, and, xor, mask);

		if (axis == X_AXIS) {
			if (signdx < 0)
				mask = ao_left(mask, 1);
			else
				mask = ao_right(mask, 1);
			if (!mask) {
				dst += signdx;
				mask = mask0;
			}
			e += e1;
			if (e >= 0) {
				dst += stride;
				e += e3;
			}
		} else {
			dst += stride;
			e += e1;
			if (e >= 0) {
				if (signdx < 0)
					mask = ao_left(mask, 1);
				else
					mask = ao_right(mask, 1);
				if (!mask) {
					dst += signdx;
					mask = mask0;
				}
				e += e3;
			}
		}
	}
}

struct ao_cc {
	int16_t	major;
	int16_t	minor;
	int16_t	sign_major;
	int16_t	sign_minor;
	int16_t	e;
	int16_t	e1;
	int16_t	e3;
	int8_t	first;
};

/* line clipping box */
struct ao_cbox {
	int16_t	maj1, min1;
	int16_t	maj2, min2;
};

/* -b <= a, so we need to make a bigger */
static int16_t
div_ceil(int32_t a, int16_t b) {
	return (a + b + b - 1) / b - 1;
}

static int16_t
div_floor_plus_one(int32_t a, int16_t b) {
	return (a + b) / b;
}

static int8_t
ao_clip_line(struct ao_cc *c, struct ao_cbox *b)
{
	int32_t	adjust_major = 0, adjust_minor = 0;

	/* Clip major axis */
	if (c->major < b->maj1) {
		if (c->sign_major <= 0)
			return false;
		adjust_major = b->maj1 - c->major;
	} else if (c->major >= b->maj2) {
		if (c->sign_major >= 0)
			return false;
		adjust_major = c->major - (b->maj2-1);
	}

	/* Clip minor axis */
	if (c->minor < b->min1) {
		if (c->sign_minor <= 0)
			return false;
		adjust_minor = b->min1 - c->minor;
	} else if (c->minor >= b->min2) {
		if (c->sign_minor >= 0)
			return false;
		adjust_minor = c->minor - (b->min2-1);
	}

	/* If unclipped, we're done */
	if (adjust_major == 0 && adjust_minor == 0)
		return true;

	/* See how much minor adjustment would happen during
	 * a major clip. This is a bit tricky because line drawing
	 * isn't symmetrical when the line passes exactly between
	 * two pixels, we have to pick which one gets drawn
	 */
	int32_t	adj_min;

	if (!c->first)
		adj_min = div_ceil(c->e + adjust_major * c->e1, -c->e3);
	else
		adj_min = div_floor_plus_one(c->e + adjust_major * c->e1, -c->e3);

	if (adj_min < adjust_minor) {
		if (c->first)
			adjust_major = div_ceil(c->e - adjust_minor * c->e3, c->e1);
		else
			adjust_major = div_floor_plus_one(c->e - adjust_minor * c->e3, c->e1);
	} else {
		adjust_minor = adj_min;
	}

	c->e += adjust_major * c->e1 + adjust_minor * c->e3;

	c->major += c->sign_major * adjust_major;
	c->minor += c->sign_minor * adjust_minor;

	return true;
}

void
ao_line(const struct ao_bitmap	*dst,
	int16_t			x1,
	int16_t			y1,
	int16_t			x2,
	int16_t			y2,
	uint32_t		fill,
	uint8_t			rop)
{
	int16_t	adx, ady;
	int16_t	e, e1, e2, e3;
	int16_t	signdx = 1, signdy = 1;
	int16_t axis;
	int16_t len;
	struct ao_cc	clip_1, clip_2;
	struct ao_cbox	cbox;

	if ((adx = x2 - x1) < 0) {
		adx = -adx;
		signdx = -1;
	}
	if ((ady = y2 - y1) < 0) {
		ady = -ady;
		signdy = -1;
	}

	if (adx > ady) {
		axis = X_AXIS;
		e1 = ady << 1;
		e2 = e1 - (adx << 1);
		e = e1 - adx;

		clip_1.major = x1;
		clip_1.minor = y1;
		clip_2.major = x2;
		clip_2.minor = y2;
		clip_1.sign_major = signdx;
		clip_1.sign_minor = signdy;

		cbox.maj1 = 0;
		cbox.maj2 = dst->width;
		cbox.min1 = 0;
		cbox.min2 = dst->height;
	} else {
		axis = Y_AXIS;
		e1 = adx << 1;
		e2 = e1 - (ady << 1);
		e = e1 - ady;

		clip_1.major = y1;
		clip_1.minor = x1;
		clip_2.major = y2;
		clip_2.minor = x2;
		clip_1.sign_major = signdy;
		clip_1.sign_minor = signdx;

		cbox.maj1 = 0;
		cbox.maj2 = dst->height;
		cbox.min1 = 0;
		cbox.min2 = dst->width;
	}

	e3 = e2 - e1;
	e = e - e1;

	clip_1.first = true;
	clip_2.first = false;
	clip_2.e = clip_1.e = e;
	clip_2.e1 = clip_1.e1 = e1;
	clip_2.e3 = clip_1.e3 = e3;
	clip_2.sign_major = -clip_1.sign_major;
	clip_2.sign_minor = -clip_1.sign_minor;

	if (!ao_clip_line(&clip_1, &cbox))
		return;

	if (!ao_clip_line(&clip_2, &cbox))
		return;

	len = clip_1.sign_major * (clip_2.major - clip_1.major) + clip_2.first;

	if (len <= 0)
		return;

	if (adx > ady) {
		x1 = clip_1.major;
		y1 = clip_1.minor;
	} else {
		x1 = clip_1.minor;
		y1 = clip_1.major;
	}
	ao_bres(dst,
		signdx,
		signdy,
		axis,
		x1,
		y1,
		clip_1.e, e1, e3, len,
		ao_and(rop, fill),
		ao_xor(rop, fill));
}
