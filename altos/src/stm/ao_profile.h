/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_PROFILE_H_
#define _AO_PROFILE_H_

void	ao_profile_init(void);

static inline uint32_t ao_profile_tick(void) {
	uint16_t	hi, lo, second_hi;

	do {
		hi = (uint16_t) stm_tim2.cnt;
		lo = (uint16_t) stm_tim4.cnt;
		second_hi = (uint16_t) stm_tim2.cnt;
	} while (hi != second_hi);
	return ((uint32_t) hi << 16) | lo;
}

#endif
