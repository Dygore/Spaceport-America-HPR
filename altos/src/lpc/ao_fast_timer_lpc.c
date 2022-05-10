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
#include <ao_fast_timer.h>

static void (*ao_fast_timer_callback[AO_FAST_TIMER_MAX])(void);
static uint8_t ao_fast_timer_count;
static uint8_t ao_fast_timer_users;

static void
ao_fast_timer_enable(void)
{
	lpc_ct16b0.tcr = ((1 << LPC_CT16B_TCR_CEN) |
			  (1 << LPC_CT16B_TCR_CRST));
}

static void
ao_fast_timer_disable(void)
{
	lpc_ct16b0.tcr = ((0 << LPC_CT16B_TCR_CEN) |
			  (0 << LPC_CT16B_TCR_CRST));
}

void
ao_fast_timer_on(void (*callback)(void))
{
	ao_fast_timer_callback[ao_fast_timer_count] = callback;
	if (!ao_fast_timer_count++)
		ao_fast_timer_enable();
}

void
ao_fast_timer_off(void (*callback)(void))
{
	uint8_t	n;

	for (n = 0; n < ao_fast_timer_count; n++)
		if (ao_fast_timer_callback[n] == callback) {
			for (; n < ao_fast_timer_count-1; n++) {
				ao_fast_timer_callback[n] = ao_fast_timer_callback[n+1];
			}
			if (!--ao_fast_timer_count)
				ao_fast_timer_disable();
			break;
		}
}

void lpc_ct16b0_isr(void)
{
	uint32_t	v = lpc_ct16b0.ir;
	int		i;

	lpc_ct16b0.ir = v;
	if (v & (1 << LPC_CT16B_IR_MR0INT)) {
		for (i = 0; i < ao_fast_timer_count; i++)
			(*ao_fast_timer_callback[i])();
	}
}

#ifndef FAST_TIMER_FREQ
#define FAST_TIMER_FREQ	10000
#endif

#define TIMER_FAST	(AO_LPC_SYSCLK / FAST_TIMER_FREQ)

void
ao_fast_timer_init(void)
{
	if (!ao_fast_timer_users) {

		lpc_nvic_set_enable(LPC_ISR_CT16B0_POS);
		lpc_nvic_set_priority(LPC_ISR_CT16B0_POS, AO_LPC_NVIC_CLOCK_PRIORITY);
		/* Turn on 16-bit timer CT16B0 */

		lpc_scb.sysahbclkctrl |= 1 << LPC_SCB_SYSAHBCLKCTRL_CT16B0;

		/* Disable timer */
		lpc_ct16b0.tcr = 0;

		/* scale factor 1 */
		lpc_ct16b0.pr = 0;
		lpc_ct16b0.pc = 0;

		lpc_ct16b0.mcr = ((1 << LPC_CT16B_MCR_MR0I) |
				  (1 << LPC_CT16B_MCR_MR0R));

		lpc_ct16b0.mr[0] = TIMER_FAST;

		ao_fast_timer_disable();
	}
	if (ao_fast_timer_users == AO_FAST_TIMER_MAX)
		ao_panic(AO_PANIC_FAST_TIMER);
	ao_fast_timer_users++;
}

