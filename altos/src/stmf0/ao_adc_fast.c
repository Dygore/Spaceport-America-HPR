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

#include <ao.h>
#include <ao_adc_fast.h>

uint16_t ao_adc_ring[AO_ADC_RING_SIZE] __attribute__((aligned(4)));

/* Maximum number of samples fetched per _ao_adc_start call */
#define AO_ADC_RING_CHUNK	(AO_ADC_RING_SIZE >> 1)

uint16_t ao_adc_ring_head, ao_adc_ring_remain;
uint16_t ao_adc_running;

/*
 * Callback from DMA ISR
 *
 * Wakeup any waiting processes, mark the DMA as done, start the ADC
 * if there's still lots of space in the ring
 */
static void ao_adc_dma_done(int index)
{
	(void) index;
	ao_adc_ring_head += ao_adc_running;
	ao_adc_ring_remain += ao_adc_running;
	if (ao_adc_ring_head == AO_ADC_RING_SIZE)
		ao_adc_ring_head = 0;
	ao_adc_running = 0;
	ao_wakeup(&ao_adc_ring_head);
	ao_dma_done_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));
	_ao_adc_start();
}

void
_ao_adc_start(void)
{
	uint16_t	*buf;
	uint16_t	count;

	if (ao_adc_running)
		return;
	count = _ao_adc_space();
	if (count == 0)
		return;
	if (count > AO_ADC_RING_CHUNK)
		count = AO_ADC_RING_CHUNK;
	ao_adc_running = count;
	buf = ao_adc_ring + ao_adc_ring_head;
	stm_adc.isr = 0;
	ao_dma_set_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1),
			    &stm_adc.dr,
			    buf,
			    count,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_HIGH << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_16 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_16 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR) |
			    (1 << STM_DMA_CCR_TCIE));

	ao_dma_set_isr(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1), ao_adc_dma_done);
	ao_dma_start(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));

	stm_adc.cr |= (1 << STM_ADC_CR_ADSTART);
}

void
ao_adc_init(void)
{
	uint32_t	chselr;

	/* Reset ADC */
	stm_rcc.apb2rstr |= (1 << STM_RCC_APB2RSTR_ADCRST);
	stm_rcc.apb2rstr &= ~(1UL << STM_RCC_APB2RSTR_ADCRST);

	/* Turn on ADC pins */
	stm_rcc.ahbenr |= AO_ADC_RCC_AHBENR;

#ifdef AO_ADC_PIN0_PORT
	stm_moder_set(AO_ADC_PIN0_PORT, AO_ADC_PIN0_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN1_PORT
	stm_moder_set(AO_ADC_PIN1_PORT, AO_ADC_PIN1_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN2_PORT
	stm_moder_set(AO_ADC_PIN2_PORT, AO_ADC_PIN2_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN3_PORT
	stm_moder_set(AO_ADC_PIN3_PORT, AO_ADC_PIN3_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN4_PORT
	stm_moder_set(AO_ADC_PIN4_PORT, AO_ADC_PIN4_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN5_PORT
	stm_moder_set(AO_ADC_PIN5_PORT, AO_ADC_PIN5_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN6_PORT
	stm_moder_set(AO_ADC_PIN6_PORT, AO_ADC_PIN6_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN7_PORT
	stm_moder_set(AO_ADC_PIN7_PORT, AO_ADC_PIN7_PIN, STM_MODER_ANALOG);
#endif
#ifdef AO_ADC_PIN24_PORT
	#error "Too many ADC ports"
#endif

	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_ADCEN);

	chselr = 0;
#if AO_NUM_ADC > 0
	chselr |= (1 << AO_ADC_PIN0_CH);
#endif
#if AO_NUM_ADC > 1
	chselr |= (1 << AO_ADC_PIN1_CH);
#endif
#if AO_NUM_ADC > 2
	chselr |= (1 << AO_ADC_PIN2_CH);
#endif
#if AO_NUM_ADC > 3
	chselr |= (1 << AO_ADC_PIN3_CH);
#endif
#if AO_NUM_ADC > 4
	chselr |= (1 << AO_ADC_PIN4_CH);
#endif
#if AO_NUM_ADC > 5
	chselr |= (1 << AO_ADC_PIN5_CH);
#endif
#if AO_NUM_ADC > 6
	chselr |= (1 << AO_ADC_PIN6_CH);
#endif
#if AO_NUM_ADC > 7
	chselr |= (1 << AO_ADC_PIN7_CH);
#endif
#if AO_NUM_ADC > 8
#error Need more ADC defines
#endif

	/* Set the clock */
	stm_adc.cfgr2 = STM_ADC_CFGR2_CKMODE_PCLK_2 << STM_ADC_CFGR2_CKMODE;

	/* Shortest sample time */
	stm_adc.smpr = STM_ADC_SMPR_SMP_1_5 << STM_ADC_SMPR_SMP;

	/* Turn off enable and start */
	stm_adc.cr &= ~((1UL << STM_ADC_CR_ADEN) | (1 << STM_ADC_CR_ADSTART));

	/* Calibrate */
	stm_adc.cr |= (1UL << STM_ADC_CR_ADCAL);
	while ((stm_adc.cr & (1UL << STM_ADC_CR_ADCAL)) != 0)
		;

	/* Enable */
	stm_adc.cr |= (1 << STM_ADC_CR_ADEN);
	while ((stm_adc.isr & (1 << STM_ADC_ISR_ADRDY)) == 0)
		;

	stm_adc.chselr = chselr;

	stm_adc.cfgr1 = ((0 << STM_ADC_CFGR1_AWDCH) |
			 (0 << STM_ADC_CFGR1_AWDEN) |
			 (0 << STM_ADC_CFGR1_AWDSGL) |
			 (0 << STM_ADC_CFGR1_DISCEN) |
			 (0 << STM_ADC_CFGR1_AUTOOFF) |
			 (0 << STM_ADC_CFGR1_WAIT) |
			 (1 << STM_ADC_CFGR1_CONT) |
			 (1 << STM_ADC_CFGR1_OVRMOD) |
			 (STM_ADC_CFGR1_EXTEN_DISABLE << STM_ADC_CFGR1_EXTEN) |
			 (0 << STM_ADC_CFGR1_ALIGN) |
			 (STM_ADC_CFGR1_RES_12 << STM_ADC_CFGR1_RES) |
			 (STM_ADC_CFGR1_SCANDIR_UP << STM_ADC_CFGR1_SCANDIR) |
			 (STM_ADC_CFGR1_DMACFG_ONESHOT << STM_ADC_CFGR1_DMACFG) |
			 (1 << STM_ADC_CFGR1_DMAEN));
	stm_adc.ccr = 0;

	/* Clear any stale status bits */
	stm_adc.isr = 0;

	/* Turn on syscfg */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SYSCFGCOMPEN);

	/* Set ADC to use DMA channel 1 (option 1) */
	stm_syscfg.cfgr1 &= ~(1UL << STM_SYSCFG_CFGR1_ADC_DMA_RMP);

	ao_dma_alloc(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));
}
