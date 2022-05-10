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

#include "ao.h"

void
ao_led_on(AO_LED_TYPE colors)
{
#ifdef LED_PORT
	LED_PORT->bsrr = (colors & LEDS_AVAILABLE);
#else
#ifdef LED_PORT_0
	LED_PORT_0->bsrr = ((colors & LEDS_AVAILABLE) & LED_PORT_0_MASK) << LED_PORT_0_SHIFT;
#endif
#ifdef LED_PORT_1
	LED_PORT_1->bsrr = ((colors & LEDS_AVAILABLE) & LED_PORT_1_MASK) << LED_PORT_1_SHIFT;
#endif
#endif
}

void
ao_led_off(AO_LED_TYPE colors)
{
#ifdef LED_PORT
	LED_PORT->bsrr = (uint32_t) (colors & LEDS_AVAILABLE) << 16;
#else
#ifdef LED_PORT_0
	LED_PORT_0->bsrr = ((uint32_t) (colors & LEDS_AVAILABLE) & LED_PORT_0_MASK) << (LED_PORT_0_SHIFT + 16);
#endif
#ifdef LED_PORT_1
	LED_PORT_1->bsrr = ((uint32_t) (colors & LEDS_AVAILABLE) & LED_PORT_1_MASK) << (LED_PORT_1_SHIFT + 16);
#endif
#endif
}

void
ao_led_set(AO_LED_TYPE colors)
{
	AO_LED_TYPE	on = colors & LEDS_AVAILABLE;
	AO_LED_TYPE	off = (AO_LED_TYPE) (~colors & LEDS_AVAILABLE);

	ao_led_off(off);
	ao_led_on(on);
}

void
ao_led_for(AO_LED_TYPE colors, AO_TICK_TYPE ticks) 
{
	ao_led_on(colors);
	ao_delay(ticks);
	ao_led_off(colors);
}

#define init_led_pin(port, bit) do { \
		stm_moder_set(port, bit, STM_MODER_OUTPUT);		\
		stm_otyper_set(port, bit, STM_OTYPER_PUSH_PULL);	\
	} while (0)

void
ao_led_init(void)
{
	int	bit;

#ifdef LED_PORT
	stm_rcc.ahbenr |= (1 << LED_PORT_ENABLE);
	LED_PORT->odr &= (uint32_t) ~LEDS_AVAILABLE;
#else
#ifdef LED_PORT_0
	stm_rcc.ahbenr |= (1 << LED_PORT_0_ENABLE);
	LED_PORT_0->odr &= (uint32_t) ~(LEDS_AVAILABLE & LED_PORT_0_MASK) << LED_PORT_0_SHIFT;
#endif
#ifdef LED_PORT_1
	stm_rcc.ahbenr |= (1 << LED_PORT_1_ENABLE);
	LED_PORT_1->odr &= (uint32_t) ~(LEDS_AVAILABLE & LED_PORT_1_MASK) << LED_PORT_1_SHIFT;
#endif
#endif
	for (bit = 0; bit < 16; bit++) {
		if (LEDS_AVAILABLE & (1 << bit)) {
#ifdef LED_PORT
			init_led_pin(LED_PORT, bit);
#else
#ifdef LED_PORT_0
			if (LED_PORT_0_MASK & (1 << bit))
				init_led_pin(LED_PORT_0, bit + LED_PORT_0_SHIFT);
#endif
#ifdef LED_PORT_1
			if (LED_PORT_1_MASK & (1 << bit))
				init_led_pin(LED_PORT_1, bit + LED_PORT_1_SHIFT);
#endif
#endif
		}
	}
}
