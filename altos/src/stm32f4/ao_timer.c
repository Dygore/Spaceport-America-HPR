/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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
#include <ao_task.h>

#ifndef HAS_TICK
#define HAS_TICK 1
#endif

#if HAS_TICK || defined(AO_TIMER_HOOK)

#if HAS_TICK
volatile AO_TICK_TYPE ao_tick_count;

AO_TICK_TYPE
ao_time(void)
{
	return ao_tick_count;
}
#endif

#if AO_DATA_ALL
volatile uint8_t	ao_data_interval = 1;
volatile uint8_t	ao_data_count;
#endif

void stm_systick_isr(void)
{
	ao_validate_cur_stack();
	if (stm_systick.csr & (1 << STM_SYSTICK_CSR_COUNTFLAG)) {
#if HAS_TICK
		++ao_tick_count;
#endif
		ao_task_check_alarm();
#if AO_DATA_ALL
		if (++ao_data_count == ao_data_interval && ao_data_interval) {
			ao_data_count = 0;
#if HAS_FAKE_FLIGHT
			if (ao_fake_flight_active)
				ao_fake_flight_poll();
			else
#endif
				ao_adc_poll();
#if (AO_DATA_ALL & ~(AO_DATA_ADC))
			ao_wakeup((void *) &ao_data_count);
#endif
		}
#endif
#ifdef AO_TIMER_HOOK
		AO_TIMER_HOOK;
#endif
	}
}

#if HAS_ADC
void
ao_timer_set_adc_interval(uint8_t interval)
{
	ao_arch_critical(
		ao_data_interval = interval;
		ao_data_count = 0;
		);
}
#endif

#define SYSTICK_RELOAD ((AO_SYSTICK / 8) / 100 - 1)

void
ao_timer_init(void)
{
	stm_systick.rvr = SYSTICK_RELOAD;
	stm_systick.cvr = 0;
	stm_systick.csr = ((1 << STM_SYSTICK_CSR_ENABLE) |
			   (1 << STM_SYSTICK_CSR_TICKINT) |
			   (STM_SYSTICK_CSR_CLKSOURCE_AHB_8 << STM_SYSTICK_CSR_CLKSOURCE));
	stm_scb.shpr3 |= AO_STM_NVIC_CLOCK_PRIORITY << 24;
}

#endif

void
ao_clock_init(void)
{
	uint32_t	cfgr;
	uint32_t	pllcfgr;

	/* Switch to HSI while messing about */
	stm_rcc.cr |= (1 << STM_RCC_CR_HSION);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSIRDY)))
		ao_arch_nop();

	stm_rcc.cfgr = (stm_rcc.cfgr & ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW)) |
		(STM_RCC_CFGR_SW_HSI << STM_RCC_CFGR_SW);

	/* wait for system to switch to HSI */
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	       (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS))
		ao_arch_nop();

	/* reset everything but the HSI selection and status */
	stm_rcc.cfgr &= (uint32_t)0x0000000f;

	/* reset everything but HSI */
	stm_rcc.cr &= 0x0000ffff;

	/* Disable and clear all interrupts */
	stm_rcc.cir = 0xffff0000;

#if AO_HSE
#if AO_HSE_BYPASS
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEBYP);
#else
	stm_rcc.cr &= ~(1 << STM_RCC_CR_HSEBYP);
#endif
	/* Enable HSE clock */
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSERDY)))
		asm("nop");

#endif

	/* Set flash latency to tolerate SYSCLK */

#define FLASH_LATENCY	((AO_SYSCLK - 1) / 25000000)

	/* Enable icache, dcache and prefetch. Set latency */
	stm_flash.acr = ((1 << STM_FLASH_ACR_DCEN) |
			 (1 << STM_FLASH_ACR_ICEN) |
			 (1 << STM_FLASH_ACR_PRFTEN) |
			 (FLASH_LATENCY << STM_FLASH_ACR_LATENCY));

	/* Enable power interface clock */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_PWREN);

#if AO_SYSCLK <= 64000000
#define VOS_SCALE_MODE	STM_PWR_CR_VOS_SCALE_MODE_1
#elif AO_SYSCLK <= 84000000
#define VOS_SCALE_MODE	STM_PWR_CR_VOS_SCALE_MODE_2
#else
#define VOS_SCALE_MODE	STM_PWR_CR_VOS_SCALE_MODE_1
#endif

	/* Set voltage scale mode */
	stm_pwr.cr = ((stm_pwr.cr & ~(STM_PWR_CR_VOS_SCALE_MODE_MASK)) |
		      (VOS_SCALE_MODE << STM_PWR_CR_VOS));

	/* HCLK */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE);
	cfgr |= (AO_RCC_CFGR_HPRE_DIV << STM_RCC_CFGR_HPRE);
	stm_rcc.cfgr = cfgr;

	/* APB1 Prescaler = AO_APB1_PRESCALER */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE1_MASK << STM_RCC_CFGR_PPRE1);
	cfgr |= (AO_RCC_CFGR_PPRE1_DIV << STM_RCC_CFGR_PPRE1);
	stm_rcc.cfgr = cfgr;

	/* APB2 Prescaler = AO_APB2_PRESCALER */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE2_MASK << STM_RCC_CFGR_PPRE2);
	cfgr |= (AO_RCC_CFGR_PPRE2_DIV << STM_RCC_CFGR_PPRE2);
	stm_rcc.cfgr = cfgr;

	/* Clock configuration register DCKCFGR2; mostly make sure USB
	 * gets clocked from PLL_Q
	 */
	stm_rcc.dckcfgr2 = ((STM_RCC_DCKCFGR2_LPTIMER1SEL_APB << STM_RCC_DCKCFGR2_LPTIMER1SEL) |
			    (STM_RCC_DCKCFGR2_SDIOSEL_CK_48MHZ << STM_RCC_DCKCFGR2_SDIOSEL) |
			    (STM_RCC_DCKCFGR2_CK48MSEL_PLL_Q << STM_RCC_DCKCFGR2_CK48MSEL) |
			    (STM_RCC_DCKCFGR2_I2CFMP1SEL_APB << STM_RCC_DCKCFGR2_I2CFMP1SEL));

	/* Disable the PLL */
	stm_rcc.cr &= ~(1 << STM_RCC_CR_PLLON);
	while (stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY))
		asm("nop");

	/* PLL1VCO */
	pllcfgr = stm_rcc.pllcfgr;
	pllcfgr &= ~(STM_RCC_PLLCFGR_PLLM_MASK << STM_RCC_PLLCFGR_PLLM);
	pllcfgr &= ~(STM_RCC_PLLCFGR_PLLN_MASK << STM_RCC_PLLCFGR_PLLN);
	pllcfgr &= ~(STM_RCC_PLLCFGR_PLLP_MASK << STM_RCC_PLLCFGR_PLLP);
	pllcfgr &= ~(STM_RCC_PLLCFGR_PLLQ_MASK << STM_RCC_PLLCFGR_PLLQ);
	pllcfgr &= ~(STM_RCC_PLLCFGR_PLLR_MASK << STM_RCC_PLLCFGR_PLLR);

	pllcfgr |= (AO_PLL_M << STM_RCC_PLLCFGR_PLLM);
	pllcfgr |= (AO_PLL1_N << STM_RCC_PLLCFGR_PLLN);
#if AO_PLL1_P == 2
#define AO_RCC_PLLCFGR_PLLP	STM_RCC_PLLCFGR_PLLP_DIV_2
#endif
#if AO_PLL1_P == 4
#define AO_RCC_PLLCFGR_PLLP	STM_RCC_PLLCFGR_PLLP_DIV_4
#endif
#if AO_PLL1_P == 6
#define AO_RCC_PLLCFGR_PLLP	STM_RCC_PLLCFGR_PLLP_DIV_6
#endif
#if AO_PLL1_P == 8
#define AO_RCC_PLLCFGR_PLLP	STM_RCC_PLLCFGR_PLLP_DIV_8
#endif
	pllcfgr |= (AO_RCC_PLLCFGR_PLLP << STM_RCC_PLLCFGR_PLLP);
	pllcfgr |= (AO_PLL1_Q << STM_RCC_PLLCFGR_PLLQ);
	pllcfgr |= (AO_PLL1_R << STM_RCC_PLLCFGR_PLLR);
	/* PLL source */
	pllcfgr &= ~(1 << STM_RCC_PLLCFGR_PLLSRC);
#if AO_HSI
	pllcfgr |= (STM_RCC_PLLCFGR_PLLSRC_HSI << STM_RCC_PLLCFGR_PLLSRC);
#endif
#if AO_HSE
	pllcfgr |= (STM_RCC_PLLCFGR_PLLSRC_HSE << STM_RCC_PLLCFGR_PLLSRC);
#endif
	stm_rcc.pllcfgr = pllcfgr;

	/* Enable the PLL and wait for it */
	stm_rcc.cr |= (1 << STM_RCC_CR_PLLON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY)))
		asm("nop");

	/* Switch to the PLL for the system clock */

	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
	cfgr |= (STM_RCC_CFGR_SW_PLL << STM_RCC_CFGR_SW);
	stm_rcc.cfgr = cfgr;
	for (;;) {
		uint32_t	c, part, mask, val;

		c = stm_rcc.cfgr;
		mask = (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS);
		val = (STM_RCC_CFGR_SWS_PLL << STM_RCC_CFGR_SWS);
		part = c & mask;
		if (part == val)
			break;
	}

#if AO_HSE
	/* Disable HSI clock */
	stm_rcc.cr &= ~(1 << STM_RCC_CR_HSION);
#endif

	/* Clear reset flags */
	stm_rcc.csr |= (1 << STM_RCC_CSR_RMVF);

#if DEBUG_THE_CLOCK
	/* Output PLL clock on PA8 and SYCLK on PC9 for measurments */

	ao_enable_port(&stm_gpioa);
	stm_afr_set(&stm_gpioa, 8, STM_AFR_AF0);
	stm_moder_set(&stm_gpioa, 8, STM_MODER_ALTERNATE);
	stm_ospeedr_set(&stm_gpioa, 8, STM_OSPEEDR_HIGH);

	ao_enable_port(&stm_gpioc);
	stm_afr_set(&stm_gpioc, 9, STM_AFR_AF0);
	stm_moder_set(&stm_gpioc, 9, STM_MODER_ALTERNATE);
	stm_ospeedr_set(&stm_gpioc, 9, STM_OSPEEDR_HIGH);

	cfgr = stm_rcc.cfgr;
	cfgr &= 0x001fffff;
	cfgr |= ((0 << STM_RCC_CFGR_MCO2) |
		 (6 << STM_RCC_CFGR_MCO2PRE) |
		 (6 << STM_RCC_CFGR_MCO1PRE) |
		 (2 << STM_RCC_CFGR_MCO1));
	stm_rcc.cfgr = cfgr;
#endif
}
