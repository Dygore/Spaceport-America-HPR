/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define LED_PORT_ENABLE	STM_RCC_AHBENR_IOPBEN
#define LED_PORT	(&stm_gpiob)
#define LED_PIN_GREEN	3
#define AO_LED_GREEN	(1 << LED_PIN_GREEN)
#define AO_LED_PANIC	AO_LED_GREEN
#define AO_CMD_LEN	128
#define AO_LISP_POOL_TOTAL	3072
#define AO_LISP_SAVE	1
#define AO_STACK_SIZE	1024

#define LEDS_AVAILABLE	(AO_LED_GREEN)

#define AO_POWER_MANAGEMENT	0

/* 48MHz clock based on USB */
#define AO_HSI48	1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER	1
#define AO_RCC_CFGR_PPRE_DIV	STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB				1
#define AO_USB_DIRECTIO			0
#define AO_PA11_PA12_RMP		0
#define HAS_BEEP			1

#define BEEPER_TIMER			2
#define BEEPER_CHANNEL			4
#define BEEPER_PORT			(&stm_gpioa)
#define BEEPER_PIN			3

#define IS_FLASH_LOADER	0

#define HAS_SERIAL_2		1
#define SERIAL_2_PA2_PA15	1
#define USE_SERIAL_2_FLOW	0
#define USE_SERIAL_2_STDIN	1
#define DELAY_SERIAL_2_STDIN	0

#endif /* _AO_PINS_H_ */
