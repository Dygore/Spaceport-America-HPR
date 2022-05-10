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
#include <stm32f0.h>

/*
 * STM32F0 definitions and code fragments for AltOS
 */

#ifndef AO_STACK_SIZE
#define AO_STACK_SIZE	512
#endif

#define AO_LED_TYPE	uint16_t

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

#define AO_ROMCONFIG_SYMBOL __attribute__((section(".init.1"))) const
#define AO_USBCONFIG_SYMBOL __attribute__((section(".init.2"))) const

extern const uint16_t ao_romconfig_version;
extern const uint16_t ao_romconfig_check;
extern const uint16_t ao_serial_number;
extern const uint32_t ao_radio_cal;

#define ao_arch_block_interrupts()	asm("cpsid i")
#define ao_arch_release_interrupts()	asm("cpsie i")

/*
 * ao_timer.c
 *
 * For the stm32f042, we want to use the USB-based HSI48 clock
 */

#ifndef AO_SYSCLK
#if AO_HSI
#define AO_SYSCLK	STM_HSI_FREQ
#endif

#if AO_HSI48
#define AO_SYSCLK	48000000
#endif
#endif

#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)

#if AO_HSE || AO_HSI

#if AO_HSE
#define AO_PLLSRC	AO_HSE
#else
#define AO_PLLSRC	STM_HSI_FREQ
#endif

#define AO_PLLVCO	(AO_PLLSRC * AO_PLLMUL)
#define AO_SYSCLK	(AO_PLLVCO / AO_PLLDIV)

#endif

#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)
#define AO_PCLK		(AO_HCLK / AO_APB_PRESCALER)
#define AO_SYSTICK	(AO_HCLK)
#define AO_PANIC_DELAY_SCALE  (AO_SYSCLK / 12000000)

#if AO_APB_PRESCALER == 1
#define AO_TIM_CLK		AO_PCLK
#else
#define AO_TIM_CLK		(2 * AO_PCLK)
#endif

#define AO_STM_NVIC_HIGH_PRIORITY	(0 << 6)
#define AO_STM_NVIC_CLOCK_PRIORITY	(1 << 6)
#define AO_STM_NVIC_MED_PRIORITY	(2 << 6)
#define AO_STM_NVIC_LOW_PRIORITY	(3 << 6)

void ao_lcd_stm_init(void);

void ao_lcd_font_init(void);

void ao_lcd_font_string(char *s);

extern const uint32_t	ao_radio_cal;

void
ao_adc_init(void);

/* ADC maximum reported value */
#define AO_ADC_MAX			4095

#ifndef HAS_BOOT_LOADER
#define HAS_BOOT_LOADER			1
#endif

#if HAS_BOOT_LOADER
#define AO_BOOT_APPLICATION_BASE	((uint32_t *) 0x08001000)
#ifndef AO_BOOT_APPLICATION_BOUND
#define AO_BOOT_APPLICATION_BOUND	((uint32_t *) (0x08000000 + stm_flash_size()))
#endif
#define AO_BOOT_LOADER_BASE		((uint32_t *) 0x08000000)
#endif

#endif /* _AO_ARCH_H_ */


