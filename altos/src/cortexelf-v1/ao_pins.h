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

/* SYSCLK = 24MHz */
#define AO_PLLDIV		4
#define AO_RCC_CFGR_PLLDIV	(STM_RCC_CFGR_PLLDIV_4)

/* HCLK = 24MHz (CPU clock) */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* Run APB1 at HCLK/1 */
#define AO_APB1_PRESCALER	1
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE1_DIV_1

/* Run APB2 at HCLK/1 */
#define AO_APB2_PRESCALER	1
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_1

/* Allow for non-maskable interrupts at priority 0 */
#define AO_NONMASK_INTERRUPT	1

/* PS/2 keyboard connection */
#define AO_PS2_CLOCK_PORT	(&stm_gpiod)
#define AO_PS2_CLOCK_BIT	9
#define AO_PS2_DATA_PORT	(&stm_gpiod)
#define AO_PS2_DATA_BIT		8

#define HAS_SERIAL_1		1
#define USE_SERIAL_1_STDIN	1
#define SERIAL_1_PB6_PB7	1
#define SERIAL_1_PA9_PA10	0

#define HAS_SERIAL_2		1
#define USE_SERIAL_2_STDIN	1
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	1
#define USE_SERIAL_2_FLOW	0
#define USE_SERIAL_2_SW_FLOW	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	0

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	0
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_BATTERY_REPORT	0
#define HAS_RADIO		0
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_COMPANION		0

#define HAS_SPI_1		0
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	0
#define SPI_2_PD1_PD3_PD4	1	/* LED displays, microSD */
#define SPI_2_OSPEEDR		STM_OSPEEDR_40MHz

#define SPI_2_PORT		(&stm_gpiod)
//#define SPI_2_SCK_PIN		1
//#define SPI_2_MISO_PIN		3
//#define SPI_2_MOSI_PIN		4

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define HAS_GPS			0
#define HAS_FLIGHT		0
#define HAS_ADC			0
#define HAS_ADC_TEMP		0
#define HAS_LOG			0

#define NUM_CMDS		16

/* SD card */
#define AO_SDCARD_SPI_BUS	AO_SPI_2_PD1_PD3_PD4
#define AO_SDCARD_SPI_CS_PORT	(&stm_gpiod)
#define AO_SDCARD_SPI_CS_PIN	2
#define AO_SDCARD_SPI_PORT	(&stm_gpiod)
#define AO_SDCARD_SPI_SCK_PIN	1
#define AO_SDCARD_SPI_MISO_PIN	3
#define AO_SDCARD_SPI_MOSI_PIN	4

/* VGA */
#define STM_DMA1_3_STOLEN	1
/* Buttons */

#define AO_EVENT		1

#define AO_BUTTON_COUNT		4
#define AO_BUTTON_MODE		AO_EXTI_MODE_PULL_DOWN
#define AO_BUTTON_INVERTED	0

/* INPUT */
#define AO_BUTTON_0_PORT	(&stm_gpioc)
#define AO_BUTTON_0		8

/* MP */
#define AO_BUTTON_1_PORT	(&stm_gpioc)
#define AO_BUTTON_1		9

/* RUN */
#define AO_BUTTON_2_PORT	(&stm_gpioc)
#define AO_BUTTON_2		10

/* LOAD */
#define AO_BUTTON_3_PORT	(&stm_gpioc)
#define AO_BUTTON_3		11

/* AS1107 */
#define AO_AS1107_NUM_DIGITS	8

/* Set the hex digits up for decode, leave the extra leds alone */

#define AO_AS1107_DECODE	((1 << 7) |	\
				 (1 << 6) |	\
				 (1 << 4) |	\
				 (1 << 3) |	\
				 (1 << 1) |	\
				 (1 << 0))

#define AO_AS1107_SPI_INDEX	AO_SPI_2_PD1_PD3_PD4
#define AO_AS1107_SPI_SPEED	AO_SPI_SPEED_8MHz
#define AO_AS1107_CS_PORT	(&stm_gpiod)
#define AO_AS1107_CS_PIN	0

/* Hex keypad */

#define AO_MATRIX_ROWS	4
#define AO_MATRIX_COLS	4

#define AO_MATRIX_KEYCODES {			\
		{ 0x0, 0x1, 0x2, 0x3 },		\
		{ 0x4, 0x5, 0x6, 0x7 },		\
		{ 0x8, 0x9, 0xa, 0xb },		\
		{ 0xc, 0xd, 0xe, 0xf }		\
	}

#include <ao_matrix.h>

#define AO_TIMER_HOOK	ao_matrix_poll()

#define AO_MATRIX_ROW_0_PORT	(&stm_gpioc)
#define AO_MATRIX_ROW_0_PIN	4

#define AO_MATRIX_ROW_1_PORT	(&stm_gpioc)
#define AO_MATRIX_ROW_1_PIN	1

#define AO_MATRIX_ROW_2_PORT	(&stm_gpioc)
#define AO_MATRIX_ROW_2_PIN	7

#define AO_MATRIX_ROW_3_PORT	(&stm_gpioc)
#define AO_MATRIX_ROW_3_PIN	0

#define AO_MATRIX_COL_0_PORT	(&stm_gpioc)
#define AO_MATRIX_COL_0_PIN	2

#define AO_MATRIX_COL_1_PORT	(&stm_gpioc)
#define AO_MATRIX_COL_1_PIN	3

#define AO_MATRIX_COL_2_PORT	(&stm_gpioc)
#define AO_MATRIX_COL_2_PIN	5

#define AO_MATRIX_COL_3_PORT	(&stm_gpioc)
#define AO_MATRIX_COL_3_PIN	6

/* 1802 connections */
#define MRD_PORT		(&stm_gpiob)
#define MRD_BIT			15

#define MWR_PORT		(&stm_gpioa)
#define MWR_BIT			3

#define TPB_PORT		(&stm_gpioa)
#define TPB_BIT			7

#define TPA_PORT		(&stm_gpioa)
#define TPA_BIT			6

#define MA_PORT			(&stm_gpioe)
#define MA_SHIFT		0
#define MA_MASK			0xff

#define BUS_PORT		(&stm_gpioe)
#define BUS_SHIFT		8
#define BUS_MASK		0xff

#define SC_PORT			(&stm_gpiob)
#define SC_SHIFT		13
#define SC_MASK			3

#define Q_PORT			(&stm_gpiob)
#define Q_BIT			12

#define N_PORT			(&stm_gpiod)
#define N_SHIFT			13
#define N_MASK			7

#define EF_PORT			(&stm_gpiob)
#define EF_SHIFT		8
#define EF_MASK			0xf

#define DMA_IN_PORT		(&stm_gpioa)
#define DMA_IN_BIT		0

#define DMA_OUT_PORT		(&stm_gpioa)
#define DMA_OUT_BIT		9

#define INT_PORT		(&stm_gpioa)
#define INT_BIT			2

#define CLEAR_PORT		(&stm_gpioa)
#define CLEAR_BIT		10

#define WAIT_PORT		(&stm_gpioa)
#define WAIT_BIT		4

#define MUX_PORT		(&stm_gpiob)
#define MUX_BIT			1

#endif /* _AO_PINS_H_ */
