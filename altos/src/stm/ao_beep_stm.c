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
#include "ao_beep.h"

#if BEEPER_TIMER == 2
#define stm_beeper	stm_tim2
#define RCC_BEEPER	STM_RCC_APB1ENR_TIM2EN
#define BEEPER_AFR	STM_AFR_AF1
#elif BEEPER_TIMER == 3
#define stm_beeper	stm_tim3
#define RCC_BEEPER	STM_RCC_APB1ENR_TIM3EN
#define BEEPER_AFR	STM_AFR_AF2
#elif BEEPER_TIMER == 4
#define stm_beeper	stm_tim4
#define RCC_BEEPER	STM_RCC_APB1ENR_TIM4EN
#define BEEPER_AFR	STM_AFR_AF2
#else
#error BEEPER_TIMER must be 2, 3 or 4
#endif

void
ao_beep(uint8_t beep)
{
	if (beep == 0) {
		stm_beeper.cr1 = 0;
		stm_rcc.apb1enr &= ~(1UL << RCC_BEEPER);
	} else {
		stm_rcc.apb1enr |= (1UL << RCC_BEEPER);

		stm_beeper.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
				(STM_TIM234_CR2_MMS_RESET << STM_TIM234_CR2_MMS) |
				(0 << STM_TIM234_CR2_CCDS));

		/* Set prescaler to match cc1111 clocks
		 */
		stm_beeper.psc = AO_TIM23467_CLK / 750000;

		/* 1. Select the counter clock (internal, external, prescaler).
		 *
		 * Setting SMCR to zero means use the internal clock
		 */

		stm_beeper.smcr = 0;

		/* 2. Write the desired data in the TIMx_ARR and TIMx_CCRx registers. */
		stm_beeper.arr = beep;
#if BEEPER_CHANNEL == 1
		stm_beeper.ccr1 = beep;
#elif BEEPER_CHANNEL == 2
		stm_beeper.ccr2 = beep;
#elif BEEPER_CHANNEL == 3
		stm_beeper.ccr3 = beep;
#elif BEEPER_CHANNEL == 4
		stm_beeper.ccr4 = beep;
#else
#error invalid BEEPER_CHANNEL
#endif

		/* 3. Set the CCxIE and/or CCxDE bits if an interrupt and/or a
		 * DMA request is to be generated.
		 */
		/* don't want this */

		/* 4. Select the output mode. For example, you must write
		 *  OCxM=011, OCxPE=0, CCxP=0 and CCxE=1 to toggle OCx output
		 *  pin when CNT matches CCRx, CCRx preload is not used, OCx
		 *  is enabled and active high.
		 */

#define OC1M	(BEEPER_CHANNEL == 1 ? STM_TIM234_CCMR1_OC1M_TOGGLE : STM_TIM234_CCMR1_OC1M_FROZEN)
#define OC2M	(BEEPER_CHANNEL == 2 ? STM_TIM234_CCMR1_OC2M_TOGGLE : STM_TIM234_CCMR1_OC2M_FROZEN)
#define OC3M	(BEEPER_CHANNEL == 3 ? STM_TIM234_CCMR2_OC3M_TOGGLE : STM_TIM234_CCMR2_OC3M_FROZEN)
#define OC4M	(BEEPER_CHANNEL == 4 ? STM_TIM234_CCMR2_OC4M_TOGGLE : STM_TIM234_CCMR2_OC4M_FROZEN)

#define CCER(n)	(BEEPER_CHANNEL == (n) ? 1 : 0)

#if BEEPER_CHANNEL == 1 || BEEPER_CHANNEL == 2
		stm_beeper.ccmr1 = ((0 << STM_TIM234_CCMR1_OC2CE) |
				    (OC2M << STM_TIM234_CCMR1_OC2M) |
				    (0 << STM_TIM234_CCMR1_OC2PE) |
				    (0 << STM_TIM234_CCMR1_OC2FE) |
				    (STM_TIM234_CCMR1_CC2S_OUTPUT << STM_TIM234_CCMR1_CC2S) |

				    (0 << STM_TIM234_CCMR1_OC1CE) |
				    (OC1M << STM_TIM234_CCMR1_OC1M) |
				    (0 << STM_TIM234_CCMR1_OC1PE) |
				    (0 << STM_TIM234_CCMR1_OC1FE) |
				    (STM_TIM234_CCMR1_CC1S_OUTPUT << STM_TIM234_CCMR1_CC1S));
#elif BEEPER_CHANNEL == 3 || BEEPER_CHANNEL == 4
		stm_beeper.ccmr2 = ((0 << STM_TIM234_CCMR2_OC4CE) |
				    (OC4M << STM_TIM234_CCMR2_OC4M) |
				    (0 << STM_TIM234_CCMR2_OC4PE) |
				    (0 << STM_TIM234_CCMR2_OC4FE) |
				    (STM_TIM234_CCMR2_CC4S_OUTPUT << STM_TIM234_CCMR2_CC4S) |

				    (0 << STM_TIM234_CCMR2_OC3CE) |
				    (OC3M << STM_TIM234_CCMR2_OC3M) |
				    (0 << STM_TIM234_CCMR2_OC3PE) |
				    (0 << STM_TIM234_CCMR2_OC3FE) |
				    (STM_TIM234_CCMR2_CC3S_OUTPUT << STM_TIM234_CCMR2_CC3S));
#else
#error invalid BEEPER_CHANNEL
#endif
		stm_beeper.ccer = ((0 << STM_TIM234_CCER_CC4NP) |
				   (0 << STM_TIM234_CCER_CC4P) |
				   (CCER(4) << STM_TIM234_CCER_CC4E) |
				   (0 << STM_TIM234_CCER_CC3NP) |
				   (0 << STM_TIM234_CCER_CC3P) |
				   (CCER(3) << STM_TIM234_CCER_CC3E) |
				   (0 << STM_TIM234_CCER_CC2NP) |
				   (0 << STM_TIM234_CCER_CC2P) |
				   (CCER(2) << STM_TIM234_CCER_CC2E) |
				   (0 << STM_TIM234_CCER_CC1NP) |
				   (0 << STM_TIM234_CCER_CC1P) |
				   (CCER(1) << STM_TIM234_CCER_CC1E));

		/* 5. Enable the counter by setting the CEN bit in the TIMx_CR1 register. */

		stm_beeper.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
				(0 << STM_TIM234_CR1_ARPE) |
				(STM_TIM234_CR1_CMS_EDGE << STM_TIM234_CR1_CMS) |
				(0 << STM_TIM234_CR1_DIR) |
				(0 << STM_TIM234_CR1_OPM) |
				(0 << STM_TIM234_CR1_URS) |
				(0 << STM_TIM234_CR1_UDIS) |
				(1 << STM_TIM234_CR1_CEN));

		/* Update the values */
		stm_beeper.egr = (1 << STM_TIM234_EGR_UG);
	}
}

void
ao_beep_for(uint8_t beep, AO_TICK_TYPE ticks)
{
	ao_beep(beep);
	ao_delay(ticks);
	ao_beep(0);
}

void
ao_beep_init(void)
{
	ao_enable_port(BEEPER_PORT);
	stm_afr_set(BEEPER_PORT, BEEPER_PIN, BEEPER_AFR);

	/* Leave the timer off until requested */
	stm_rcc.apb1enr &= ~(1UL << RCC_BEEPER);
}
