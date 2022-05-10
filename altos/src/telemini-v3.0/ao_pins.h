/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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

#define HAS_BEEP		1
#define HAS_SERIAL_1		0
#define HAS_BATTERY_REPORT	1

#define AO_STACK_SIZE	448

#define IS_FLASH_LOADER	0

/* 48MHz clock based on 16MHz reference */
//#define AO_HSI48	1
#define AO_HSE			16000000
#define AO_RCC_CFGR_PLLMUL	STM_RCC_CFGR_PLLMUL_3
#define AO_RCC_CFGR2_PLLDIV	STM_RCC_CFGR2_PREDIV_1
#define AO_PLLMUL		3
#define AO_PLLDIV		1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* APB = 40MHz */
#define AO_APB_PRESCALER	1
#define AO_RCC_CFGR_PPRE_DIV	STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB			1
#define AO_USB_DIRECTIO		0
#define AO_PA11_PA12_RMP	1
#define AO_USB_FORCE_IDLE	1

#define PACKET_HAS_SLAVE	1

#define AO_LOG_FORMAT		AO_LOG_FORMAT_TELEMINI3
#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	((uint32_t) 112 * (uint32_t) 1024)

#define HAS_BOOT_RADIO		0

#define HAS_ACCEL		0
#define HAS_GPS			0
#define HAS_RADIO		1
#define HAS_RADIO_RATE		1
#define HAS_FLIGHT		1
#define HAS_EEPROM		1
#define HAS_TELEMETRY		1
#define AO_SEND_MINI		AO_TELEMETRY_MINI3
#define HAS_APRS		0
#define HAS_LOG			1
#define USE_INTERNAL_FLASH	0
#define HAS_IGNITE		1
#define HAS_IGNITE_REPORT	1
#define AO_SMALL_ALTITUDE_TABLE	1

/* Use debug connector clock for recovery mode */
#define HAS_FORCE_FREQ		1
#define AO_RECOVERY_PORT	(&stm_gpioa)
#define AO_RECOVERY_PIN		14
#define AO_RECOVERY_VALUE	0
#define AO_RECOVERY_MODE	AO_EXTI_MODE_PULL_UP

/* Beeper is on Tim2 CH4 */
#define BEEPER_CHANNEL		4
#define BEEPER_TIMER		2
#define BEEPER_PORT		(&stm_gpioa)
#define BEEPER_PIN		3

/* SPI */

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1
#define SPI_1_PB3_PB4_PB5	1
#define SPI_1_OSPEEDR		STM_OSPEEDR_MEDIUM

/* M25 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	(&stm_gpioa)
#define AO_M25_SPI_CS_MASK	(1 << 4)
#define AO_M25_SPI_BUS		AO_SPI_1_PA5_PA6_PA7

/* MS5607 */

#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	1
#define AO_MS5607_CS_PORT	(&stm_gpioa)
#define AO_MS5607_CS_PIN	15
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS_PIN)
#define AO_MS5607_MISO_PORT	(&stm_gpiob)
#define AO_MS5607_MISO_PIN	4
#define AO_MS5607_MISO_MASK	(1 << AO_MS5607_MISO_PIN)
#define AO_MS5607_SPI_INDEX	AO_SPI_1_PB3_PB4_PB5

/* CC1200 */

// #define AO_RADIO_CAL_DEFAULT 	5695733
#define AO_RADIO_CAL_DEFAULT 	5695717

#define AO_FEC_DEBUG		0
#define CC1200_DEBUG		0
#define AO_CC1200_SPI_CS_PORT	(&stm_gpiob)
#define AO_CC1200_SPI_CS_PIN	0
#define AO_CC1200_SPI_BUS	AO_SPI_1_PA5_PA6_PA7
#define AO_CC1200_SPI		stm_spi1

#define AO_CC1200_INT_PORT		(&stm_gpiob)
#define AO_CC1200_INT_PIN		1

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2


#define AO_DATA_RING		16

/*
 * ADC
 */

#define HAS_ADC			1

#define AO_ADC_PIN0_PORT	(&stm_gpioa)	/* sense_m */
#define AO_ADC_PIN0_PIN		0
#define AO_ADC_PIN0_CH		0
#define AO_ADC_PIN1_PORT	(&stm_gpioa)	/* sense_a */
#define AO_ADC_PIN1_PIN		1
#define AO_ADC_PIN1_CH		1
#define AO_ADC_PIN2_PORT	(&stm_gpioa)	/* v_batt */
#define AO_ADC_PIN2_PIN		2
#define AO_ADC_PIN2_CH		2

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_IOPAEN))

#define AO_NUM_ADC		3

struct ao_adc {
	int16_t		sense_m;
	int16_t		sense_a;
	int16_t		v_batt;
};

/*
 * Igniter
 */

#define AO_IGNITER_CLOSED	400
#define AO_IGNITER_OPEN		60

#define AO_IGNITER_DROGUE_PORT	(&stm_gpiob)
#define AO_IGNITER_DROGUE_PIN	7

#define AO_IGNITER_MAIN_PORT	(&stm_gpiob)
#define AO_IGNITER_MAIN_PIN	6

#define AO_SENSE_DROGUE(p)	((p)->adc.sense_a)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)

#define AO_ADC_DUMP(p) \
	printf("tick: %5lu apogee: %5d main: %5d batt: %5d\n", \
	       (p)->tick, (p)->adc.sense_a, (p)->adc.sense_m, (p)->adc.v_batt)

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	56	/* 5.6k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * Voltage divider on ADC igniter samplers
 */
#define AO_IGNITE_DIV_PLUS	100	/* 100k */
#define AO_IGNITE_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33
