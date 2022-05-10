/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#ifndef BEEPER_CHANNEL
#error BEEPER_CHANNEL undefined
#endif

#ifndef BEEPER_TIMER
#define BEEPER_TIMER	1
#endif

#ifndef BEEPER_AFR
#define BEEPER_AFR	STM_AFR_AF2
#endif

#if BEEPER_TIMER == 1
#define timer stm_tim1
#define STM_RCC_TIMER STM_RCC_APB2ENR_TIM1EN
#define stm_rcc_enr stm_rcc.apb2enr
#endif

#if BEEPER_TIMER == 2
#define timer stm_tim2
#define STM_RCC_TIMER STM_RCC_APB1ENR_TIM2EN
#define stm_rcc_enr stm_rcc.apb1enr
#endif

#if BEEPER_TIMER == 3
#define timer stm_tim3
#define STM_RCC_TIMER STM_RCC_APB1ENR_TIM3EN
#define stm_rcc_enr stm_rcc.apb1enr
#endif

#ifndef timer
#error BEEPER_TIMER invalid
#endif

static inline void
disable(void)
{
	timer.cr1 = 0;
#if BEEPER_TIMER == 1
	timer.bdtr = 0;
#endif
	stm_rcc_enr &= ~(1UL << STM_RCC_TIMER);

	/* Disconnect the timer from the pin */
	stm_afr_set(BEEPER_PORT, BEEPER_PIN, STM_AFR_NONE);
}

void
ao_beep(uint8_t beep)
{
	if (beep == 0) {
		disable();
	} else {
		stm_rcc_enr |= (1 << STM_RCC_TIMER);

#if BEEPER_TIMER == 1
		/* Master output enable */
		stm_tim1.bdtr = (1 << STM_TIM1_BDTR_MOE);

		stm_tim1.cr2 = ((0 << STM_TIM1_CR2_TI1S) |
				(STM_TIM1_CR2_MMS_RESET << STM_TIM1_CR2_MMS) |
				(0 << STM_TIM1_CR2_CCDS));

		/* Set prescaler to match cc1111 clocks
		 */
		stm_tim1.psc = AO_TIM_CLK / 750000;

		/* 1. Select the counter clock (internal, external, prescaler).
		 *
		 * Setting SMCR to zero means use the internal clock
		 */

		stm_tim1.smcr = 0;

		/* 2. Write the desired data in the TIMx_ARR and TIMx_CCRx registers. */
		stm_tim1.arr = beep;
		stm_tim1.ccr1 = beep;

		/* 3. Set the CCxIE and/or CCxDE bits if an interrupt and/or a
		 * DMA request is to be generated.
		 */
		/* don't want this */

		/* 4. Select the output mode. For example, you must write
		 *  OCxM=011, OCxPE=0, CCxP=0 and CCxE=1 to toggle OCx output
		 *  pin when CNT matches CCRx, CCRx preload is not used, OCx
		 *  is enabled and active high.
		 */

#if BEEPER_CHANNEL == 1
		stm_tim1.ccmr1 = ((0 << STM_TIM1_CCMR1_OC2CE) |
				  (STM_TIM1_CCMR_OCM_FROZEN << STM_TIM1_CCMR1_OC2M) |
				  (0 << STM_TIM1_CCMR1_OC2PE) |
				  (0 << STM_TIM1_CCMR1_OC2FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR1_CC2S) |

				  (0 << STM_TIM1_CCMR1_OC1CE) |
				  (STM_TIM1_CCMR_OCM_TOGGLE << STM_TIM1_CCMR1_OC1M) |
				  (0 << STM_TIM1_CCMR1_OC1PE) |
				  (0 << STM_TIM1_CCMR1_OC1FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR1_CC1S));

		stm_tim1.ccer = ((0 << STM_TIM1_CCER_CC4P) |
				 (0 << STM_TIM1_CCER_CC4E) |
				 (0 << STM_TIM1_CCER_CC3NP) |
				 (0 << STM_TIM1_CCER_CC3NE) |
				 (0 << STM_TIM1_CCER_CC3P) |
				 (0 << STM_TIM1_CCER_CC3E) |
				 (0 << STM_TIM1_CCER_CC2NP) |
				 (0 << STM_TIM1_CCER_CC2NE) |
				 (0 << STM_TIM1_CCER_CC2P) |
				 (0 << STM_TIM1_CCER_CC2E) |
				 (0 << STM_TIM1_CCER_CC1NE) |
				 (0 << STM_TIM1_CCER_CC1P) |
				 (1 << STM_TIM1_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 2
		stm_tim1.ccmr1 = ((0 << STM_TIM1_CCMR1_OC2CE) |
				  (STM_TIM1_CCMR_OCM_TOGGLE << STM_TIM1_CCMR1_OC2M) |
				  (0 << STM_TIM1_CCMR1_OC2PE) |
				  (0 << STM_TIM1_CCMR1_OC2FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR1_CC2S) |

				  (0 << STM_TIM1_CCMR1_OC1CE) |
				  (STM_TIM1_CCMR_OCM_FROZEN << STM_TIM1_CCMR1_OC1M) |
				  (0 << STM_TIM1_CCMR1_OC1PE) |
				  (0 << STM_TIM1_CCMR1_OC1FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR1_CC1S));

		stm_tim1.ccer = ((0 << STM_TIM1_CCER_CC4P) |
				 (0 << STM_TIM1_CCER_CC4E) |
				 (0 << STM_TIM1_CCER_CC3NP) |
				 (0 << STM_TIM1_CCER_CC3NE) |
				 (0 << STM_TIM1_CCER_CC3P) |
				 (0 << STM_TIM1_CCER_CC3E) |
				 (0 << STM_TIM1_CCER_CC2NP) |
				 (0 << STM_TIM1_CCER_CC2NE) |
				 (0 << STM_TIM1_CCER_CC2P) |
				 (1 << STM_TIM1_CCER_CC2E) |
				 (0 << STM_TIM1_CCER_CC1NE) |
				 (0 << STM_TIM1_CCER_CC1P) |
				 (0 << STM_TIM1_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 3
		stm_tim1.ccmr2 = ((0 << STM_TIM1_CCMR2_OC4CE) |
				  (STM_TIM1_CCMR_OCM_FROZEN << STM_TIM1_CCMR2_OC4M) |
				  (0 << STM_TIM1_CCMR2_OC4PE) |
				  (0 << STM_TIM1_CCMR2_OC4FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR2_CC4S) |

				  (0 << STM_TIM1_CCMR2_OC3CE) |
				  (STM_TIM1_CCMR_OCM_TOGGLE << STM_TIM1_CCMR2_OC3M) |
				  (0 << STM_TIM1_CCMR2_OC3PE) |
				  (0 << STM_TIM1_CCMR2_OC3FE) |
				  (STM_TIM1_CCMR_CCS_OUTPUT << STM_TIM1_CCMR2_CC3S));

		stm_tim1.ccer = ((0 << STM_TIM1_CCER_CC4P) |
				 (0 << STM_TIM1_CCER_CC4E) |
				 (0 << STM_TIM1_CCER_CC3NP) |
				 (0 << STM_TIM1_CCER_CC3NE) |
				 (0 << STM_TIM1_CCER_CC3P) |
				 (1 << STM_TIM1_CCER_CC3E) |
				 (0 << STM_TIM1_CCER_CC2NP) |
				 (0 << STM_TIM1_CCER_CC2NE) |
				 (0 << STM_TIM1_CCER_CC2P) |
				 (0 << STM_TIM1_CCER_CC2E) |
				 (0 << STM_TIM1_CCER_CC1NE) |
				 (0 << STM_TIM1_CCER_CC1P) |
				 (0 << STM_TIM1_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 4
		stm_tim1.ccmr2 = ((0 << STM_TIM1_CCMR2_OC4CE) |
				  (STM_TIM1_CCMR2_OC4M_TOGGLE << STM_TIM1_CCMR2_OC4M) |
				  (0 << STM_TIM1_CCMR2_OC4PE) |
				  (0 << STM_TIM1_CCMR2_OC4FE) |
				  (STM_TIM1_CCMR2_CC4S_OUTPUT << STM_TIM1_CCMR2_CC4S) |

				  (0 << STM_TIM1_CCMR2_OC3CE) |
				  (STM_TIM1_CCMR2_OC3M_FROZEN << STM_TIM1_CCMR2_OC3M) |
				  (0 << STM_TIM1_CCMR2_OC3PE) |
				  (0 << STM_TIM1_CCMR2_OC3FE) |
				  (STM_TIM1_CCMR2_CC3S_OUTPUT << STM_TIM1_CCMR2_CC3S));

		stm_tim1.ccer = ((0 << STM_TIM1_CCER_CC4NP) |
				 (0 << STM_TIM1_CCER_CC4P) |
				 (1 << STM_TIM1_CCER_CC4E) |
				 (0 << STM_TIM1_CCER_CC3NP) |
				 (0 << STM_TIM1_CCER_CC3P) |
				 (0 << STM_TIM1_CCER_CC3E) |
				 (0 << STM_TIM1_CCER_CC2NP) |
				 (0 << STM_TIM1_CCER_CC2P) |
				 (0 << STM_TIM1_CCER_CC2E) |
				 (0 << STM_TIM1_CCER_CC1NP) |
				 (0 << STM_TIM1_CCER_CC1P) |
				 (0 << STM_TIM1_CCER_CC1E));
#endif
		/* 5. Enable the counter by setting the CEN bit in the TIMx_CR1 register. */

		stm_tim1.cr1 = ((STM_TIM1_CR1_CKD_1 << STM_TIM1_CR1_CKD) |
				(0 << STM_TIM1_CR1_ARPE) |
				(STM_TIM1_CR1_CMS_EDGE << STM_TIM1_CR1_CMS) |
				(0 << STM_TIM1_CR1_DIR) |
				(0 << STM_TIM1_CR1_OPM) |
				(0 << STM_TIM1_CR1_URS) |
				(0 << STM_TIM1_CR1_UDIS) |
				(1 << STM_TIM1_CR1_CEN));

		/* Update the values */
		stm_tim1.egr = (1 << STM_TIM1_EGR_UG);
#endif
#if BEEPER_TIMER == 2 || BEEPER_TIMER == 3

		timer.cr2 = ((0 << STM_TIM23_CR2_TI1S) |
			     (STM_TIM23_CR2_MMS_RESET << STM_TIM23_CR2_MMS) |
			     (0 << STM_TIM23_CR2_CCDS));

		/* Set prescaler to match cc1111 clocks
		 */
		timer.psc = AO_TIM_CLK / 750000;

		/* 1. Select the counter clock (internal, external, prescaler).
		 *
		 * Setting SMCR to zero means use the internal clock
		 */

		timer.smcr = 0;

		/* 2. Write the desired data in the TIMx_ARR and TIMx_CCRx registers. */
		timer.arr = beep;
		timer.ccr1 = beep;

		/* 3. Set the CCxIE and/or CCxDE bits if an interrupt and/or a
		 * DMA request is to be generated.
		 */
		/* don't want this */

		/* 4. Select the output mode. For example, you must write
		 *  OCxM=011, OCxPE=0, CCxP=0 and CCxE=1 to toggle OCx output
		 *  pin when CNT matches CCRx, CCRx preload is not used, OCx
		 *  is enabled and active high.
		 */

#if BEEPER_CHANNEL == 1
		timer.ccmr1 = ((0 << STM_TIM23_CCMR1_OC2CE) |
			       (STM_TIM23_CCMR1_OC2M_FROZEN << STM_TIM23_CCMR1_OC2M) |
			       (0 << STM_TIM23_CCMR1_OC2PE) |
			       (0 << STM_TIM23_CCMR1_OC2FE) |
			       (STM_TIM23_CCMR1_CC2S_OUTPUT << STM_TIM23_CCMR1_CC2S) |

			       (0 << STM_TIM23_CCMR1_OC1CE) |
			       (STM_TIM23_CCMR1_OC1M_TOGGLE << STM_TIM23_CCMR1_OC1M) |
			       (0 << STM_TIM23_CCMR1_OC1PE) |
			       (0 << STM_TIM23_CCMR1_OC1FE) |
			       (STM_TIM23_CCMR1_CC1S_OUTPUT << STM_TIM23_CCMR1_CC1S));

		timer.ccer = ((0 << STM_TIM23_CCER_CC4P) |
			      (0 << STM_TIM23_CCER_CC4E) |
			      (0 << STM_TIM23_CCER_CC3NP) |
			      (0 << STM_TIM23_CCER_CC3P) |
			      (0 << STM_TIM23_CCER_CC3E) |
			      (0 << STM_TIM23_CCER_CC2NP) |
			      (0 << STM_TIM23_CCER_CC2P) |
			      (0 << STM_TIM23_CCER_CC2E) |
			      (0 << STM_TIM23_CCER_CC1P) |
			      (1 << STM_TIM23_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 2
		timer.ccmr1 = ((0 << STM_TIM23_CCMR1_OC2CE) |
			       (STM_TIM23_CCMR1_OC2M_TOGGLE << STM_TIM23_CCMR1_OC2M) |
			       (0 << STM_TIM23_CCMR1_OC2PE) |
			       (0 << STM_TIM23_CCMR1_OC2FE) |
			       (STM_TIM23_CCMR1_CC2S_OUTPUT << STM_TIM23_CCMR1_CC2S) |

			       (0 << STM_TIM23_CCMR1_OC1CE) |
			       (STM_TIM23_CCMR1_OC1M_FROZEN << STM_TIM23_CCMR1_OC1M) |
			       (0 << STM_TIM23_CCMR1_OC1PE) |
			       (0 << STM_TIM23_CCMR1_OC1FE) |
			       (STM_TIM23_CCMR1_CC1S_OUTPUT << STM_TIM23_CCMR1_CC1S));

		timer.ccer = ((0 << STM_TIM23_CCER_CC4P) |
			      (0 << STM_TIM23_CCER_CC4E) |
			      (0 << STM_TIM23_CCER_CC3NP) |
			      (0 << STM_TIM23_CCER_CC3P) |
			      (0 << STM_TIM23_CCER_CC3E) |
			      (0 << STM_TIM23_CCER_CC2NP) |
			      (0 << STM_TIM23_CCER_CC2P) |
			      (1 << STM_TIM23_CCER_CC2E) |
			      (0 << STM_TIM23_CCER_CC1P) |
			      (0 << STM_TIM23_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 3
		timer.ccmr2 = ((0 << STM_TIM23_CCMR2_OC4CE) |
			       (STM_TIM23_CCMR2_OC4M_FROZEN << STM_TIM23_CCMR2_OC4M) |
			       (0 << STM_TIM23_CCMR2_OC4PE) |
			       (0 << STM_TIM23_CCMR2_OC4FE) |
			       (STM_TIM23_CCMR2_CC4S_OUTPUT << STM_TIM23_CCMR2_CC4S) |

			       (0 << STM_TIM23_CCMR2_OC3CE) |
			       (STM_TIM23_CCMR2_OC3M_TOGGLE << STM_TIM23_CCMR2_OC3M) |
			       (0 << STM_TIM23_CCMR2_OC3PE) |
			       (0 << STM_TIM23_CCMR2_OC3FE) |
			       (STM_TIM23_CCMR2_CC3S_OUTPUT << STM_TIM23_CCMR2_CC3S));

		timer.ccer = ((0 << STM_TIM23_CCER_CC4P) |
			      (0 << STM_TIM23_CCER_CC4E) |
			      (0 << STM_TIM23_CCER_CC3NP) |
			      (0 << STM_TIM23_CCER_CC3P) |
			      (1 << STM_TIM23_CCER_CC3E) |
			      (0 << STM_TIM23_CCER_CC2NP) |
			      (0 << STM_TIM23_CCER_CC2P) |
			      (0 << STM_TIM23_CCER_CC2E) |
			      (0 << STM_TIM23_CCER_CC1P) |
			      (0 << STM_TIM23_CCER_CC1E));
#endif
#if BEEPER_CHANNEL == 4
		timer.ccmr2 = ((0 << STM_TIM23_CCMR2_OC4CE) |
			       (STM_TIM23_CCMR2_OC4M_TOGGLE << STM_TIM23_CCMR2_OC4M) |
			       (0 << STM_TIM23_CCMR2_OC4PE) |
			       (0 << STM_TIM23_CCMR2_OC4FE) |
			       (STM_TIM23_CCMR2_CC4S_OUTPUT << STM_TIM23_CCMR2_CC4S) |

			       (0 << STM_TIM23_CCMR2_OC3CE) |
			       (STM_TIM23_CCMR2_OC3M_FROZEN << STM_TIM23_CCMR2_OC3M) |
			       (0 << STM_TIM23_CCMR2_OC3PE) |
			       (0 << STM_TIM23_CCMR2_OC3FE) |
			       (STM_TIM23_CCMR2_CC3S_OUTPUT << STM_TIM23_CCMR2_CC3S));

		timer.ccer = ((0 << STM_TIM23_CCER_CC4P) |
			      (1 << STM_TIM23_CCER_CC4E) |
			      (0 << STM_TIM23_CCER_CC3NP) |
			      (0 << STM_TIM23_CCER_CC3P) |
			      (0 << STM_TIM23_CCER_CC3E) |
			      (0 << STM_TIM23_CCER_CC2NP) |
			      (0 << STM_TIM23_CCER_CC2P) |
			      (0 << STM_TIM23_CCER_CC2E) |
			      (0 << STM_TIM23_CCER_CC1P) |
			      (0 << STM_TIM23_CCER_CC1E));
#endif
		/* 5. Enable the counter by setting the CEN bit in the TIMx_CR1 register. */

		timer.cr1 = ((STM_TIM23_CR1_CKD_1 << STM_TIM23_CR1_CKD) |
			     (0 << STM_TIM23_CR1_ARPE) |
			     (STM_TIM23_CR1_CMS_EDGE << STM_TIM23_CR1_CMS) |
			     (0 << STM_TIM23_CR1_DIR) |
			     (0 << STM_TIM23_CR1_OPM) |
			     (0 << STM_TIM23_CR1_URS) |
			     (0 << STM_TIM23_CR1_UDIS) |
			     (1 << STM_TIM23_CR1_CEN));

		/* Update the values */
		timer.egr = (1 << STM_TIM23_EGR_UG);

		/* Hook the timer up to the beeper pin */
		stm_afr_set(BEEPER_PORT, BEEPER_PIN, BEEPER_AFR);
#endif
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
	ao_enable_output(BEEPER_PORT, BEEPER_PIN, 0);

	/* Leave the timer off until requested */
	stm_rcc_enr &= ~(1UL << STM_RCC_TIMER);
}
