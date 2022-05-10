/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_ADC_FAST_H_
#define _AO_ADC_FAST_H_

void
ao_adc_read(uint16_t *dest, int len);

void
ao_adc_init(void);

/* Total ring size in samples */
#define AO_ADC_RING_SIZE	1024

extern uint16_t	ao_adc_ring[AO_ADC_RING_SIZE] __attribute__((aligned(4)));

#define ao_adc_ring_step(pos,inc)	(((pos) + (inc)) & (AO_ADC_RING_SIZE - 1))

extern uint16_t	ao_adc_ring_head, ao_adc_ring_remain;
extern uint16_t	ao_adc_running;

/*
 * Place to start fetching values from
 */
static inline uint16_t
ao_adc_ring_tail(void)
{
	return (ao_adc_ring_head - ao_adc_ring_remain) & (AO_ADC_RING_SIZE - 1);
}

void
_ao_adc_start(void);

/*
 * Space available to write ADC values into
 */
static inline uint16_t
_ao_adc_space(void)
{
	/* Free to end of buffer? */
	if (ao_adc_ring_remain <= ao_adc_ring_head)
		return AO_ADC_RING_SIZE - ao_adc_ring_head;

	/* no, return just the unused entries beyond head */
	return AO_ADC_RING_SIZE - ao_adc_ring_remain;
}

static inline uint16_t
ao_adc_get(uint16_t n)
{
	ao_arch_block_interrupts();
	while (ao_adc_ring_remain < n) {
		if (!ao_adc_running)
			_ao_adc_start();
		ao_sleep(&ao_adc_ring_head);
	}
	ao_arch_release_interrupts();
	return ao_adc_ring_tail();
}

static inline void
ao_adc_ack(uint16_t n)
{
	ao_arch_block_interrupts();
	ao_adc_ring_remain -= n;
	if (!ao_adc_running)
		_ao_adc_start();
	ao_arch_release_interrupts();
}

#endif /* _AO_ADC_FAST_H_ */
