/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "ao.h"

#define LED_PORT	PORTB
#define LED_DDR		DDRB

void
ao_led_on(uint8_t colors)
{
	LED_PORT |= colors;
}

void
ao_led_off(uint8_t colors)
{
	LED_PORT &= (uint8_t) ~colors;
}

void
ao_led_set(uint8_t colors)
{
	LED_PORT = (uint8_t) ((LED_PORT & ~LEDS_AVAILABLE) | (colors & LEDS_AVAILABLE));
}

void
ao_led_toggle(uint8_t colors)
{
	LED_PORT = (uint8_t) (LED_PORT ^ (colors & LEDS_AVAILABLE));
}

void
ao_led_for(uint8_t colors, AO_TICK_TYPE ticks)
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(void)
{
	LED_PORT &= (uint8_t) ~LEDS_AVAILABLE;
	LED_DDR |= LEDS_AVAILABLE;
}
