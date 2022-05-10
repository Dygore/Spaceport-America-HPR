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

struct ao_dma_config {
	void		(*isr)(int index);
};

uint8_t ao_dma_done[STM_NUM_DMA];

static struct ao_dma_config ao_dma_config[STM_NUM_DMA];
static uint8_t ao_dma_allocated[STM_NUM_DMA];
static uint8_t ao_dma_mutex[STM_NUM_DMA];
static uint8_t ao_dma_active;

#define ch_mask(id)	(STM_DMA_ISR_MASK << STM_DMA_ISR(id))

static void
ao_dma_isr(uint8_t low_index, uint8_t high_index, uint32_t mask) {
	/* Get channel interrupt bits */
	uint32_t	isr = stm_dma.isr & mask;
	uint8_t		index;

	/* Ack them */
	stm_dma.ifcr = isr;
	for (index = low_index; index <= high_index; index++) {
		if (isr & ch_mask(index)) {
			if (ao_dma_config[index].isr)
				(*ao_dma_config[index].isr)(index);
			else {
				ao_dma_done[index] = 1;
				ao_wakeup(&ao_dma_done[index]);
			}
		}
	}
}

void stm_dma_ch1_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(1),
		   STM_DMA_INDEX(1),
		   ch_mask(STM_DMA_INDEX(1)));
}

void stm_dma_ch2_3_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(2),
		   STM_DMA_INDEX(3),
		   ch_mask(STM_DMA_INDEX(2)) |
		   ch_mask(STM_DMA_INDEX(3)));
}

void stm_dma_ch4_5_6_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(4), STM_DMA_INDEX(6),
		   ch_mask(STM_DMA_INDEX(4)) |
		   ch_mask(STM_DMA_INDEX(5)) |
		   ch_mask(STM_DMA_INDEX(6)));
}

void
ao_dma_set_transfer(uint8_t 		index,
		    volatile void	*peripheral,
		    void		*memory,
		    uint16_t		count,
		    uint32_t		ccr)
{
	if (ao_dma_allocated[index]) {
		if (ao_dma_mutex[index])
			ao_panic(AO_PANIC_DMA);
		ao_dma_mutex[index] = 1;
	} else
		ao_mutex_get(&ao_dma_mutex[index]);
	ao_arch_critical(
		if (ao_dma_active++ == 0)
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_DMAEN);
		);
	ao_dma_config[index].isr = NULL;
	ao_dma_done[index] = 0;
	stm_dma.channel[index].cndtr = count;
	stm_dma.channel[index].cpar = peripheral;
	stm_dma.channel[index].cmar = memory;
	stm_dma.channel[index].ccr = ccr;
}

void
ao_dma_set_isr(uint8_t index, void (*isr)(int))
{
	ao_dma_config[index].isr = isr;
}

void
ao_dma_start(uint8_t index)
{
	stm_dma.channel[index].ccr |= (1 << STM_DMA_CCR_EN);
}

void
ao_dma_done_transfer(uint8_t index)
{
	stm_dma.channel[index].ccr &= ~(1UL << STM_DMA_CCR_EN);
	ao_arch_critical(
		if (--ao_dma_active == 0)
			stm_rcc.ahbenr &= ~(1UL << STM_RCC_AHBENR_DMAEN);
		);
	if (ao_dma_allocated[index])
		ao_dma_mutex[index] = 0;
	else
		ao_mutex_put(&ao_dma_mutex[index]);
}

void
ao_dma_abort(uint8_t index)
{
	stm_dma.channel[index].ccr &= ~(1UL << STM_DMA_CCR_EN);
	ao_wakeup(&ao_dma_done[index]);
}

void
ao_dma_alloc(uint8_t index)
{
	if (ao_dma_allocated[index])
		ao_panic(AO_PANIC_DMA);
	ao_dma_allocated[index] = 1;
}

#define STM_NUM_DMA_ISR	3

void
ao_dma_init(void)
{
	int	isr_id;
	int	index;

	for (isr_id = 0; isr_id < STM_NUM_DMA_ISR; isr_id++) {
		stm_nvic_set_enable(STM_ISR_DMA_CH1_POS + isr_id);
		stm_nvic_set_priority(STM_ISR_DMA_CH1_POS + isr_id, 4);
	}
	for (index = 0; index < STM_NUM_DMA; index++) {
		ao_dma_allocated[index] = 0;
		ao_dma_mutex[index] = 0;
	}
}
