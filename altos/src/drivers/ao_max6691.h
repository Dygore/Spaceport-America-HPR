/*
 * Copyright © 2019 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_MAX6691_H_
#define _AO_MAX6691_H_

#define AO_MAX6691_CHANNELS	4

struct ao_max6691_sample {
	struct {
		uint16_t	t_high;
		uint16_t	t_low;
	} sensor[AO_MAX6691_CHANNELS];
};

extern struct ao_max6691_sample ao_max6691_current;

void
ao_max6691_init(void);

#endif /* _AO_MAX6691_H_ */
