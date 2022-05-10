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

#ifndef AO_FLIGHT_TEST
#include <ao.h>
#include <ao_data.h>
#endif

volatile struct ao_data	ao_data_ring[AO_DATA_RING];
volatile uint8_t		ao_data_head;
volatile uint8_t		ao_data_present;

#ifndef ao_data_count
void
ao_data_get(struct ao_data *packet)
{
	uint8_t	i = ao_data_ring_prev(ao_data_head);
	memcpy(packet, (void *) &ao_data_ring[i], sizeof (struct ao_data));
}
#endif

#if HAS_ACCEL
accel_t
ao_data_accel(volatile struct ao_data *packet) {
	accel_t raw;
	raw = ao_data_accel_raw(packet);
	if (ao_config.pad_orientation != AO_PAD_ORIENTATION_ANTENNA_UP)
		raw = ao_data_accel_invert(raw);
	return raw;
}
#endif
