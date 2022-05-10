/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

AO_PORT_TYPE ao_led_enable;

void
ao_led_on(AO_PORT_TYPE colors)
{
	lpc_gpio.pin[LED_PORT] |= colors;
}

void
ao_led_off(AO_PORT_TYPE colors)
{
	lpc_gpio.pin[LED_PORT] &= ~colors;
}

void
ao_led_set(AO_PORT_TYPE colors)
{
	AO_PORT_TYPE	on = colors & LEDS_AVAILABLE;
	AO_PORT_TYPE	off = ~colors & LEDS_AVAILABLE;

	ao_led_off(off);
	ao_led_on(on);
}

void
ao_led_for(AO_PORT_TYPE colors, AO_TICK_TYPE ticks) 
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

void
ao_led_init(void)
{
	ao_enable_port(LED_PORT);
	if (LED_PORT == 0) {
		if (LEDS_AVAILABLE & (1 << 11))
			lpc_ioconf.pio0_11 = LPC_IOCONF_FUNC_PIO0_11 | (1 << LPC_IOCONF_ADMODE);
		if (LEDS_AVAILABLE & (1 << 12))
			lpc_ioconf.pio0_12 = LPC_IOCONF_FUNC_PIO0_12 | (1 << LPC_IOCONF_ADMODE);
		if (LEDS_AVAILABLE & (1 << 14))
			lpc_ioconf.pio0_14 = LPC_IOCONF_FUNC_PIO0_14 | (1 << LPC_IOCONF_ADMODE);
	}
	lpc_gpio.dir[LED_PORT] |= LEDS_AVAILABLE;
	ao_led_off(LEDS_AVAILABLE);
}
