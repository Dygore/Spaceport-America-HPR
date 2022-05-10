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

#define HAS_SERIAL_1		0
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	1

#define HAS_SERIAL_2		1
#define USE_SERIAL_2_STDIN	1
#define DELAY_SERIAL_2_STDIN	1
#define USE_SERIAL_2_FLOW	1
#define USE_SERIAL_2_SW_FLOW	1
#define SERIAL_2_PA2_PA3	1
#define SERIAL_2_PD5_PD6	0
#define SERIAL_2_PORT_RTS	(&stm_gpioa)
#define SERIAL_2_PIN_RTS	0
#define SERIAL_2_PORT_CTS	(&stm_gpioa)
#define SERIAL_2_PIN_CTS	1

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	1
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	0

#define AO_CONFIG_MAX_SIZE			1024

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_BATTERY_REPORT	0
#define HAS_RADIO		1
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_ACCEL		0
#define HAS_AES			1

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1	/* CC1200 */
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		0
#define SPI_2_PB13_PB14_PB15	0
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	1

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_GPIOAEN
#define LED_PORT_1_ENABLE	STM_RCC_AHBENR_GPIOCEN
#define LED_PORT_0		(&stm_gpioa)
#define LED_PORT_1		(&stm_gpioc)
#define LED_PORT_0_SHIFT	0
#define LED_PORT_1_SHIFT	0
#define LED_PIN_RED		(4 + LED_PORT_0_SHIFT)
#define LED_PIN_BLUE		(15 + LED_PORT_1_SHIFT)
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_BLUE		(1 << LED_PIN_BLUE)
#define LED_PORT_0_MASK		(AO_LED_RED)
#define LED_PORT_1_MASK		(AO_LED_BLUE)
#define AO_BT_LED		AO_LED_BLUE

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_BLUE)

#define HAS_GPS			0
#define HAS_FLIGHT		0
#define HAS_ADC			1
#define HAS_ADC_TEMP		0
#define HAS_LOG			0

/*
 * ADC
 */
#define AO_DATA_RING		32
#define AO_ADC_NUM_SENSE	2

struct ao_adc {
	int16_t			v_batt;
};

#define AO_ADC_DUMP(p) \
	printf("tick: %5lu batt %5d\n", \
	       (p)->tick, \
	       (p)->adc.v_batt);

#define AO_ADC_V_BATT		8
#define AO_ADC_V_BATT_PORT	(&stm_gpiob)
#define AO_ADC_V_BATT_PIN	0

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOEEN))

#define AO_NUM_ADC_PIN		1

#define AO_ADC_PIN0_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_V_BATT_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN)

#define AO_ADC_SQ1		AO_ADC_V_BATT

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	51	/* 5.6k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/*
 * BTM
 */
#define HAS_BTM			1

#define ao_serial_btm_getchar	ao_serial2_getchar
#define ao_serial_btm_putchar	ao_serial2_putchar
#define _ao_serial_btm_pollchar	_ao_serial2_pollchar
#define _ao_serial_btm_sleep_for	_ao_serial2_sleep_for
#define ao_serial_btm_set_speed ao_serial2_set_speed
#define ao_serial_btm_drain	ao_serial2_drain
#define ao_serial_btm_rx_fifo	(ao_stm_usart2.rx_fifo)

#define AO_BTM_INT_PORT		(&stm_gpioa)
#define AO_BTM_INT_PIN		15
#define AO_BTM_RESET_PORT	(&stm_gpiob)
#define AO_BTM_RESET_PIN	3

/*
 * Radio (cc1200)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	5695485

#define AO_FEC_DEBUG		0
#define AO_CC1200_SPI_CS_PORT	(&stm_gpiob)
#define AO_CC1200_SPI_CS_PIN	10
#define AO_CC1200_SPI_BUS	AO_SPI_1_PA5_PA6_PA7
#define AO_CC1200_SPI		stm_spi1

#define AO_CC1200_INT_PORT		(&stm_gpiob)
#define AO_CC1200_INT_PIN		(11)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define HAS_BOOT_RADIO		0

/* Monitor bits */
#define HAS_MONITOR		1
#define LEGACY_MONITOR		0
#define AO_MONITOR_LED		AO_LED_RED

#endif /* _AO_PINS_H_ */
