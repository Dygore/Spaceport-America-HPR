/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_POWER_H_
#define _AO_POWER_H_

#if AO_POWER_MANAGEMENT

struct ao_power {
	struct ao_power	*prev, *next;

	void (*suspend)(void *arg);
	void (*resume)(void *arg);
	void *arg;
	uint8_t	registered;
};

void
ao_power_register(struct ao_power *power);

void
ao_power_unregister(struct ao_power *power);

void
ao_power_suspend(void);

void
ao_power_resume(void);

#else /* AO_POWER_MANAGEMENT */

#define ao_power_register(power)
#define ao_power_unregister(power)
#define ao_power_suspend()
#define ao_power_resume()

#endif /* else AO_POWER_MANAGEMENT */

#endif /* _AO_POWER_H_ */
