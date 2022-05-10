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
#define LED_PIN_RED	3
#define AO_LED_RED	(1 << LED_PIN_RED)

#define LEDS_AVAILABLE	(AO_LED_RED)

#define HAS_BEEP	0

/* 48MHz clock based on USB */
//#define AO_HSI48	1
#define AO_HSE			16000000
#define AO_RCC_CFGR_PLLMUL	STM_RCC_CFGR_PLLMUL_3
#define AO_PLLMUL		3
#define AO_PLLDIV		1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER	1
#define AO_RCC_CFGR_PPRE_DIV	STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB				1
#define AO_USB_DIRECTIO			0
#define AO_PA11_PA12_RMP		1

#define IS_FLASH_LOADER	0

#define AO_DATA_RING	16

#define HAS_ADC		0
#define HAS_ACCEL	0
#define HAS_GPS		0
#define HAS_RADIO	0
#define HAS_FLIGHT	1
#define HAS_EEPROM	1
#define HAS_LOG		1

#define AO_LOG_FORMAT		AO_LOG_FORMAT_DETHERM

#define USE_INTERNAL_FLASH	0

/* SPI */
#define HAS_SPI_1		1
#define HAS_SPI_2		0
#define SPI_1_PA5_PA6_PA7	1
#define SPI_1_PB3_PB4_PB5	1
#define SPI_1_OSPEEDR		STM_OSPEEDR_HIGH

/* MS5607 */
#define HAS_MS5607		1

#define AO_MS5607_CS_PORT	(&stm_gpioa)
#define AO_MS5607_CS_PIN	0
#define AO_MS5607_SPI_INDEX	AO_SPI_1_PB3_PB4_PB5
#define AO_MS5607_MISO_PORT	(&stm_gpiob)
#define AO_MS5607_MISO_PIN	4
#define AO_MS5607_PRIVATE_PINS	1
#define AO_MS5607_SPI_SPEED	AO_SPI_SPEED_6MHz

/* Flash */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	(&stm_gpioa)
#define AO_M25_SPI_CS_MASK	(1 << 4)
#define AO_M25_SPI_BUS		AO_SPI_1_PA5_PA6_PA7

/* PWM */

#define NUM_PWM			1
#define AO_PWM_TIMER		(&stm_tim3)
#define AO_PWM_0_GPIO		(&stm_gpiob)
#define AO_PWM_0_PIN		1
#define AO_PWM_0_CH		4
#define PWM_MAX			20000
#define AO_PWM_TIMER_ENABLE	STM_RCC_APB1ENR_TIM3EN
#define AO_PWM_TIMER_SCALE	32

/* Servo */

#define AO_SERVO_DIR_PORT	(&stm_gpiob)
#define AO_SERVO_DIR_BIT	0
#define AO_SERVO_SPEED_PWM	0

/* limit 2 */
#define AO_SERVO_LIMIT_FORE_PORT	(&stm_gpiob)
#define AO_SERVO_LIMIT_FORE_BIT		6

/* limit 1 */
#define AO_SERVO_LIMIT_BACK_PORT	(&stm_gpiob)
#define AO_SERVO_LIMIT_BACK_BIT		7

#endif /* _AO_PINS_H_ */
