/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#define HAS_SERIAL_1		1
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	1

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	1
#define SERIAL_3_PD8_PD9	0

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define HAS_USB			1
#define HAS_BEEP		0
#define HAS_RADIO		0
#define HAS_TELEMETRY		0

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	1	/* */
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		1
#define SPI_2_PB13_PB14_PB15	1	/* */
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz
#define HAS_STORAGE_DEBUG	1

#define SPI_2_PORT		(&stm_gpiob)
#define SPI_2_SCK_PIN		13
#define SPI_2_MISO_PIN		14
#define SPI_2_MOSI_PIN		15

#define HAS_I2C_1		1
#define I2C_1_PB6_PB7		1
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		1
#define I2C_2_PB10_PB11		1

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define LED_PORT_0_ENABLE	STM_RCC_AHBENR_GPIOCEN
#define LED_PORT_0		(&stm_gpioc)
#define LED_PORT_0_MASK		(0xff)
#define LED_PORT_0_SHIFT	0
#define LED_PIN_RED		6
#define LED_PIN_GREEN		7
#define LED_PIN_RED_2		8
#define LED_PIN_GREEN_2		9
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)
#define AO_LED_RED_2		(1 << LED_PIN_RED_2)
#define AO_LED_GREEN_2		(1 << LED_PIN_GREEN_2)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN | AO_LED_RED_2 | AO_LED_GREEN_2)

#define HAS_GPS			0
#define HAS_FLIGHT		0
#define HAS_ADC			1
#define HAS_ADC_TEMP		1
#define HAS_LOG			0

/*
 * ADC
 */
#define AO_DATA_RING		32

struct ao_adc {
	int16_t			tx_pa_current;	/* 0 ADC_IN0 */
	int16_t			tx_temp;	/* 1 ADC_IN1 */
	int16_t			exp4_temp;	/* 2 ADC_IN2 */
	int16_t			rx_temp;	/* 3 ADC_IN3 */
	int16_t			tx_analog_1;	/* 4 ADC_IN4 */
	int16_t			sense_batt;	/* 5 ADC_IN5 */
	int16_t			rx_analog_1;	/* 6 ADC_IN6 */
	int16_t			rx_rssi;	/* 7 ADC_IN8 */
	int16_t			rx_cd;		/* 8 ADC_IN9 */
	int16_t			ant_sense_1;	/* 9 ADC_IN10 */
	int16_t			ant_sense_2;	/* 10 ADC_IN11 */
	int16_t			gyro_x_1;	/* 11 ADC_IN12 */
	int16_t			gyro_z_1;	/* 12 ADC_IN13 */
	int16_t			gyro_vref_1;	/* 13 ADC_IN24 */
	int16_t			gyro_x_2;	/* 14 ADC_IN14 */
	int16_t			gyro_z_2;	/* 15 ADC_IN15 */
	int16_t			gyro_vref_2;	/* 16 ADC_IN25 */
};

#define AO_ADC_TX_PA_CURRENT		0
#define AO_ADC_TX_PA_CURRENT_PORT	(&stm_gpioa)
#define AO_ADC_TX_PA_CURRENT_PIN	0

#define AO_ADC_TX_TEMP			1
#define AO_ADC_TX_TEMP_PORT		(&stm_gpioa)
#define AO_ADC_TX_TEMP_PIN		1

#define AO_ADC_EXP4_TEMP		2
#define AO_ADC_EXP4_TEMP_PORT		(&stm_gpioa)
#define AO_ADC_EXP4_TEMP_PIN		2

#define AO_ADC_RX_TEMP			3
#define AO_ADC_RX_TEMP_PORT		(&stm_gpioa)
#define AO_ADC_RX_TEMP_PIN		3

#define AO_ADC_TX_ANALOG_1		4
#define AO_ADC_TX_ANALOG_1_PORT		(&stm_gpioa)
#define AO_ADC_TX_ANALOG_1_PIN		4

#define AO_ADC_SENSE_BATT		5
#define AO_ADC_SENSE_BATT_PORT		(&stm_gpioa)
#define AO_ADC_SENSE_BATT_PIN		5

#define AO_ADC_RX_ANALOG_1		6
#define AO_ADC_RX_ANALOG_1_PORT		(&stm_gpioa)
#define AO_ADC_RX_ANALOG_1_PIN		6

#define AO_ADC_RX_RSSI			8
#define AO_ADC_RX_RSSI_PORT		(&stm_gpiob)
#define AO_ADC_RX_RSSI_PIN		0

#define AO_ADC_RX_CD			9
#define AO_ADC_RX_CD_PORT		(&stm_gpiob)
#define AO_ADC_RX_CD_PIN		1

#define AO_ADC_ANT_SENSE_1		10
#define AO_ADC_ANT_SENSE_1_PORT		(&stm_gpioc)
#define AO_ADC_ANT_SENSE_1_PIN		0

#define AO_ADC_ANT_SENSE_2		11
#define AO_ADC_ANT_SENSE_2_PORT		(&stm_gpioc)
#define AO_ADC_ANT_SENSE_2_PIN		1

#define AO_ADC_GYRO_X_1			12
#define AO_ADC_GYRO_X_1_PORT		(&stm_gpioc)
#define AO_ADC_GYRO_X_1_PIN		2

#define AO_ADC_GYRO_Z_1			13
#define AO_ADC_GYRO_Z_1_PORT		(&stm_gpioc)
#define AO_ADC_GYRO_Z_1_PIN		3

#define AO_ADC_GYRO_VREF_1		24
#define AO_ADC_GYRO_VREF_1_PORT		(&stm_gpioe)
#define AO_ADC_GYRO_VREF_1_PIN		9

#define AO_ADC_GYRO_X_2			14
#define AO_ADC_GYRO_X_2_PORT		(&stm_gpioc)
#define AO_ADC_GYRO_X_2_PIN		4

#define AO_ADC_GYRO_Z_2			15
#define AO_ADC_GYRO_Z_2_PORT		(&stm_gpioc)
#define AO_ADC_GYRO_Z_2_PIN		5

#define AO_ADC_GYRO_VREF_2		25
#define AO_ADC_GYRO_VREF_2_PORT		(&stm_gpioe)
#define AO_ADC_GYRO_VREF_2_PIN		10

#define AO_ADC_TEMP			16

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOAEN) | \
				 (1 << STM_RCC_AHBENR_GPIOBEN) | \
				 (1 << STM_RCC_AHBENR_GPIOCEN) | \
				 (1 << STM_RCC_AHBENR_GPIOEEN))

#define AO_NUM_ADC_PIN		(17)

#define AO_ADC_PIN0_PORT	AO_ADC_TX_PA_CURRENT_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_TX_PA_CURRENT_PIN
#define AO_ADC_PIN1_PORT	AO_ADC_TX_TEMP_PORT
#define AO_ADC_PIN1_PIN		AO_ADC_TX_TEMP_PIN
#define AO_ADC_PIN2_PORT	AO_ADC_EXP4_TEMP_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_EXP4_TEMP_PIN
#define AO_ADC_PIN3_PORT	AO_ADC_RX_TEMP_PORT
#define AO_ADC_PIN3_PIN		AO_ADC_RX_TEMP_PIN
#define AO_ADC_PIN4_PORT	AO_ADC_TX_ANALOG_1_PORT
#define AO_ADC_PIN4_PIN		AO_ADC_TX_ANALOG_1_PIN
#define AO_ADC_PIN5_PORT	AO_ADC_SENSE_BATT_PORT
#define AO_ADC_PIN5_PIN		AO_ADC_SENSE_BATT_PIN
#define AO_ADC_PIN6_PORT	AO_ADC_RX_ANALOG_1_PORT
#define AO_ADC_PIN6_PIN		AO_ADC_RX_ANALOG_1_PIN
#define AO_ADC_PIN7_PORT	AO_ADC_RX_RSSI_PORT
#define AO_ADC_PIN7_PIN		AO_ADC_RX_RSSI_PIN
#define AO_ADC_PIN8_PORT	AO_ADC_RX_CD_PORT
#define AO_ADC_PIN8_PIN		AO_ADC_RX_CD_PIN
#define AO_ADC_PIN9_PORT	AO_ADC_ANT_SENSE_1_PORT
#define AO_ADC_PIN9_PIN		AO_ADC_ANT_SENSE_1_PIN
#define AO_ADC_PIN10_PORT	AO_ADC_ANT_SENSE_2_PORT
#define AO_ADC_PIN10_PIN	AO_ADC_ANT_SENSE_2_PIN
#define AO_ADC_PIN11_PORT	AO_ADC_GYRO_X_1_PORT
#define AO_ADC_PIN11_PIN	AO_ADC_GYRO_X_1_PIN
#define AO_ADC_PIN12_PORT	AO_ADC_GYRO_Z_1_PORT
#define AO_ADC_PIN12_PIN	AO_ADC_GYRO_Z_1_PIN
#define AO_ADC_PIN13_PORT	AO_ADC_GYRO_VREF_1_PORT
#define AO_ADC_PIN13_PIN	AO_ADC_GYRO_VREF_1_PIN
#define AO_ADC_PIN14_PORT	AO_ADC_GYRO_X_2_PORT
#define AO_ADC_PIN14_PIN	AO_ADC_GYRO_X_2_PIN
#define AO_ADC_PIN15_PORT	AO_ADC_GYRO_Z_2_PORT
#define AO_ADC_PIN15_PIN	AO_ADC_GYRO_Z_2_PIN
#define AO_ADC_PIN16_PORT	AO_ADC_GYRO_VREF_2_PORT
#define AO_ADC_PIN16_PIN	AO_ADC_GYRO_VREF_2_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN + 1)	/* Add internal temp sensor */

#define AO_ADC_SQ1		AO_ADC_TX_PA_CURRENT
#define AO_ADC_SQ1_NAME		"tx_pa_current"
#define AO_ADC_SQ2		AO_ADC_TX_TEMP
#define AO_ADC_SQ2_NAME		"tx_temp"
#define AO_ADC_SQ3		AO_ADC_EXP4_TEMP
#define AO_ADC_SQ3_NAME		"expr_temp"
#define AO_ADC_SQ4		AO_ADC_RX_TEMP
#define AO_ADC_SQ4_NAME		"rx_temp"
#define AO_ADC_SQ5		AO_ADC_TX_ANALOG_1
#define AO_ADC_SQ5_NAME		"tx_analog_1"
#define AO_ADC_SQ6		AO_ADC_SENSE_BATT
#define AO_ADC_SQ6_NAME		"sense_batt"
#define AO_ADC_SQ7		AO_ADC_RX_ANALOG_1
#define AO_ADC_SQ7_NAME		"rx_analog_1"
#define AO_ADC_SQ8		AO_ADC_RX_RSSI
#define AO_ADC_SQ8_NAME		"rx_rssi"
#define AO_ADC_SQ9		AO_ADC_RX_CD
#define AO_ADC_SQ9_NAME		"rx_cd"
#define AO_ADC_SQ10		AO_ADC_ANT_SENSE_1
#define AO_ADC_SQ10_NAME	"ant_sense_1"
#define AO_ADC_SQ11		AO_ADC_ANT_SENSE_2
#define AO_ADC_SQ11_NAME	"ant_sense_2"
#define AO_ADC_SQ12		AO_ADC_GYRO_X_1
#define AO_ADC_SQ12_NAME	"gyro_x_1"
#define AO_ADC_SQ13		AO_ADC_GYRO_Z_1
#define AO_ADC_SQ13_NAME	"gyro_z_1"
#define AO_ADC_SQ14		AO_ADC_GYRO_VREF_1
#define AO_ADC_SQ14_NAME	"gyro_vref_1"
#define AO_ADC_SQ15		AO_ADC_GYRO_X_2
#define AO_ADC_SQ15_NAME	"gyro_x_2"
#define AO_ADC_SQ16		AO_ADC_GYRO_Z_2
#define AO_ADC_SQ16_NAME	"gyro_z_2"
#define AO_ADC_SQ17		AO_ADC_GYRO_VREF_2
#define AO_ADC_SQ17_NAME	"gyro_vref_2"
#define AO_ADC_SQ18		AO_ADC_TEMP
#define AO_ADC_SQ18_NAME	"temp"

/* Watchdog timer */

#define AO_WATCHDOG_INTERVAL	AO_MS_TO_TICKS(40)
#define AO_WATCHDOG_PORT	(&stm_gpiod)
#define AO_WATCHDOG_BIT		3

/* MRAM device */

#define AO_MR25_SPI_CS_PORT	(&stm_gpiod)
#define AO_MR25_SPI_CS_PIN	0
#define AO_MR25_SPI_BUS		AO_SPI_2_PB13_PB14_PB15

/* SD card */

#define AO_SDCARD_SPI_CS_PORT	(&stm_gpiod)
#define AO_SDCARD_SPI_CS_PIN	1
#define AO_SDCARD_SPI_BUS	AO_SPI_2_PB13_PB14_PB15
#define AO_SDCARD_SPI_PORT	(&stm_gpiob)
#define AO_SDCARD_SPI_SCK_PIN	13
#define AO_SDCARD_SPI_MISO_PIN	14
#define AO_SDCARD_SPI_MOSI_PIN	15

#endif /* _AO_PINS_H_ */
