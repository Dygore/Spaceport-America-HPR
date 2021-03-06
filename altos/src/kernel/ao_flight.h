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

#ifndef _AO_FLIGHT_H_
#define _AO_FLIGHT_H_


/*
 * ao_flight.c
 */

enum ao_flight_state {
	ao_flight_startup = 0,
	ao_flight_idle = 1,
	ao_flight_pad = 2,
	ao_flight_boost = 3,
	ao_flight_fast = 4,
	ao_flight_coast = 5,
	ao_flight_drogue = 6,
	ao_flight_main = 7,
	ao_flight_landed = 8,
	ao_flight_invalid = 9,
	ao_flight_test = 10
};

extern enum ao_flight_state	ao_flight_state;
extern AO_TICK_TYPE		ao_boost_tick;
extern AO_TICK_TYPE		ao_launch_tick;
extern uint16_t			ao_motor_number;

extern uint16_t			ao_launch_time;
extern uint8_t			ao_flight_force_idle;

/* Flight thread */
void
ao_flight(void);

/* Initialize flight thread */
void
ao_flight_init(void);

/*
 * ao_flight_nano.c
 */

void
ao_flight_nano_init(void);

#endif /* _AO_FLIGHT_H_ */
