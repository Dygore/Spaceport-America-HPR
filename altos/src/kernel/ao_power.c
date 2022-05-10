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

#include <ao.h>
#include <ao_power.h>

static struct ao_power	*head, *tail;

void
ao_power_register(struct ao_power *power)
{
	if (power->registered)
		return;
	power->registered = true;
	if (tail) {
		tail->next = power;
		power->prev = tail;
		tail = power;
	} else {
		head = tail = power;
	}
#ifdef AO_LED_POWER
	ao_led_on(AO_LED_POWER);
#endif
}

void
ao_power_unregister(struct ao_power *power)
{
	if (!power->registered)
		return;
	power->registered = false;
	if (power->prev)
		power->prev->next = power->next;
	else
		head = power->next;
	if (power->next)
		power->next->prev = power->prev;
	else
		tail = power->prev;
}

void
ao_power_suspend(void)
{
	struct ao_power	*p;

#ifdef AO_LED_POWER
	ao_led_off(AO_LED_POWER);
#endif
	for (p = tail; p; p = p->prev)
		p->suspend(p->arg);
}

void
ao_power_resume(void)
{
	struct ao_power	*p;

	for (p = head; p; p = p->next)
		p->resume(p->arg);
#ifdef AO_LED_POWER
	ao_led_on(AO_LED_POWER);
#endif
}

