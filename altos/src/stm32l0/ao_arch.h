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

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include <stdio.h>
#include <stm32l0.h>

/*
 * STM32L0 definitions and code fragments for AltOS
 */

#ifndef AO_STACK_SIZE
#define AO_STACK_SIZE	256
#endif

#define HAS_USB		0

#define AO_PORT_TYPE	uint16_t

/* Various definitions to make GCC look more like SDCC */

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define
#define __interrupt(n)
#define __at(n)

#define ao_arch_reboot() \
	(stm_scb.aircr = ((STM_SCB_AIRCR_VECTKEY_KEY << STM_SCB_AIRCR_VECTKEY) | \
			  (1 << STM_SCB_AIRCR_SYSRESETREQ)))

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

/*
 * ao_romconfig.c
 */

#define AO_ROMCONFIG_SYMBOL __attribute__((section(".romconfig"))) const

#ifndef AO_SYSCLK
#if AO_HSE
#define AO_PLLSRC	AO_HSE
#else
#define AO_PLLSRC	STM_HSI_FREQ
#endif

#define AO_PLLVCO	(AO_PLLSRC * AO_PLLMUL)
#define AO_SYSCLK	(AO_PLLVCO / AO_PLLDIV)
#endif

#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)
#define AO_FCLK		AO_HCLK
#define AO_PCLK1	(AO_HCLK / AO_APB1_PRESCALER)
#define AO_PCLK2	(AO_HCLK / AO_APB2_PRESCALER)
#define AO_SYSTICK	(AO_HCLK / 8)

#if AO_APB1_PRESCALER == 1
#define AO_TIM23467_CLK		AO_PCLK1
#else
#define AO_TIM23467_CLK		(2 * AO_PCLK1)
#endif

#if AO_APB2_PRESCALER == 1
#define AO_TIM91011_CLK		AO_PCLK2
#else
#define AO_TIM91011_CLK		(2 * AO_PCLK2)
#endif

/* The stm32l implements only 4 bits of the priority fields */

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

void ao_lcd_stm_init(void);

void ao_lcd_font_init(void);

void ao_lcd_font_string(char *s);

extern const uint32_t	ao_radio_cal;

void
ao_adc_init(void);

/* ADC maximum reported value */
#define AO_ADC_MAX			4095

#define HAS_BOOT_LOADER			0

#ifndef AO_LED_TYPE
#define AO_LED_TYPE uint16_t
#endif

void
ao_timer_stop(void);

#endif /* _AO_ARCH_H_ */


