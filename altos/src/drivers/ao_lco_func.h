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

#ifndef _AO_LCO_FUNC_H_
#define _AO_LCO_FUNC_H_

#include <ao_pad.h>

int8_t
ao_lco_query(uint16_t box, struct ao_pad_query *query, uint16_t *tick_offset);

void
ao_lco_arm(uint16_t box, uint8_t channels, uint16_t tick_offset);

void
ao_lco_ignite(uint8_t cmd);

#endif /* _AO_LCO_FUNC_H_ */
