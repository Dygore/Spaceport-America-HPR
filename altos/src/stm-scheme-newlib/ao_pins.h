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

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

/* Bridge SB17 on the board and use the MCO from the other chip */
#define AO_HSE		8000000
#define AO_HSE_BYPASS		1

/* PLLVCO = 96MHz (so that USB will work) */
#define AO_PLLMUL		12
#define AO_RCC_CFGR_PLLMUL	(STM_RCC_CFGR_PLLMUL_12)

/* SYSCLK = 32MHz */
#define AO_PLLDIV		3
#define AO_RCC_CFGR_PLLDIV	(STM_RCC_CFGR_PLLDIV_3)

/* HCLK = 32MHZ (CPU clock) */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* Run APB1 at HCLK/1 */
#define AO_APB1_PRESCALER	1
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE2_DIV_1

/* Run APB2 at HCLK/1 */
#define AO_APB2_PRESCALER      	1
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_1

#define HAS_SERIAL_1		0
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	1
#define SERIAL_1_PA9_PA10	0

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	1

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	1
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	1

#define HAS_SPI_1		0
#define SPI_1_PB3_PB4_PB5	1
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		0

#define HAS_USB			1
#define HAS_BEEP		0
#define PACKET_HAS_SLAVE	0

#define AO_BOOT_CHAIN		1

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_ENABLE		STM_RCC_AHBENR_GPIOBEN
#define LED_PORT		(&stm_gpiob)
#define LED_PIN_GREEN		7
#define LED_PIN_BLUE		6
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)
#define AO_LED_BLUE		(1 << LED_PIN_BLUE)
#define AO_LED_PANIC		AO_LED_BLUE

#define LEDS_AVAILABLE		(AO_LED_BLUE | AO_LED_GREEN)

#define HAS_ADC			0

#define AO_TICK_TYPE		uint32_t
#define AO_TICK_SIGNED		int32_t

#endif /* _AO_PINS_H_ */
