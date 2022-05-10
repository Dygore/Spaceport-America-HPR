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

#define NUM_DMA	7

struct ao_dma_config {
	void		(*isr)(int index);
};

uint8_t ao_dma_done[NUM_DMA];

static struct ao_dma_config ao_dma_config[NUM_DMA];
static uint8_t ao_dma_allocated[NUM_DMA];
static uint8_t ao_dma_mutex[NUM_DMA];
static uint8_t ao_dma_active;

#ifndef LEAVE_DMA_ON
static uint8_t ao_dma_active;
#endif

#define ch_mask(id)	(STM_DMA_ISR_MASK << STM_DMA_ISR(id))

static void
ao_dma_isr(uint8_t low_index, uint8_t high_index, uint32_t mask) {
	/* Get channel interrupt bits */
	uint32_t	isr = stm_dma1.isr & mask;
	uint8_t		index;

	/* Ack them */
	stm_dma1.ifcr = isr;
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

void stm_dma1_channel1_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(1),
		   STM_DMA_INDEX(1),
		   ch_mask(STM_DMA_INDEX(1)));
}

void stm_dma1_channel3_2_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(2),
		   STM_DMA_INDEX(3),
		   ch_mask(STM_DMA_INDEX(2)) |
		   ch_mask(STM_DMA_INDEX(3)));
}

void stm_dma1_channel7_4_isr(void) {
	ao_dma_isr(STM_DMA_INDEX(4), STM_DMA_INDEX(7),
		   ch_mask(STM_DMA_INDEX(4)) |
		   ch_mask(STM_DMA_INDEX(5)) |
		   ch_mask(STM_DMA_INDEX(6)) |
		   ch_mask(STM_DMA_INDEX(7)));
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
		ao_dma_mutex[index] = 0xff;
	} else
		ao_mutex_get(&ao_dma_mutex[index]);
#ifndef LEAVE_DMA_ON
	ao_arch_critical(
		if (ao_dma_active++ == 0)
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_DMA1EN);
		);
#endif
	stm_dma1.channel[index].ccr = ccr | (1 << STM_DMA_CCR_TCIE);
	stm_dma1.channel[index].cndtr = count;
	stm_dma1.channel[index].cpar = peripheral;
	stm_dma1.channel[index].cmar = memory;
	ao_dma_config[index].isr = NULL;
}

void
ao_dma_set_isr(uint8_t index, void (*isr)(int))
{
	ao_dma_config[index].isr = isr;
}

void
ao_dma_start(uint8_t index)
{
	ao_dma_done[index] = 0;
	stm_dma1.channel[index].ccr |= (1 << STM_DMA_CCR_EN);
}

void
ao_dma_done_transfer(uint8_t index)
{
	stm_dma1.channel[index].ccr &= ~(1 << STM_DMA_CCR_EN);
#ifndef LEAVE_DMA_ON
	ao_arch_critical(
		if (--ao_dma_active == 0)
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_DMA1EN);
		);
#endif
	if (ao_dma_allocated[index])
		ao_dma_mutex[index] = 0;
	else
		ao_mutex_put(&ao_dma_mutex[index]);
}

void
ao_dma_alloc(uint8_t index, uint8_t cselr)
{
	if (ao_dma_allocated[index])
		ao_panic(AO_PANIC_DMA);
	ao_dma_allocated[index] = 1;

	int shift = (index << 2);
	uint32_t mask = ~(0xf << shift);
	stm_dma1.cselr = (stm_dma1.cselr & mask) | (cselr << shift);
}

#if DEBUG
void
ao_dma_dump_cmd(void)
{
	int i;

#ifndef LEAVE_DMA_ON
	ao_arch_critical(
		if (ao_dma_active++ == 0)
			stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_DMA1EN);
		);
#endif
	printf ("isr %08x ifcr%08x\n", stm_dma1.isr, stm_dma1.ifcr);
	for (i = 0; i < NUM_DMA; i++)
		printf("%d: done %d allocated %d mutex %2d ccr %04x cndtr %04x cpar %08x cmar %08x isr %08x\n",
		       i,
		       ao_dma_done[i],
		       ao_dma_allocated[i],
		       ao_dma_mutex[i],
		       stm_dma1.channel[i].ccr,
		       stm_dma1.channel[i].cndtr,
		       stm_dma1.channel[i].cpar,
		       stm_dma1.channel[i].cmar,
		       ao_dma_config[i].isr);
#ifndef LEAVE_DMA_ON
	ao_arch_critical(
		if (--ao_dma_active == 0)
			stm_rcc.ahbenr &= ~(1 << STM_RCC_AHBENR_DMA1EN);
		);
#endif
}

static const struct ao_cmds ao_dma_cmds[] = {
	{ ao_dma_dump_cmd, 	"D\0Dump DMA status" },
	{ 0, NULL }
};
#endif

void
ao_dma_init(void)
{
	int	index;

#ifdef LEAVE_DMA_ON
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_DMA1EN);
#endif
	for (index = 0; index < STM_NUM_DMA; index++) {
		stm_nvic_set_enable(STM_ISR_DMA1_CHANNEL1_POS + index);
		stm_nvic_set_priority(STM_ISR_DMA1_CHANNEL1_POS + index,
				      AO_STM_NVIC_MED_PRIORITY);
		ao_dma_allocated[index] = 0;
		ao_dma_mutex[index] = 0;
	}
#if DEBUG
	ao_cmd_register(&ao_dma_cmds[0]);
#endif
}
