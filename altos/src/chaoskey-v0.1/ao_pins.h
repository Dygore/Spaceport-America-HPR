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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define LED_PORT_ENABLE	STM_RCC_AHBENR_IOPAEN
#define LED_PORT	(&stm_gpioa)
#define LED_PIN_RED	2
#define LED_PIN_GREEN	3
#define AO_LED_RED	(1 << LED_PIN_RED)
#define AO_LED_GREEN	(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE	(AO_LED_RED | AO_LED_GREEN)

#define AO_POWER_MANAGEMENT	1
#define AO_LED_POWER		AO_LED_RED

#define HAS_BEEP	0

/* 48MHz clock based on USB */
#define AO_HSI48	1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER	1
#define AO_RCC_CFGR_PPRE_DIV	STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB				1
#define AO_USB_DIRECTIO			1
#define AO_PA11_PA12_RMP		0
#define AO_USB_DEVICE_CLASS		0xff
#define AO_USB_INTERFACE_CLASS_DATA	0xff
#define AO_USB_HAS_OUT			0
#define AO_USB_HAS_IN			1
#define AO_USB_HAS_IN2			1
#define AO_USB_HAS_INT			0
#define USE_USB_STDIO			0
#define AO_USB_SELF_POWER		0
#define AO_USB_DEVICE_ID_SERIAL		1

#define IS_FLASH_LOADER	0

/* ADC */

#define AO_ADC_PIN0_PORT	(&stm_gpioa)
#define AO_ADC_PIN0_PIN		6
#define AO_ADC_PIN0_CH		6

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_IOPAEN))

#define AO_NUM_ADC		1

/* TRNG enable */

#define AO_TRNG_ENABLE_PORT	(&stm_gpioa)
#define AO_TRNG_ENABLE_BIT	8

/* CRC */
#define AO_CRC_WIDTH	32
#define AO_CRC_INIT	0xffffffff

/* TRNG */
#define AO_LED_TRNG_COOKED	AO_LED_GREEN
#define AO_LED_TRNG_RAW		AO_LED_GREEN

/* Mode pin */
#define AO_RAW_PORT		(&stm_gpioa)
#define AO_RAW_BIT		15

#endif /* _AO_PINS_H_ */
