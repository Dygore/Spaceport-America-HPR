/*
 * Copyright © 2017 Bdale Garbee <bdale@gag.com>
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

#define AO_STACK_SIZE		512

#define AO_HSE                  32000000
#define AO_RCC_CFGR_PLLMUL	STM_RCC_CFGR_PLLMUL_3
#define AO_RCC_CFGR2_PLLDIV	STM_RCC_CFGR2_PREDIV_2
#define AO_PLLMUL		3
#define AO_PLLDIV		2

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER        1
#define AO_RCC_CFGR_HPRE_DIV    STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER        1
#define AO_RCC_CFGR_PPRE_DIV    STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB			1
#define AO_USB_DIRECTIO		0
#define AO_PA11_PA12_RMP	0

#define IS_FLASH_LOADER 	0

/*
 * Serial ports
 */
#define HAS_SERIAL_1		0
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	0

#define HAS_SERIAL_2		1
#define USE_SERIAL_2_STDIN	1
#define DELAY_SERIAL_2_STDIN	1
#define USE_SERIAL_2_FLOW	1
#define USE_SERIAL_2_SW_FLOW	0
#define SERIAL_2_PA2_PA3	1
#define SERIAL_2_PD5_PD6	0
#define SERIAL_2_PORT_RTS	(&stm_gpioa)
#define SERIAL_2_PIN_RTS	1
#define SERIAL_2_PORT_CTS	(&stm_gpioa)
#define SERIAL_2_PIN_CTS	0

#define AO_CONFIG_MAX_SIZE	1024

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	0
#define USE_STORAGE_CONFIG	0
#define HAS_BEEP		0
#define HAS_BATTERY_REPORT	0
#define HAS_RADIO		1
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_ACCEL		0
#define HAS_AES			0

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1	/* CC1200 */
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_HIGH

#define HAS_SPI_2		0
#define SPI_2_PB13_PB14_PB15	0
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_HIGH

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	1

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_IOPBEN
#define LED_PORT_1_ENABLE	STM_RCC_AHBENR_IOPCEN
#define LED_PORT_0		(&stm_gpiob)
#define LED_PORT_1		(&stm_gpioc)
#define LED_PORT_0_SHIFT	0
#define LED_PORT_1_SHIFT	0
#define LED_PIN_RED		(0 + LED_PORT_0_SHIFT)
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

#define AO_ADC_V_BATT		4
#define AO_ADC_V_BATT_PORT	(&stm_gpioa)
#define AO_ADC_V_BATT_PIN	4

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_IOPAEN))

#define AO_NUM_ADC_PIN		1

#define AO_ADC_PIN0_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_V_BATT_PIN
#define AO_ADC_PIN0_CH		AO_ADC_V_BATT_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN)

#define AO_ADC_SQ1		AO_ADC_V_BATT

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	51	/* 5.1k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/*
 * RN4678
 */
#define HAS_RN			1

#define ao_serial_rn_getchar	ao_serial2_getchar
#define ao_serial_rn_putchar	ao_serial2_putchar
#define _ao_serial_rn_pollchar	_ao_serial2_pollchar
#define _ao_serial_rn_sleep_for	_ao_serial2_sleep_for
#define ao_serial_rn_set_speed ao_serial2_set_speed
#define ao_serial_rn_drain	ao_serial2_drain
#define ao_serial_rn_rx_fifo	(ao_stm_usart2.rx_fifo)

/* Pin 5. BM70 P2_2 */
#define AO_RN_SW_BTN_PORT	(&stm_gpioc)
#define AO_RN_SW_BTN_PIN	14

/* Pin 9. BM70 P2_3 */
#define AO_RN_WAKEUP_PORT	(&stm_gpiob)
#define AO_RN_WAKEUP_PIN	9

/* Pin 11. BM70 P2_7/tx_ind. Status indication along with P1_5 */
#define AO_RN_P0_4_PORT		(&stm_gpioc)
#define AO_RN_P0_4_PIN		13

/* Pin 12. BM70 P1_1. Status indication along with P0_4 */
#define AO_RN_P1_5_PORT		(&stm_gpiob)
#define AO_RN_P1_5_PIN		6

/* Pin 13. BM70 P1_2. Also I2C SCL */
#define AO_RN_P1_2_PORT		(&stm_gpiob)
#define AO_RN_P1_2_PIN		7

/* Pin 14. BM70 P1_3. Also I2C SDA */
#define AO_RN_P1_3_PORT		(&stm_gpiob)
#define AO_RN_P1_3_PIN		8

/* Pin 15. BM70 P0_0/cts. */
#define AO_RN_CTS_PORT		(&stm_gpioa)
#define AO_RN_CTS_PIN		1

/* Pin 16. BM70 P1_0. */
#define AO_RN_P0_5_PORT		(&stm_gpiob)
#define AO_RN_P0_5_PIN		5

/* Pin 17. BM70 P3_6. */
#define AO_RN_RTS_PORT		(&stm_gpioa)
#define AO_RN_RTS_PIN		0

/* Pin 18. BM70 P2_0. */
#define AO_RN_P2_0_PORT		(&stm_gpiob)
#define AO_RN_P2_0_PIN		3

/* Pin 19. BM70 P2_4. */
#define AO_RN_P2_4_PORT		(&stm_gpioa)
#define AO_RN_P2_4_PIN		10

/* Pin 20. BM70 NC. */
#define AO_RN_EAN_PORT
#define AO_RN_EAN_PIN

/* Pin 21. BM70 RST_N. */
#define AO_RN_RST_N_PORT	(&stm_gpioa)
#define AO_RN_RST_N_PIN		15

/* Pin 22. BM70 RXD. */
#define AO_RN_RXD_PORT		(&stm_gpioa)
#define AO_RN_RXD_PIN		2

/* Pin 23. BM70 TXD. */
#define AO_RN_TXD_PORT		(&stm_gpioa)
#define AO_RN_TXD_PIN		3

/* Pin 24. BM70 P3_1/RSSI_IND. */
#define AO_RN_P3_1_PORT		(&stm_gpiob)
#define AO_RN_P3_1_PIN		2

/* Pin 25. BM70 P3_2/LINK_DROP. */
#define AO_RN_P3_2_PORT		(&stm_gpioa)
#define AO_RN_P3_2_PIN		8

/* Pin 26. BM70 P3_3/UART_RX_IND. */
#define AO_RN_P3_3_PORT		(&stm_gpiob)
#define AO_RN_P3_3_PIN		15

/* Pin 27. BM70 P3_4/PAIRING_KEY. */
#define AO_RN_P3_4_PORT		(&stm_gpiob)
#define AO_RN_P3_4_PIN		14

/* Pin 28. BM70 P3_5. */
#define AO_RN_P3_6_PORT		(&stm_gpiob)
#define AO_RN_P3_6_PIN		13

/* Pin 29. BM70 P0_7. */
#define AO_RN_P3_7_PORT		(&stm_gpiob)
#define AO_RN_P3_7_PIN		12

/*
 * Radio (cc1200)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	5695485

#define AO_FEC_DEBUG		0
#define AO_CC1200_SPI_CS_PORT	(&stm_gpiob)
#define AO_CC1200_SPI_CS_PIN	11
#define AO_CC1200_SPI_BUS	AO_SPI_1_PA5_PA6_PA7
#define AO_CC1200_SPI		stm_spi1

#define AO_CC1200_INT_PORT		(&stm_gpiob)
#define AO_CC1200_INT_PIN		(10)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define HAS_BOOT_RADIO		0

/* Monitor bits */
#define HAS_MONITOR		1
#define LEGACY_MONITOR		0
#define AO_MONITOR_LED		AO_LED_RED

#endif /* _AO_PINS_H_ */
