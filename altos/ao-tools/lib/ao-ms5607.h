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

#ifndef _AO_MS5607_H_
#define _AO_MS5607_H_

#include <stdint.h>
#include <stdbool.h>

struct ao_ms5607_prom {
	uint16_t	reserved;
	uint16_t	sens;
	uint16_t	off;
	uint16_t	tcs;
	uint16_t	tco;
	uint16_t	tref;
	uint16_t	tempsens;
	uint16_t	crc;
};

struct ao_ms5607_sample {
	uint32_t	pres;	/* raw 24 bit sensor */
	uint32_t	temp;	/* raw 24 bit sensor */
};

struct ao_ms5607_value {
	int32_t		pres;	/* in Pa * 10 */
	int32_t		temp;	/* in °C * 100 */
};

void
ao_ms5607_convert(struct ao_ms5607_sample *sample, struct ao_ms5607_value *value,
		  struct ao_ms5607_prom *prom, bool is_ms5611);

#endif /* _AO_MS5607_H_ */
