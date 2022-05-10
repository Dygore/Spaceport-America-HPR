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


/* 16MHz High speed external crystal */
#define AO_HSE			16000000

/* PLLVCO = 96MHz (so that USB will work) */
#define AO_PLLMUL		6
#define AO_RCC_CFGR_PLLMUL	(STM_RCC_CFGR_PLLMUL_6)

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
#define SERIAL_1_PA9_PA10	0

#define HAS_SERIAL_2		0
#define USE_SERIAL_2_STDIN	0
#define SERIAL_2_PA2_PA3	0
#define SERIAL_2_PD5_PD6	0

#define HAS_SERIAL_3		0
#define USE_SERIAL_3_STDIN	0
#define SERIAL_3_PB10_PB11	0
#define SERIAL_3_PC10_PC11	0
#define SERIAL_3_PD8_PD9	0

#define AO_CONFIG_MAX_SIZE	1024

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_USB			1
#define HAS_BEEP		1
#define HAS_BATTERY_REPORT	1
#define BEEPER_CHANNEL		1
#define BEEPER_TIMER		4
#define BEEPER_PORT		(&stm_gpiob)
#define BEEPER_PIN		6
#define HAS_RADIO		0
#define HAS_TELEMETRY		0
#define HAS_APRS		0
#define HAS_COMPANION		0

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	0	
#define SPI_1_PB3_PB4_PB5	1	/* IMU */
#define SPI_1_PE13_PE14_PE15	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		0
#define SPI_2_PB13_PB14_PB15	0	/* Flash, Companion, Radio */
#define SPI_2_PD1_PD3_PD4	0
#define SPI_2_OSPEEDR		STM_OSPEEDR_10MHz

#define SPI_2_PORT		(&stm_gpiob)
#define SPI_2_SCK_PIN		13
#define SPI_2_MISO_PIN		14
#define SPI_2_MOSI_PIN		15

#define HAS_I2C_1		0
#define I2C_1_PB8_PB9		0

#define HAS_I2C_2		0
#define I2C_2_PB10_PB11		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define LOW_LEVEL_DEBUG		0

#define LEDS_AVAILABLE		0

#define HAS_GPS			0
#define HAS_FLIGHT		1
#define HAS_ADC			1
#define HAS_ADC_TEMP		1
#define HAS_LOG			0

/*
 * Igniter
 */

#define HAS_IGNITE		0
#define HAS_IGNITE_REPORT	1
#define AO_PYRO_NUM		2

#define AO_SENSE_PYRO(p,n)	((p)->adc.sense[n])
#define AO_IGNITER_CLOSED	400
#define AO_IGNITER_OPEN		60

/* Pyro A */
#define AO_PYRO_PORT_0  (&stm_gpiob)
#define AO_PYRO_PIN_0   0

#define AO_ADC_SENSE_A          1
#define AO_ADC_SENSE_A_PORT     (&stm_gpioa)
#define AO_ADC_SENSE_A_PIN      1

/* Pyro B */
#define AO_PYRO_PORT_1  (&stm_gpiob)
#define AO_PYRO_PIN_1   11

#define AO_ADC_SENSE_B          0
#define AO_ADC_SENSE_B_PORT     (&stm_gpioa)
#define AO_ADC_SENSE_B_PIN      0


/*
 * ADC
 */
#define AO_DATA_RING		32
#define AO_ADC_NUM_SENSE	2

struct ao_adc {
	int16_t			sense[AO_ADC_NUM_SENSE];
	int16_t			v_batt;
	int16_t			temp;
};

#define AO_ADC_DUMP(p) \
	printf("tick: %5lu A: %5d B: %5d batt: %5d\n", \
	       (p)->tick, \
               (p)->adc.sense[0], (p)->adc.sense[1], \
	       (p)->adc.v_batt);

#define AO_ADC_V_BATT		2
#define AO_ADC_V_BATT_PORT	(&stm_gpioa)
#define AO_ADC_V_BATT_PIN	2

#define AO_ADC_TEMP		16

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOAEN) | \
				 (1 << STM_RCC_AHBENR_GPIOEEN) | \
				 (1 << STM_RCC_AHBENR_GPIOBEN))

#define AO_NUM_ADC_PIN		(AO_ADC_NUM_SENSE + 1)

#define AO_ADC_PIN0_PORT        AO_ADC_SENSE_A_PORT
#define AO_ADC_PIN0_PIN         AO_ADC_SENSE_A_PIN
#define AO_ADC_PIN1_PORT        AO_ADC_SENSE_B_PORT
#define AO_ADC_PIN1_PIN         AO_ADC_SENSE_B_PIN
#define AO_ADC_PIN2_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_V_BATT_PIN

#define AO_NUM_ADC	       	(AO_NUM_ADC_PIN + 1)

#define AO_ADC_SQ1              AO_ADC_SENSE_A
#define AO_ADC_SQ2              AO_ADC_SENSE_B
#define AO_ADC_SQ3		AO_ADC_V_BATT
#define AO_ADC_SQ4		AO_ADC_TEMP

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	100	/* 100k */
#define AO_BATTERY_DIV_MINUS	27	/* 27k */

/*
 * Voltage divider on ADC igniter samplers
 */
#define AO_IGNITE_DIV_PLUS	100	/* 100k */
#define AO_IGNITE_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

/*
 * bmx160
 */

#define HAS_BMX160              1
#define AO_BMX160_INT_PORT      (&stm_gpioc)
#define AO_BMX160_INT_PIN       13
#define AO_BMX160_SPI_BUS       (AO_SPI_1_PB3_PB4_PB5 | AO_SPI_MODE_0)
#define AO_BMX160_SPI_CS_PORT   (&stm_gpioa)
#define AO_BMX160_SPI_CS_PIN    15
#define HAS_IMU                 1

#define ao_data_along(packet)   ((packet)->bmx160.acc_x)
#define ao_data_across(packet)  (-(packet)->bmx160.acc_y)
#define ao_data_through(packet) ((packet)->bmx160.acc_z)

#define ao_data_roll(packet)    ((packet)->bmx160.gyr_x)
#define ao_data_pitch(packet)   (-(packet)->bmx160.gyr_y)
#define ao_data_yaw(packet)     ((packet)->bmx160.gyr_z)

#define ao_data_mag_along(packet)       ((packet)->bmx160.mag_x)
#define ao_data_mag_across(packet)      (-(packet)->bmx160.mag_y)
#define ao_data_mag_through(packet)     ((packet)->bmx160.mag_z)

#define ao_data_accel_cook(packet)		(-ao_data_along(packet))

/*
 * Monitor
 */

#define HAS_MONITOR		0
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		1
#define AO_MONITOR_LED		0
#define HAS_RSSI		0

#endif /* _AO_PINS_H_ */
