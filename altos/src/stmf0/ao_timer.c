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
#include <ao_task.h>
#if HAS_FAKE_FLIGHT
#include <ao_fake_flight.h>
#endif

#ifndef HAS_TICK
#define HAS_TICK 1
#endif

#if HAS_TICK
volatile AO_TICK_TYPE ao_tick_count;

AO_TICK_TYPE
ao_time(void)
{
	return ao_tick_count;
}

uint64_t
ao_time_ns(void)
{
	AO_TICK_TYPE	before, after;
	uint32_t	cvr;

	do {
		before = ao_tick_count;
		cvr = stm_systick.cvr;
		after = ao_tick_count;
	} while (before != after);

	return (uint64_t) after * (1000000000ULL / AO_HERTZ) +
		(uint64_t) cvr * (1000000000ULL / AO_SYSTICK);
}

#if AO_DATA_ALL
volatile uint8_t	ao_data_interval = 1;
volatile uint8_t	ao_data_count;
#endif

void stm_systick_isr(void)
{
	if (stm_systick.csr & (1 << STM_SYSTICK_CSR_COUNTFLAG)) {
		++ao_tick_count;
		ao_task_check_alarm();
#if AO_DATA_ALL
		if (++ao_data_count == ao_data_interval && ao_data_interval) {
			ao_data_count = 0;
#if HAS_ADC
#if HAS_FAKE_FLIGHT
			if (ao_fake_flight_active)
				ao_fake_flight_poll();
			else
#endif
				ao_adc_poll();
#endif
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

#define SYSTICK_RELOAD (AO_SYSTICK / 100 - 1)

void
ao_timer_init(void)
{
	stm_systick.csr = 0;
	stm_systick.rvr = SYSTICK_RELOAD;
	stm_systick.cvr = 0;
	stm_systick.csr = ((1 << STM_SYSTICK_CSR_ENABLE) |
			   (1 << STM_SYSTICK_CSR_TICKINT) |
			   (STM_SYSTICK_CSR_CLKSOURCE_HCLK_8 << STM_SYSTICK_CSR_CLKSOURCE));
}

#endif

#if AO_HSI48
static void
ao_clock_enable_crs(void)
{
	/* Enable crs interface clock */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_CRSEN);

	/* Disable error counter */
	stm_crs.cr = ((stm_crs.cr & (1 << 4)) |
		      (32 << STM_CRS_CR_TRIM) |
		      (0 << STM_CRS_CR_SWSYNC) |
		      (0 << STM_CRS_CR_AUTOTRIMEN) |
		      (0 << STM_CRS_CR_CEN) |
		      (0 << STM_CRS_CR_ESYNCIE) |
		      (0 << STM_CRS_CR_ERRIE) |
		      (0 << STM_CRS_CR_SYNCWARNIE) |
		      (0 << STM_CRS_CR_SYNCOKIE));

	/* Configure for USB source */
	stm_crs.cfgr = ((stm_crs.cfgr & ((1 << 30) | (1 << 27))) |
			(0 << STM_CRS_CFGR_SYNCPOL) |
			(STM_CRS_CFGR_SYNCSRC_USB << STM_CRS_CFGR_SYNCSRC) |
			(STM_CRS_CFGR_SYNCDIV_1 << STM_CRS_CFGR_SYNCDIV) |
			(0x22 << STM_CRS_CFGR_FELIM) |
			(((48000000 / 1000) - 1) << STM_CRS_CFGR_RELOAD));

	/* Enable error counter, set auto trim */
	stm_crs.cr = ((stm_crs.cr & (1 << 4)) |
		      (32 << STM_CRS_CR_TRIM) |
		      (0 << STM_CRS_CR_SWSYNC) |
		      (1 << STM_CRS_CR_AUTOTRIMEN) |
		      (1 << STM_CRS_CR_CEN) |
		      (0 << STM_CRS_CR_ESYNCIE) |
		      (0 << STM_CRS_CR_ERRIE) |
		      (0 << STM_CRS_CR_SYNCWARNIE) |
		      (0 << STM_CRS_CR_SYNCOKIE));
}
#endif

static void
ao_clock_hsi(void)
{
	stm_rcc.cr |= (1 << STM_RCC_CR_HSION);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSIRDY)))
		ao_arch_nop();

	stm_rcc.cfgr = (stm_rcc.cfgr & ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW)) |
		(STM_RCC_CFGR_SW_HSI << STM_RCC_CFGR_SW);

	/* wait for system to switch to HSI */
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS)) !=
	       (STM_RCC_CFGR_SWS_HSI << STM_RCC_CFGR_SWS))
		ao_arch_nop();

	/* reset the clock config, leaving us running on the HSI */
	stm_rcc.cfgr &= (uint32_t)0x0000000f;

	/* reset PLLON, CSSON, HSEBYP, HSEON */
	stm_rcc.cr &= 0x0000ffff;
}

static void
ao_clock_normal_start(void)
{
#if AO_HSE
	uint32_t	cfgr;
#define STM_RCC_CFGR_SWS_TARGET_CLOCK		STM_RCC_CFGR_SWS_PLL
#define STM_RCC_CFGR_SW_TARGET_CLOCK		STM_RCC_CFGR_SW_PLL
#define STM_PLLSRC				AO_HSE
#define STM_RCC_CFGR_PLLSRC_TARGET_CLOCK	STM_RCC_CFGR_PLLSRC_HSE

#if AO_HSE_BYPASS
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEBYP);
#else
	stm_rcc.cr &= ~(1UL << STM_RCC_CR_HSEBYP);
#endif
	/* Enable HSE clock */
	stm_rcc.cr |= (1 << STM_RCC_CR_HSEON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_HSERDY)))
		asm("nop");

	/* Disable the PLL */
	stm_rcc.cr &= ~(1UL << STM_RCC_CR_PLLON);
	while (stm_rcc.cr & (1UL << STM_RCC_CR_PLLRDY))
		asm("nop");

	/* Set multiplier */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PLLMUL_MASK << STM_RCC_CFGR_PLLMUL);
	cfgr |= (AO_RCC_CFGR_PLLMUL << STM_RCC_CFGR_PLLMUL);

	/* PLL source */
	cfgr &= ~(1UL << STM_RCC_CFGR_PLLSRC);
	cfgr |= (STM_RCC_CFGR_PLLSRC_TARGET_CLOCK  << STM_RCC_CFGR_PLLSRC);
	stm_rcc.cfgr = cfgr;

	/* Set pre divider */
	stm_rcc.cfgr2 = (AO_RCC_CFGR2_PLLDIV << STM_RCC_CFGR2_PREDIV);

	/* Enable the PLL and wait for it */
	stm_rcc.cr |= (1 << STM_RCC_CR_PLLON);
	while (!(stm_rcc.cr & (1 << STM_RCC_CR_PLLRDY)))
		asm("nop");

#endif

#if AO_HSI48
#define STM_RCC_CFGR_SWS_TARGET_CLOCK		STM_RCC_CFGR_SWS_HSI48
#define STM_RCC_CFGR_SW_TARGET_CLOCK		STM_RCC_CFGR_SW_HSI48

	/* Turn HSI48 clock on */
	stm_rcc.cr2 |= (1 << STM_RCC_CR2_HSI48ON);

	/* Wait for clock to stabilize */
	while ((stm_rcc.cr2 & (1 << STM_RCC_CR2_HSI48RDY)) == 0)
		ao_arch_nop();

	ao_clock_enable_crs();
#endif

#ifndef STM_RCC_CFGR_SWS_TARGET_CLOCK
#define STM_HSI 				16000000
#define STM_RCC_CFGR_SWS_TARGET_CLOCK		STM_RCC_CFGR_SWS_HSI
#define STM_RCC_CFGR_SW_TARGET_CLOCK		STM_RCC_CFGR_SW_HSI
#define STM_PLLSRC				STM_HSI
#define STM_RCC_CFGR_PLLSRC_TARGET_CLOCK	0
#endif


}

static void
ao_clock_normal_switch(void)
{
	uint32_t	cfgr;

	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_SW_MASK << STM_RCC_CFGR_SW);
	cfgr |= (STM_RCC_CFGR_SW_TARGET_CLOCK << STM_RCC_CFGR_SW);
	stm_rcc.cfgr = cfgr;
	for (;;) {
		uint32_t	c, part, mask, val;

		c = stm_rcc.cfgr;
		mask = (STM_RCC_CFGR_SWS_MASK << STM_RCC_CFGR_SWS);
		val = (STM_RCC_CFGR_SWS_TARGET_CLOCK << STM_RCC_CFGR_SWS);
		part = c & mask;
		if (part == val)
			break;
	}
#if !AO_HSI && !AO_NEED_HSI
	/* Turn off the HSI clock */
	stm_rcc.cr &= ~(1UL << STM_RCC_CR_HSION);
#endif
#ifdef STM_PLLSRC
	/* USB PLL source */
	stm_rcc.cfgr3 |= (1 << STM_RCC_CFGR3_USBSW);
#endif
}

void
ao_clock_init(void)
{
	uint32_t	cfgr;

	/* Switch to HSI while messing about */
	ao_clock_hsi();

	/* Disable all interrupts */
	stm_rcc.cir = 0;

	/* Start high speed clock */
	ao_clock_normal_start();

	/* Set flash latency to tolerate 48MHz SYSCLK  -> 1 wait state */

	/* Enable prefetch */
	stm_flash.acr |= (1 << STM_FLASH_ACR_PRFTBE);

	/* Enable 1 wait state so the CPU can run at 48MHz */
	stm_flash.acr |= (STM_FLASH_ACR_LATENCY_1 << STM_FLASH_ACR_LATENCY);

	/* HCLK to 48MHz -> AHB prescaler = /1 */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE);
	cfgr |= (AO_RCC_CFGR_HPRE_DIV << STM_RCC_CFGR_HPRE);
	stm_rcc.cfgr = cfgr;
	while ((stm_rcc.cfgr & (STM_RCC_CFGR_HPRE_MASK << STM_RCC_CFGR_HPRE)) !=
	       (AO_RCC_CFGR_HPRE_DIV << STM_RCC_CFGR_HPRE))
		ao_arch_nop();

	/* APB Prescaler = AO_APB_PRESCALER */
	cfgr = stm_rcc.cfgr;
	cfgr &= ~(STM_RCC_CFGR_PPRE_MASK << STM_RCC_CFGR_PPRE);
	cfgr |= (AO_RCC_CFGR_PPRE_DIV << STM_RCC_CFGR_PPRE);
	stm_rcc.cfgr = cfgr;

	/* Switch to the desired system clock */
	ao_clock_normal_switch();

	/* Clear reset flags */
	stm_rcc.csr |= (1 << STM_RCC_CSR_RMVF);

#ifdef AO_MCO_PORT
	cfgr = stm_rcc.cfgr;

	/* Send PLL clock to MCO */
	cfgr &= ~(STM_RCC_CFGR_MCO_MASK << STM_RCC_CFGR_MCO);
	cfgr |= (STM_RCC_CFGR_MCO_PLLCLK << STM_RCC_CFGR_MCO);

	/* Divide by 1 */
	cfgr &= ~(STM_RCC_CFGR_MCOPRE_DIV_MASK << STM_RCC_CFGR_MCOPRE);
	cfgr |= (STM_RCC_CFGR_MCOPRE_DIV_1 << STM_RCC_CFGR_MCOPRE);

	/* Don't divide PLL */
	cfgr |= (1 << STM_RCC_CFGR_PLL_NODIV);

	stm_rcc.cfgr = cfgr;

	ao_enable_port(AO_MCO_PORT);
	stm_ospeedr_set(AO_MCO_PORT, AO_MCO_PIN, STM_OSPEEDR_HIGH);
	stm_afr_set(AO_MCO_PORT, AO_MCO_PIN, AO_MCO_AF);
#endif

#if DEBUG_THE_CLOCK
	/* Output SYSCLK on PA8 for measurments */

	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);

	stm_afr_set(&stm_gpioa, 8, STM_AFR_AF0);
	stm_ospeedr_set(&stm_gpioa, 8, STM_OSPEEDR_HIGH);

	stm_rcc.cfgr |= (STM_RCC_CFGR_MCOPRE_DIV_1 << STM_RCC_CFGR_MCOPRE);
	stm_rcc.cfgr |= (STM_RCC_CFGR_MCOSEL_HSE << STM_RCC_CFGR_MCOSEL);
#endif
}

#if AO_POWER_MANAGEMENT
void
ao_clock_suspend(void)
{
	ao_clock_hsi();
}

void
ao_clock_resume(void)
{
	ao_clock_normal_start();
	ao_clock_normal_switch();
}
#endif
