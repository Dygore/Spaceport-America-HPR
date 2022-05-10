/*
 * Copyright © 2019 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "ao.h"
#include "ao_max6691.h"
#include "ao_exti.h"

#define cat2(a,b)	a ## b
#define cat(a,b)	cat2(a,b)

#if AO_MAX6691_CH != 2
#error ao_max6691 driver currently only works for timer channel 2
#endif

#define AO_MAX6691_CCR		(AO_MAX6691_TIMER->cat(ccr, AO_MAX6691_CH))

/* Two samples per channel, plus time start value and two for Tready pulse */

#define AO_MAX6691_SAMPLES	(AO_MAX6691_CHANNELS * 2 + 3)

static uint16_t	ao_max6691_raw[AO_MAX6691_SAMPLES];

static inline uint16_t
ao_max6691_t_high(int channel)
{
	return ao_max6691_raw[channel * 2 + 3] - ao_max6691_raw[channel * 2 + 2];
}

static inline uint16_t
ao_max6691_t_low(int channel)
{
	return ao_max6691_raw[channel * 2 + 4] - ao_max6691_raw[channel * 2 + 3];
}

struct ao_max6691_sample ao_max6691_current;

static void
ao_max6691_sample(void)
{
	struct stm_tim234	*tim = AO_MAX6691_TIMER;

	tim->sr = 0;

	memset(&ao_max6691_raw, '\0', sizeof (ao_max6691_raw));
	/* Get the DMA engine ready */
	ao_dma_set_transfer(AO_MAX6691_DMA,
			    &AO_MAX6691_CCR,
			    &ao_max6691_raw,
			    AO_MAX6691_SAMPLES,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_16 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_16 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR));
	ao_dma_start(AO_MAX6691_DMA);

	/* Prod the max6691 */
	ao_set_output(AO_MAX6691_GPIO, AO_MAX6691_PIN, 0);
	int i;
	for (i = 0; i < 100; i++)
		ao_arch_nop();
	ao_set_input(AO_MAX6691_GPIO, AO_MAX6691_PIN);
	for (i = 0; i < 100; i++)
		ao_arch_nop();

	/* Reset the timer count */
	tim->cnt = 0;

	/* Switch the pin to timer input mode */
	stm_afr_set(AO_MAX6691_GPIO, AO_MAX6691_PIN, STM_AFR_AF1);

	tim->ccer = ((0 << STM_TIM234_CCER_CC1E) |
		     (0 << STM_TIM234_CCER_CC1P) |
		     (0 << STM_TIM234_CCER_CC1NP) |
		     (1 << STM_TIM234_CCER_CC2E) |
		     (1 << STM_TIM234_CCER_CC2P) |
		     (1 << STM_TIM234_CCER_CC2NP) |
		     (0 << STM_TIM234_CCER_CC3E) |
		     (0 << STM_TIM234_CCER_CC3P) |
		     (0 << STM_TIM234_CCER_CC3NP) |
		     (0 << STM_TIM234_CCER_CC4E) |
		     (0 << STM_TIM234_CCER_CC4P) |
		     (0 << STM_TIM234_CCER_CC4NP));

	/* Enable event generation on channel 2 */

	tim->egr = ((0 << STM_TIM234_EGR_TG) |
		    (0 << STM_TIM234_EGR_CC4G) |
		    (0 << STM_TIM234_EGR_CC3G) |
		    (1 << STM_TIM234_EGR_CC2G) |
		    (0 << STM_TIM234_EGR_CC1G) |
		    (0 << STM_TIM234_EGR_UG));
	/* Start the timer */
	tim->cr1 |= (1 << STM_TIM234_CR1_CEN);

	ao_arch_block_interrupts();
	while (!ao_dma_done[AO_MAX6691_DMA])
		ao_sleep(&ao_dma_done[AO_MAX6691_DMA]);
	ao_arch_release_interrupts();

	/* Disable event generation */
	tim->egr = 0;

	/* Disable capture */
	tim->ccer = 0;

	/* Stop the timer */
	tim->cr1 &= ~(1 << STM_TIM234_CR1_CEN);

	/* Switch back to GPIO mode */
	stm_moder_set(AO_MAX6691_GPIO, AO_MAX6691_PIN, STM_MODER_INPUT);

	/* Mark DMA done */
	ao_dma_done_transfer(AO_MAX6691_DMA);

	for (i = 0; i < AO_MAX6691_CHANNELS; i++) {
		ao_max6691_current.sensor[i].t_high = ao_max6691_t_high(i);
		ao_max6691_current.sensor[i].t_low = ao_max6691_t_low(i);
	}
}

static void
ao_max6691(void)
{
	for (;;) {
		ao_max6691_sample();
		ao_arch_critical(AO_DATA_PRESENT(AO_DATA_MAX6691););
	}
}

static struct ao_task ao_max6691_task;

#define R_EXT	1000.0f

static void
ao_max6691_dump(void)
{
	struct ao_max6691_sample ao_max6691;

	ao_max6691 = ao_max6691_current;

	int i;
	for (i = 0; i < AO_MAX6691_CHANNELS; i++) {
		uint16_t	t_high = ao_max6691.sensor[i].t_high;
		uint16_t	t_low = ao_max6691.sensor[i].t_low;

		/*
		 * From the MAX6691 data sheet
		 *
		 *	Thigh   Vext               Rext
		 *	----- = ---- - 0.0002 = ---------- - 0.0002
		 *	Tlow    Vref            Rext - Rth
		 *
		 *	We want to find Rth given Rext and the timing values
		 *
		 *	Thigh              Rext
		 *	----- + 0.0002 = ----------
		 *	Tlow             Rext + Rth
		 *
		 *	V = (Thigh / Tlow + 0.0002)
		 *
		 *	(Rext + Rth) * V = Rext
		 *
		 *	Rext * V + Rth * V = Rext
		 *
		 *	Rth * V = Rext - Rext * V
		 *
		 *	Rth * V = Rext * (1 - V)
		 *
		 *	Rth = Rext * (1 - V) / V
		 */

		float V = (float) t_high / (float) t_low + 0.0002f;

		float Rth = R_EXT * (1 - V) / V;

		printf("max6691 channel %d: high %5u low %5u ohms: %7g\n", i, t_high, t_low, Rth);
	}
}

static const struct ao_cmds ao_max6691_cmds[] = {
	{ ao_max6691_dump, 	"q\0Thermistor test" },
	{ 0, NULL },
};


void
ao_max6691_init(void)
{
	ao_cmd_register(&ao_max6691_cmds[0]);

	struct stm_tim234	*tim = AO_MAX6691_TIMER;

	stm_rcc.apb1enr |= (1 << AO_MAX6691_TIMER_ENABLE);

	tim->cr1 = 0;
	tim->psc = (AO_TIM23467_CLK / 4000000) - 1;	/* run the timer at 4MHz */
	tim->cnt = 0;

	/*
	 * XXX This assumes we're using CH2, which is true on TeleFireOne v2.0
	 */
	tim->ccmr1 = ((STM_TIM234_CCMR1_IC2F_NONE << STM_TIM234_CCMR1_IC2F) |
		      (STM_TIM234_CCMR1_IC2PSC_NONE << STM_TIM234_CCMR1_IC2PSC) |
		      (STM_TIM234_CCMR1_CC2S_INPUT_TI2 << STM_TIM234_CCMR1_CC2S));

	tim->ccer = 0;

	tim->sr = 0;
	tim->dier = 0;
	tim->smcr = 0;
	tim->cr2 = ((0 << STM_TIM234_CR2_TI1S) |
		    (STM_TIM234_CR2_MMS_RESET<< STM_TIM234_CR2_MMS) |
		    (0 << STM_TIM234_CR2_CCDS));

	tim->dier = ((0 << STM_TIM234_DIER_TDE) |
		     (0 << STM_TIM234_DIER_CC4DE) |
		     (0 << STM_TIM234_DIER_CC3DE) |
		     (1 << STM_TIM234_DIER_CC2DE) |
		     (0 << STM_TIM234_DIER_CC1DE) |
		     (0 << STM_TIM234_DIER_TIE) |
		     (0 << STM_TIM234_DIER_CC4IE) |
		     (0 << STM_TIM234_DIER_CC3IE) |
		     (0 << STM_TIM234_DIER_CC2IE) |
		     (0 << STM_TIM234_DIER_CC1IE) |
		     (0 << STM_TIM234_DIER_UIE));

	tim->egr = 0;

	tim->cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
		    (0 << STM_TIM234_CR1_ARPE) |
		    (STM_TIM234_CR1_CMS_EDGE << STM_TIM234_CR1_CMS) |
		    (STM_TIM234_CR1_DIR_UP << STM_TIM234_CR1_DIR) |
		    (0 << STM_TIM234_CR1_OPM) |
		    (0 << STM_TIM234_CR1_URS) |
		    (0 << STM_TIM234_CR1_UDIS) |
		    (0 << STM_TIM234_CR1_CEN));

	stm_ospeedr_set(AO_MAX6691_GPIO, AO_MAX6691_PIN, STM_OSPEEDR_40MHz);
	ao_enable_input(AO_MAX6691_GPIO, AO_MAX6691_PIN, AO_EXTI_MODE_PULL_UP);

	ao_add_task(&ao_max6691_task, ao_max6691, "max6691");
}
