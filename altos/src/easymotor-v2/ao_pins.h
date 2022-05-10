/*
 * Copyright © 2020 Bdale Garbee <bdale@gag.com>
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

#define HAS_TASK_QUEUE		1
#define IS_FLASH_LOADER		0

/* 48MHz clock based on 32MHz reference */
#define AO_HSE                  32000000
#define AO_RCC_CFGR_PLLMUL      STM_RCC_CFGR_PLLMUL_3
#define AO_RCC_CFGR2_PLLDIV     STM_RCC_CFGR2_PREDIV_2
#define AO_PLLMUL               3
#define AO_PLLDIV               2

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER        1
#define AO_RCC_CFGR_HPRE_DIV    STM_RCC_CFGR_HPRE_DIV_1

/* APB = 40MHz */
#define AO_APB_PRESCALER        1
#define AO_RCC_CFGR_PPRE_DIV    STM_RCC_CFGR_PPRE_DIV_1

#define HAS_SERIAL_1		0
#define USE_SERIAL_1_STDIN	0
#define SERIAL_1_PB6_PB7	0
#define SERIAL_1_PA9_PA10	1

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	1
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	0

#define AO_CONFIG_DEFAULT_FLIGHT_LOG_MAX	(1984 * 1024)
#define AO_CONFIG_MAX_SIZE			1024
#define LOG_ERASE_MARK				0x55
#define LOG_MAX_ERASE				128
#define AO_LOG_FORMAT				AO_LOG_FORMAT_EASYMOTOR

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	0
#define USE_STORAGE_CONFIG	1
#define HAS_USB			1
#define AO_PA11_PA12_RMP	1
#define HAS_BEEP		1
#define HAS_BATTERY_REPORT	1
#define HAS_PAD_REPORT		1
#define BEEPER_CHANNEL		3
#define BEEPER_TIMER		2
#define BEEPER_PORT		(&stm_gpioa)
#define BEEPER_PIN		2
#define BEEPER_AFR              STM_AFR_AF2

#define HAS_RADIO		0
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_COMPANION		0

#define LOW_LEVEL_DEBUG		0

#define HAS_GPS			0
#define HAS_FLIGHT		1
#define HAS_LOG			1

/*
 * ADC
 */
#define HAS_ADC			1

#define AO_ADC_PIN0_PORT	(&stm_gpioa)	/* pressure */
#define AO_ADC_PIN0_PIN		0
#define AO_ADC_PIN0_CH		0
#define AO_ADC_PIN1_PORT	(&stm_gpioa)	/* v_batt */
#define AO_ADC_PIN1_PIN		1
#define AO_ADC_PIN1_CH		1

#define AO_ADC_RCC_AHBENR       ((1 << STM_RCC_AHBENR_IOPAEN))

#define AO_NUM_ADC		2

#define AO_DATA_RING		64

struct ao_adc {
	int16_t			motor_pressure;
	int16_t			v_batt;
};

#define AO_ADC_DUMP(p) \
	printf("tick: %5lu motor_pressure: %5d batt: %5d\n", \
	       (p)->tick, \
	       (p)->adc.motor_pressure, \
	       (p)->adc.v_batt);


/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS     100     /* 100k */
#define AO_BATTERY_DIV_MINUS    27      /* 27k */

/*
 * Voltage divider on pressure sensor input
 */
#define AO_PRESSURE_DIV_PLUS    56      /* 5.6k 0.1% */
#define AO_PRESSURE_DIV_MINUS   100     /* 10k  0.1% */ 

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/* SPI */

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	1	/* flash */
#define SPI_1_PB3_PB4_PB5	1	/* adxl375 */
#define SPI_1_OSPEEDR		STM_OSPEEDR_MEDIUM

/*
 * SPI Flash memory
 */

#define M25_MAX_CHIPS		1
#define AO_M25_SPI_CS_PORT	(&stm_gpioa)
#define AO_M25_SPI_CS_MASK	(1 << 4)
#define AO_M25_SPI_BUS		AO_SPI_1_PA5_PA6_PA7

/* ADXL375 */

#define HAS_ADXL375		1
#define AO_ADXL375_SPI_INDEX	(AO_SPI_1_PB3_PB4_PB5 | AO_SPI_MODE_3)
#define AO_ADXL375_CS_PORT	(&stm_gpiob)
#define AO_ADXL375_CS_PIN	6

#define AO_ADXL375_AXIS		x
#define AO_ADXL375_ACROSS_AXIS	y
#define AO_ADXL375_THROUGH_AXIS	z
#define AO_ADXL375_INVERT	0
#define HAS_IMU			1
#define USE_ADXL375_IMU		1

/* Motor pressure */
#define HAS_MOTOR_PRESSURE	1
#define ao_data_motor_pressure(packet) ((packet)->adc.motor_pressure)

typedef int16_t	motor_pressure_t;

#endif /* _AO_PINS_H_ */
