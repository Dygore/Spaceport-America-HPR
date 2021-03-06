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
#include <ao_quadrature.h>
#include <ao_exti.h>
#include <ao_fast_timer.h>
#include <ao_event.h>

int32_t ao_quadrature_count[AO_QUADRATURE_COUNT];
#ifndef AO_QUADRATURE_SINGLE_CODE
static int8_t ao_quadrature_step[AO_QUADRATURE_COUNT];
#endif

static uint8_t  ao_quadrature_state[AO_QUADRATURE_COUNT];

struct ao_debounce {
	uint8_t	state;
	uint8_t	count;
};

static struct ao_debounce ao_debounce_state[AO_QUADRATURE_COUNT][2];

#define port(q)	AO_QUADRATURE_ ## q ## _PORT
#define bita(q) AO_QUADRATURE_ ## q ## _A
#define bitb(q) AO_QUADRATURE_ ## q ## _B
#define pina(q) AO_QUADRATURE_ ## q ## _A ## _PIN
#define pinb(q) AO_QUADRATURE_ ## q ## _B ## _PIN
#define isr(q)  ao_quadrature_isr_ ## q

#ifndef AO_QUADRATURE_DEBOUNCE
#error must define AO_QUADRATURE_DEBOUNCE
#endif

static uint8_t
ao_debounce(uint8_t cur, struct ao_debounce *debounce)
{
#if AO_QUADRATURE_DEBOUNCE > 0
	if (debounce->count > 0) {
		debounce->count--;
	} else if (cur != debounce->state) {
		debounce->state = cur;
		debounce->count = AO_QUADRATURE_DEBOUNCE;
	}
	return debounce->state;
#else
	(void) debounce;
	return cur;
#endif
}

static uint16_t
ao_quadrature_read(struct stm_gpio *gpio, uint8_t pin_a, uint8_t pin_b, struct ao_debounce debounce_state[2]) {
	uint16_t	v = ~stm_gpio_get_all(gpio);
	uint8_t		a = (v >> pin_a) & 1;
	uint8_t		b = (v >> pin_b) & 1;

	a = ao_debounce(a, &debounce_state[0]);
	b = ao_debounce(b, &debounce_state[1]);

	return (uint16_t) (a | (b << 1));
}

#define _ao_quadrature_get(q)	ao_quadrature_read(port(q), bita(q), bitb(q), ao_debounce_state[q])

static void
_ao_quadrature_step(uint8_t q, int8_t step)
{
#ifndef AO_QUADRATURE_SINGLE_CODE
	ao_quadrature_step[q] += step;
	if (ao_quadrature_state[q] != 0)
		return;
	if (ao_quadrature_step[q] >= 4) {
		ao_quadrature_step[q] = 0;
		step = 1;
	} else if (ao_quadrature_step[q] <= -4) {
		ao_quadrature_step[q] = 0;
		step = -1;
	} else
		return;
#endif
	ao_quadrature_count[q] += step;
#if AO_EVENT
	ao_event_put_isr(AO_EVENT_QUADRATURE, q, step);
#endif
	ao_wakeup(&ao_quadrature_count[q]);
}

static const struct {
	uint8_t prev, next;
} ao_quadrature_steps[4] = {
	[0] { .prev = 2, .next = 1 },
	[1] { .prev = 0, .next = 3 },
	[3] { .prev = 1, .next = 2 },
	[2] { .prev = 3, .next = 0 },
};

static void
_ao_quadrature_set(uint8_t q, uint8_t new)
{
	uint8_t	old;

	ao_arch_block_interrupts();
	old = ao_quadrature_state[q];
	ao_quadrature_state[q] = new;
	ao_arch_release_interrupts();

	if (new == ao_quadrature_steps[old].next)
		_ao_quadrature_step(q, 1);
	else if (new == ao_quadrature_steps[old].prev)
		_ao_quadrature_step(q, -1);
}

static void
ao_quadrature_isr(void)
{
#if AO_QUADRATURE_COUNT > 0
	_ao_quadrature_set(0, (uint8_t) _ao_quadrature_get(0));
#endif
#if AO_QUADRATURE_COUNT > 1
	_ao_quadrature_set(1, (uint8_t) _ao_quadrature_get(1));
#endif
}

static void
_ao_quadrature_start_one(uint8_t q, uint8_t new)
{
	ao_arch_block_interrupts();
	ao_quadrature_state[q] = new;
	ao_arch_release_interrupts();
}

int32_t
ao_quadrature_poll(uint8_t q)
{
	int32_t	ret;
	ao_arch_critical(ret = ao_quadrature_count[q];);
	return ret;
}

int32_t
ao_quadrature_wait(uint8_t q)
{
	ao_sleep(&ao_quadrature_count[q]);
	return ao_quadrature_poll(q);
}

static void
ao_quadrature_test(void)
{
	uint32_t q;
	int32_t	c;
	uint8_t	s;
#ifndef AO_QUADRATURE_SINGLE_CODE
	int8_t t = 0;
#endif

	q = ao_cmd_decimal();
	if (q >= AO_QUADRATURE_COUNT)
		ao_cmd_status = ao_cmd_syntax_error;
	if (ao_cmd_status != ao_cmd_success)
		return;

	c = -10000;
	s = 0;
	while (ao_quadrature_count[q] != 10) {
		if (ao_quadrature_count[q] != c ||
#ifndef AO_QUADRATURE_SINGLE_CODE
		    ao_quadrature_step[q] != t ||
#endif
		    ao_quadrature_state[q] != s)
		{
			c = ao_quadrature_count[q];
			s = ao_quadrature_state[q];
#ifndef AO_QUADRATURE_SINGLE_CODE
			t = ao_quadrature_step[q];
			printf("step %3d ", t);
#endif
			printf ("count %3ld state %2x\n", (long) c, s);
			flush();
		}
	}
#if 0
	for (;;) {
		int32_t	c;
		flush();
		c = ao_quadrature_wait(q);
		printf ("new count %6d\n", c);
		if (c == 100)
			break;
	}
#endif
}

static const struct ao_cmds ao_quadrature_cmds[] = {
	{ ao_quadrature_test,	"q <unit>\0Test quadrature" },
	{ 0, NULL }
};

#define init(q) do {							\
		ao_enable_input(port(q), bita(q), 0);			\
		ao_enable_input(port(q), bitb(q), 0);			\
		_ao_quadrature_start_one(q, (uint8_t) _ao_quadrature_get(q)); \
	} while (0)

void
ao_quadrature_init(void)
{
#if AO_QUADRATURE_COUNT > 0
	init(0);
#endif
#if AO_QUADRATURE_COUNT > 1
	init(1);
#endif
	ao_fast_timer_init();
	ao_fast_timer_on(ao_quadrature_isr);
	ao_cmd_register(&ao_quadrature_cmds[0]);
}
