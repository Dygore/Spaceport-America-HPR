/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include <stdio.h>
#include <stm32f4.h>

#ifndef AO_STACK_SIZE
#define AO_STACK_SIZE	2048
#endif

#define AO_STACK_ALIGNMENT __attribute__ ((aligned(8)))

#define AO_PORT_TYPE	uint16_t

#define ao_arch_reboot() \
	(stm_scb.aircr = ((STM_SCB_AIRCR_VECTKEY_KEY << STM_SCB_AIRCR_VECTKEY) | \
			  (1 << STM_SCB_AIRCR_SYSRESETREQ)))

#define ao_arch_nop()		asm("nop")

#define AO_ROMCONFIG_SYMBOL __attribute__((section(".romconfig"))) const

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define

/*
 * ao_timer.c
 *
 * We'll generally use the HSE clock through the PLL
 */

#define AO_STM_NVIC_CLOCK_PRIORITY	0xf0	/* low priority for clock */

#if AO_HSE
#define AO_PLLSRC	AO_HSE
#endif

#if AO_HSI
#define AO_PLLSRC	STM_HSI_FREQ
#endif

#if AO_PLL_M
#define AO_PLLIN	(AO_PLLSRC / AO_PLL_M)
#endif

#if AO_PLL1_N

#define AO_PLL1_VCO	(AO_PLLIN * AO_PLL1_N)
#define AO_PLL1_CLK_P	(AO_PLL1_VCO / AO_PLL1_P)
#define AO_SYSCLK	(AO_PLL1_CLK_P)

# if AO_PLL1_Q
#define AO_PLL1_CLK_Q	(AO_PLL1_VCO / AO_PLL1_Q)
# endif

#else

#define AO_SYSCLK	AO_PLLSRC

#endif

#if AO_PLL2_N

#define AO_PLL2_VCO	(AO_PLLIN * AO_PLL2_N)

# if AO_PLL2_Q
#define AO_PLL2_CLK_Q	(AL_PLL2_VCO / AO_PLL2_Q)
# endif

# if AO_PLL2_R
#define AO_PLL2_CLK_R	(AL_PLL2_VCO / AO_PLL2_R)
# endif

#endif

#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)
#define AO_P1CLK	(AO_HCLK / AO_APB1_PRESCALER)
#if AO_ABP1_PRESCALER == 1
#define AO_P1_TIMER_CLK	AO_P1CLK
#else
#define AO_P1_TIMER_CLK	(AO_P1CLK * 2)
#endif
#define AO_P2CLK	(AO_HCLK / AO_APB2_PRESCALER)
#if AO_ABP2_PRESCALER == 1
#define AO_P2_TIMER_CLK	AO_P2CLK
#else
#define AO_P2_TIMER_CLK	(AO_P2CLK * 2)
#endif
#define AO_SYSTICK	(AO_HCLK)
#define AO_PANIC_DELAY_SCALE  (AO_SYSCLK / 12000000)

/* The stm32f413 implements only 4 bits of the priority fields? */

#if AO_NONMASK_INTERRUPT
#define AO_STM_NVIC_NONMASK_PRIORITY	0x00

/* Set the basepri register to this value to mask all
 * non-maskable priorities
 */
#define AO_STM_NVIC_BASEPRI_MASK	0x10
#endif

#define AO_STM_NVIC_HIGH_PRIORITY	0x40
#define AO_STM_NVIC_MED_PRIORITY	0x80
#define AO_STM_NVIC_LOW_PRIORITY	0xC0
#define AO_STM_NVIC_CLOCK_PRIORITY	0xf0

#define AO_GPIO_MODE_PULL_NONE	0
#define AO_GPIO_MODE_PULL_UP	4
#define AO_GPIO_MODE_PULL_DOWN	8

/* usart stuff */

#define AO_SERIAL_SPEED_4800	4800
#define AO_SERIAL_SPEED_9600	9600
#define AO_SERIAL_SPEED_19200	19200
#define AO_SERIAL_SPEED_57600	57600
#define AO_SERIAL_SPEED_115200	115200

#endif /* _AO_ARCH_H_ */
