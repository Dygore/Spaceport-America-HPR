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

/* 8MHz High speed external crystal */
#define AO_HSE			8000000

/* PLLVCO = 96MHz (so that USB will work) */
#define AO_PLLMUL		12
#define AO_RCC_CFGR_PLLMUL	(STM_RCC_CFGR_PLLMUL_12)

#define AO_CC1200_FOSC		40000000

/* SYSCLK = 32MHz (no need to go faster than CPU) */
#define AO_PLLDIV		3
#define AO_RCC_CFGR_PLLDIV	(STM_RCC_CFGR_PLLDIV_3)

/* HCLK = 32MHz (CPU clock) */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* Run APB1 at 16MHz (HCLK/2) */
#define AO_APB1_PRESCALER	2
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE2_DIV_2

/* Run APB2 at 16MHz (HCLK/2) */
#define AO_APB2_PRESCALER	2
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_2

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	1
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		1
#define BEEPER_CHANNEL		4
#define BEEPER_TIMER		3
#define BEEPER_PORT		(&stm_gpiob)
#define BEEPER_PIN		1
#define HAS_RADIO		1
#define HAS_RADIO_RATE		1
#define HAS_TELEMETRY		0
#define HAS_AES			1
#define HAS_FIXED_PAD_BOX	1

#define HAS_SPI_1		0
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0

#define HAS_SPI_2		1	/* CC1200 */
#define SPI_2_PB13_PB14_PB15	1
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_GPIO		(&stm_gpiod)
#define SPI_2_SCK		13
#define SPI_2_MISO		14
#define SPI_2_MOSI		15
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_I2C_1		0

#define HAS_I2C_2		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define FAST_TIMER_FREQ		10000	/* .1ms for debouncing */

/*
 * Radio is a cc1200 connected via SPI
 */

#define AO_RADIO_CAL_DEFAULT 	5695733

#define AO_FEC_DEBUG		0
#define AO_CC1200_SPI_CS_PORT	(&stm_gpioa)
#define AO_CC1200_SPI_CS_PIN	7
#define AO_CC1200_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_CC1200_SPI		stm_spi2

#define AO_CC1200_INT_PORT		(&stm_gpiob)
#define AO_CC1200_INT_PIN		(11)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0		(&stm_gpioa)
#define LED_PORT_1		(&stm_gpiob)

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_GPIOAEN
#define LED_PORT_1_ENABLE	STM_RCC_AHBENR_GPIOBEN

/* Port A, pins 4-6 */
#define LED_PORT_0_SHIFT	4
#define LED_PORT_0_MASK		0x7

#define LED_PIN_GREEN		0
#define LED_PIN_AMBER		1
#define LED_PIN_RED		2

#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_AMBER		(1 << LED_PIN_AMBER)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

/* Port B, pins 3-5 */
#define LED_PORT_1_SHIFT	0
#define LED_PORT_1_MASK		(0x7 << 3)

#define LED_PIN_CONTINUITY_1	3
#define LED_PIN_CONTINUITY_0	4
#define LED_PIN_REMOTE_ARM	5

#define AO_LED_CONTINUITY_1	(1 << LED_PIN_CONTINUITY_1)
#define AO_LED_CONTINUITY_0	(1 << LED_PIN_CONTINUITY_0)

#define AO_LED_CONTINUITY_NUM	2

#define AO_LED_REMOTE_ARM	(1 << LED_PIN_REMOTE_ARM)

#define LEDS_AVAILABLE		(AO_LED_RED |		\
				 AO_LED_AMBER |		\
				 AO_LED_GREEN |		\
				 AO_LED_CONTINUITY_1 |	\
				 AO_LED_CONTINUITY_0 |	\
				 AO_LED_REMOTE_ARM)

#define AO_LCO_DRAG		0

/*
 * Use event queue for input devices
 */

#define AO_EVENT		1

/*
 * Buttons
 */

#define AO_BUTTON_COUNT		3
#define AO_BUTTON_MODE		AO_EXTI_MODE_PULL_UP

#define AO_BUTTON_0_PORT	&stm_gpioa
#define AO_BUTTON_0		0

#define AO_BUTTON_BOX		0

#define AO_BUTTON_1_PORT	&stm_gpioa
#define AO_BUTTON_1		1

#define AO_BUTTON_ARM		1

#define AO_BUTTON_2_PORT	&stm_gpioa
#define AO_BUTTON_2		2

#define AO_BUTTON_FIRE		2

#endif /* _AO_PINS_H_ */
