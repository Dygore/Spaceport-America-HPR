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

#include <ao.h>
#include <ao_button.h>
#include <ao_exti.h>
#include <ao_fast_timer.h>
#if AO_EVENT
#include <ao_event.h>
#define ao_button_queue(b,v)	ao_event_put_isr(AO_EVENT_BUTTON, b, v)
#else
#define ao_button_queue(b,v)
#endif

#define AO_BUTTON_DEBOUNCE_INTERVAL	AO_MS_TO_TICKS(50)

struct ao_button_state {
	AO_TICK_TYPE	time;
	uint8_t		value;
};

static struct ao_button_state	ao_button_state[AO_BUTTON_COUNT];

#define port(q)	AO_BUTTON_ ## q ## _PORT
#define bit(q) AO_BUTTON_ ## q

#ifndef AO_BUTTON_INVERTED
#define AO_BUTTON_INVERTED	1
#endif

#if AO_BUTTON_INVERTED
/* pins are inverted */
#define ao_button_value(b)	!ao_gpio_get(port(b), bit(b))
#else
#define ao_button_value(b)	ao_gpio_get(port(b), bit(b))
#endif

static uint8_t
_ao_button_get(uint8_t b)
{
	switch (b) {
#if AO_BUTTON_COUNT > 0
	case 0: return ao_button_value(0);
#endif
#if AO_BUTTON_COUNT > 1
	case 1: return ao_button_value(1);
#endif
#if AO_BUTTON_COUNT > 2
	case 2: return ao_button_value(2);
#endif
#if AO_BUTTON_COUNT > 3
	case 3: return ao_button_value(3);
#endif
#if AO_BUTTON_COUNT > 4
	case 4: return ao_button_value(4);
#endif
#if AO_BUTTON_COUNT > 5
	case 5: return ao_button_value(5);
#endif
#if AO_BUTTON_COUNT > 6
	case 6: return ao_button_value(6);
#endif
#if AO_BUTTON_COUNT > 7
	case 7: return ao_button_value(7);
#endif
#if AO_BUTTON_COUNT > 8
	case 8: return ao_button_value(8);
#endif
#if AO_BUTTON_COUNT > 9
	case 9: return ao_button_value(9);
#endif
#if AO_BUTTON_COUNT > 10
	case 10: return ao_button_value(10);
#endif
#if AO_BUTTON_COUNT > 11
	case 11: return ao_button_value(11);
#endif
#if AO_BUTTON_COUNT > 12
	case 12: return ao_button_value(12);
#endif
#if AO_BUTTON_COUNT > 13
	case 13: return ao_button_value(13);
#endif
#if AO_BUTTON_COUNT > 14
	case 14: return ao_button_value(14);
#endif
#if AO_BUTTON_COUNT > 15
	case 15: return ao_button_value(15);
#endif
	}
	return 0;
}

static void
_ao_button_check(uint8_t b)
{
	uint8_t	value = _ao_button_get(b);

	if (value != ao_button_state[b].value) {
		AO_TICK_TYPE	now = ao_time();

		if ((now - ao_button_state[b].time) >= AO_BUTTON_DEBOUNCE_INTERVAL) {
			ao_button_state[b].value = value;
			ao_button_queue(b, value);
		}
		ao_button_state[b].time = now;
	}
}

static void
_ao_button_init(uint8_t b)
{
	uint32_t	m = ao_arch_irqsave();
	uint8_t value = _ao_button_get(b);
	ao_button_state[b].value = value;
	ao_button_state[b].time = ao_time();
	ao_button_queue(b, value);
	ao_arch_irqrestore(m);

}

uint8_t
ao_button_get(uint8_t b)
{
	return ao_button_state[b].value;
}

static void
ao_button_isr(void)
{
	uint8_t	b;

	for (b = 0; b < AO_BUTTON_COUNT; b++)
		_ao_button_check(b);
}

#define init(b) do {							\
		ao_enable_input(port(b), bit(b), AO_BUTTON_MODE);	\
		_ao_button_init(b);					\
	} while (0)

void
ao_button_init(void)
{
#if AO_BUTTON_COUNT > 0
	init(0);
#endif
#if AO_BUTTON_COUNT > 1
	init(1);
#endif
#if AO_BUTTON_COUNT > 2
	init(2);
#endif
#if AO_BUTTON_COUNT > 3
	init(3);
#endif
#if AO_BUTTON_COUNT > 4
	init(4);
#endif
#if AO_BUTTON_COUNT > 5
	init(5);
#endif
#if AO_BUTTON_COUNT > 6
	init(6);
#endif
#if AO_BUTTON_COUNT > 7
	init(7);
#endif
#if AO_BUTTON_COUNT > 8
	init(8);
#endif
#if AO_BUTTON_COUNT > 9
	init(9);
#endif
#if AO_BUTTON_COUNT > 10
	init(10);
#endif
#if AO_BUTTON_COUNT > 11
	init(11);
#endif
#if AO_BUTTON_COUNT > 12
	init(12);
#endif
#if AO_BUTTON_COUNT > 13
	init(13);
#endif
#if AO_BUTTON_COUNT > 14
	init(14);
#endif
#if AO_BUTTON_COUNT > 15
	init(15);
#endif
#if AO_BUTTON_COUNT > 16
	#error too many buttons
#endif
	ao_fast_timer_init();
	ao_fast_timer_on(ao_button_isr);
}
