/*
 * Copyright © 2010 Keith Packard <keithp@keithp.com>
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

#define HAS_RADIO		1
#define HAS_RADIO_RATE		1
#define HAS_TELEMETRY		0
#define HAS_FLIGHT		0
#define HAS_USB			1
#define HAS_BEEP		0
#define BEEPER_TIMER		3
#define BEEPER_CHANNEL		1
#define BEEPER_PORT		(&stm_gpioc)
#define BEEPER_PIN		6
#define HAS_GPS			0
#define HAS_SERIAL_1		0
#define HAS_ADC			1
#define HAS_DBG			0
#define HAS_EEPROM		1
#define HAS_LOG			0
#define HAS_PAD			1
#define USE_INTERNAL_FLASH	1
#define IGNITE_ON_P0		0
#define PACKET_HAS_MASTER	0
#define PACKET_HAS_SLAVE	0
#define AO_DATA_RING		32
#define USE_EEPROM_CONFIG	1
#define USE_STORAGE_CONFIG	0
#define HAS_AES			1

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

#define HAS_SPI_1		1
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	1
#define SPI_1_GPIO		(&stm_gpioe)
#define SPI_1_SCK		13
#define SPI_1_MISO		14
#define SPI_1_MOSI		15
#define SPI_1_OSPEEDR		STM_OSPEEDR_10MHz

#define HAS_SPI_2		0
#define SPI_2_PB13_PB14_PB15	0
#define SPI_2_PD1_PD3_PD4	0

#define HAS_I2C_1		0

#define HAS_I2C_2		0

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	0

#define FAST_TIMER_FREQ		10000	/* .1ms for debouncing */

/*
 * Radio is a cc1200 connected via SPI
 */

#define AO_RADIO_CAL_DEFAULT 	5695733

#define AO_CC1200_SPI_CS_PORT	(&stm_gpioe)
#define AO_CC1200_SPI_CS_PIN	11
#define AO_CC1200_SPI_BUS	AO_SPI_1_PE13_PE14_PE15
#define AO_CC1200_SPI		stm_spi1

#define AO_CC1200_INT_PORT	(&stm_gpioe)
#define AO_CC1200_INT_PIN	(12)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define HAS_LED			1
#define LED_TYPE		uint16_t

/* Continuity leds 1-8 */
#define LED_0_PORT		(&stm_gpiob)
#define LED_0_PIN		13
#define LED_1_PORT		(&stm_gpiob)
#define LED_1_PIN		12
#define LED_2_PORT		(&stm_gpiob)
#define LED_2_PIN		11
#define LED_3_PORT		(&stm_gpiob)
#define LED_3_PIN		10
#define LED_4_PORT		(&stm_gpioc)
#define LED_4_PIN		9
#define LED_5_PORT		(&stm_gpioa)
#define LED_5_PIN		8
#define LED_6_PORT		(&stm_gpioa)
#define LED_6_PIN		9
#define LED_7_PORT		(&stm_gpioa)
#define LED_7_PIN		10

#define AO_LED_CONTINUITY(c)	((AO_LED_TYPE) (1 << (c)))
#define AO_LED_CONTINUITY_MASK	(0xff)

/* ARM */
#define LED_8_PORT		(&stm_gpioe)
#define LED_8_PIN		3

#define LED_PIN_ARMED		8

/* RF good/marginal/poor */
#define LED_9_PORT		(&stm_gpioe)
#define LED_9_PIN		4
#define LED_10_PORT		(&stm_gpioe)
#define LED_10_PIN		5
#define LED_11_PORT		(&stm_gpioe)
#define LED_11_PIN		6

#define AO_LED_ARMED		AO_LED_8
#define AO_LED_GREEN		AO_LED_9
#define AO_LED_AMBER		AO_LED_10
#define AO_LED_RED		AO_LED_11

/* Alarm A */
#define AO_SIREN
#define AO_SIREN_PORT		(&stm_gpiod)
#define AO_SIREN_PIN		10

/* Alarm B */
#define AO_STROBE
#define AO_STROBE_PORT		(&stm_gpiod)
#define AO_STROBE_PIN		11

/* Pad selector is on PD0-7 */

#define HAS_FIXED_PAD_BOX	1
#define AO_PAD_SELECTOR_PORT	(&stm_gpiod)
#define AO_PAD_SELECTOR_PINS	(0xff)

#define SPI_CONST	0x00

#define AO_PAD_NUM		8
#define	AO_PAD_PORT_0		(&stm_gpiod)
#define	AO_PAD_PORT_1		(&stm_gpiob)

#define AO_PAD_PIN_0		9
#define AO_PAD_0_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_0	3
#define AO_ADC_SENSE_PAD_0_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_0_PIN	3

#define AO_PAD_PIN_1		8
#define AO_PAD_1_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_1	2
#define AO_ADC_SENSE_PAD_1_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_1_PIN	2

#define AO_PAD_PIN_2		15
#define AO_PAD_2_PORT		(&stm_gpiob)
#define AO_ADC_SENSE_PAD_2	1
#define AO_ADC_SENSE_PAD_2_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_2_PIN	1

#define AO_PAD_PIN_3		14
#define AO_PAD_3_PORT		(&stm_gpiob)
#define AO_ADC_SENSE_PAD_3	0
#define AO_ADC_SENSE_PAD_3_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_3_PIN	0

#define AO_PAD_PIN_4		12
#define AO_PAD_4_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_4	7
#define AO_ADC_SENSE_PAD_4_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_4_PIN	7

#define AO_PAD_PIN_5		13
#define AO_PAD_5_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_5	6
#define AO_ADC_SENSE_PAD_5_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_5_PIN	6

#define AO_PAD_PIN_6		14
#define AO_PAD_6_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_6	5
#define AO_ADC_SENSE_PAD_6_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_6_PIN	5

#define AO_PAD_PIN_7		15
#define AO_PAD_7_PORT		(&stm_gpiod)
#define AO_ADC_SENSE_PAD_7	4
#define AO_ADC_SENSE_PAD_7_PORT	(&stm_gpioa)
#define AO_ADC_SENSE_PAD_7_PIN	4

#define AO_ADC_PYRO		8
#define AO_ADC_PYRO_PORT	(&stm_gpiob)
#define AO_ADC_PYRO_PIN		0

#define AO_ADC_BATT		15
#define AO_ADC_BATT_PORT	(&stm_gpioc)
#define AO_ADC_BATT_PIN		5

#define AO_ADC_PIN0_PORT	AO_ADC_SENSE_PAD_0_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_SENSE_PAD_0_PIN

#define AO_ADC_PIN1_PORT	AO_ADC_SENSE_PAD_1_PORT
#define AO_ADC_PIN1_PIN		AO_ADC_SENSE_PAD_1_PIN

#define AO_ADC_PIN2_PORT	AO_ADC_SENSE_PAD_2_PORT
#define AO_ADC_PIN2_PIN		AO_ADC_SENSE_PAD_2_PIN

#define AO_ADC_PIN3_PORT	AO_ADC_SENSE_PAD_3_PORT
#define AO_ADC_PIN3_PIN		AO_ADC_SENSE_PAD_3_PIN

#define AO_ADC_PIN4_PORT	AO_ADC_SENSE_PAD_4_PORT
#define AO_ADC_PIN4_PIN		AO_ADC_SENSE_PAD_4_PIN

#define AO_ADC_PIN5_PORT	AO_ADC_SENSE_PAD_5_PORT
#define AO_ADC_PIN5_PIN		AO_ADC_SENSE_PAD_5_PIN

#define AO_ADC_PIN6_PORT	AO_ADC_SENSE_PAD_6_PORT
#define AO_ADC_PIN6_PIN		AO_ADC_SENSE_PAD_6_PIN

#define AO_ADC_PIN7_PORT	AO_ADC_SENSE_PAD_7_PORT
#define AO_ADC_PIN7_PIN		AO_ADC_SENSE_PAD_7_PIN

#define AO_ADC_PIN8_PORT	AO_ADC_PYRO_PORT
#define AO_ADC_PIN8_PIN		AO_ADC_PYRO_PIN

#define AO_ADC_PIN9_PORT	AO_ADC_BATT_PORT
#define AO_ADC_PIN9_PIN		AO_ADC_BATT_PIN

#define AO_PAD_ALL_CHANNELS	(0xff)

/* test these values with real igniters */
#define AO_PAD_RELAY_CLOSED	3524
#define AO_PAD_NO_IGNITER	16904
#define AO_PAD_GOOD_IGNITER	22514

#define AO_ADC_FIRST_PIN	0

#define AO_NUM_ADC		10

#define AO_ADC_SQ1		AO_ADC_SENSE_PAD_0
#define AO_ADC_SQ2		AO_ADC_SENSE_PAD_1
#define AO_ADC_SQ3		AO_ADC_SENSE_PAD_2
#define AO_ADC_SQ4		AO_ADC_SENSE_PAD_3
#define AO_ADC_SQ5		AO_ADC_SENSE_PAD_4
#define AO_ADC_SQ6		AO_ADC_SENSE_PAD_5
#define AO_ADC_SQ7		AO_ADC_SENSE_PAD_6
#define AO_ADC_SQ8		AO_ADC_SENSE_PAD_7
#define AO_ADC_SQ9		AO_ADC_PYRO
#define AO_ADC_SQ10		AO_ADC_BATT

#define AO_ADC_REFERENCE_DV	33

#define AO_ADC_RCC_AHBENR	((1 << STM_RCC_AHBENR_GPIOAEN) | \
				 (1 << STM_RCC_AHBENR_GPIOBEN) | \
				 (1 << STM_RCC_AHBENR_GPIOCEN))


#define AO_PAD_R_V_BATT_BATT_SENSE	200
#define AO_PAD_R_BATT_SENSE_GND		22

#define AO_PAD_R_V_BATT_V_PYRO		200
#define AO_PAD_R_V_PYRO_PYRO_SENSE	200
#define AO_PAD_R_PYRO_SENSE_GND		22

#undef AO_PAD_R_V_PYRO_IGNITER
#define AO_PAD_R_IGNITER_IGNITER_SENSE	200
#define AO_PAD_R_IGNITER_SENSE_GND	22

#define HAS_ADC_TEMP		0

struct ao_adc {
	int16_t		sense[AO_PAD_NUM];
	int16_t		pyro;
	int16_t		batt;
};

#define AO_ADC_DUMP(p)							\
	printf ("tick: %5lu "						\
		"0: %5d 1: %5d 2: %5d 3: %5d "				\
		"4: %5d 5: %5d 6: %5d 7: %5d "				\
		"pyro: %5d batt: %5d\n",				\
		(p)->tick,						\
		(p)->adc.sense[0],					\
		(p)->adc.sense[1],					\
		(p)->adc.sense[2],					\
		(p)->adc.sense[3],					\
		(p)->adc.sense[4],					\
		(p)->adc.sense[5],					\
		(p)->adc.sense[6],					\
		(p)->adc.sense[7],					\
		(p)->adc.pyro,						\
		(p)->adc.batt)

#endif /* _AO_PINS_H_ */
