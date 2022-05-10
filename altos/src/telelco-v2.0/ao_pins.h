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
#define BEEPER_TIMER		3
#define BEEPER_CHANNEL		1
#define BEEPER_PORT		(&stm_gpioc)
#define BEEPER_PIN		6
#define HAS_RADIO		1
#define HAS_RADIO_RATE		1
#define HAS_TELEMETRY		0
#define HAS_AES			1
#define HAS_STATIC_TEST		0

#define HAS_SPI_1		0
#define SPI_1_PA5_PA6_PA7	0
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_PE13_PE14_PE15	0

#define HAS_SPI_2		1	/* CC1200 */
#define SPI_2_PB13_PB14_PB15	0
#define SPI_2_PD1_PD3_PD4	1
#define SPI_2_GPIO		(&stm_gpiod)
#define SPI_2_SCK		1
#define SPI_2_MISO		3
#define SPI_2_MOSI		4
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

#define AO_CC1200_SPI_CS_PORT	(&stm_gpiod)
#define AO_CC1200_SPI_CS_PIN	0
#define AO_CC1200_SPI_BUS	AO_SPI_2_PD1_PD3_PD4
#define AO_CC1200_SPI		stm_spi2

#define AO_CC1200_INT_PORT		(&stm_gpiod)
#define AO_CC1200_INT_PIN		(5)

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

#define LOW_LEVEL_DEBUG		0

#define HAS_LED			1

/* PC7 - PC9, LED 0 - 2 */
#define LED_0_PORT		(&stm_gpioc)
#define LED_0_PIN		7
#define LED_1_PORT		(&stm_gpioc)
#define LED_1_PIN		8
#define LED_2_PORT		(&stm_gpioc)
#define LED_2_PIN		9

/* PD8 - PD10, LED 3 - 5 */
#define LED_3_PORT		(&stm_gpiod)
#define LED_3_PIN		8
#define LED_4_PORT		(&stm_gpiod)
#define LED_4_PIN		9
#define LED_5_PORT		(&stm_gpiod)
#define LED_5_PIN		10

/* PE2 - PE11 (not PE6), LED 6-14 */
#define LED_6_PORT		(&stm_gpioe)
#define LED_6_PIN		2
#define LED_7_PORT		(&stm_gpioe)
#define LED_7_PIN		3
#define LED_8_PORT		(&stm_gpioe)
#define LED_8_PIN		4
#define LED_9_PORT		(&stm_gpioe)
#define LED_9_PIN		5
#define LED_10_PORT		(&stm_gpioe)
#define LED_10_PIN		7
#define LED_11_PORT		(&stm_gpioe)
#define LED_11_PIN		8
#define LED_12_PORT		(&stm_gpioe)
#define LED_12_PIN		9
#define LED_13_PORT		(&stm_gpioe)
#define LED_13_PIN		10
#define LED_14_PORT		(&stm_gpioe)
#define LED_14_PIN		11

/* PA5, LED 15 */
#define LED_15_PORT		(&stm_gpioa)
#define LED_15_PIN		5

#define AO_LED_RED		AO_LED_0
#define AO_LED_AMBER		AO_LED_1
#define AO_LED_GREEN		AO_LED_2
#define AO_LED_BOX		AO_LED_3
#define AO_LED_PAD		AO_LED_4
#define AO_LED_DRAG		AO_LED_5
#define AO_LED_CONTINUITY_7	AO_LED_6
#define AO_LED_CONTINUITY_6	AO_LED_7
#define AO_LED_CONTINUITY_5	AO_LED_8
#define AO_LED_CONTINUITY_4	AO_LED_9
#define AO_LED_CONTINUITY_3	AO_LED_10
#define AO_LED_CONTINUITY_2	AO_LED_11
#define AO_LED_CONTINUITY_1	AO_LED_12
#define AO_LED_CONTINUITY_0	AO_LED_13
#define AO_LED_CONTINUITY_NUM	8
#define AO_LED_REMOTE_ARM	AO_LED_14
#define AO_LED_FIRE		AO_LED_15

/* LCD displays */

#define LCD_DEBUG		0
#define SEVEN_SEGMENT_DEBUG	0

#define AO_LCD_STM_SEG_ENABLED_0 (		\
		(1 << 0) | /* PA1 */		\
		(1 << 1) | /* PA2 */		\
		(1 << 2) | /* PA3 */		\
		(1 << 3) | /* PA6 */		\
		(1 << 4) | /* PA7 */		\
		(1 << 5) | /* PB0 */		\
		(1 << 6) | /* PB1 */		\
		(1 << 7) | /* PB3 */		\
		(1 << 8) | /* PB4 */		\
		(1 << 9) | /* PB5 */		\
		(1 << 10) | /* PB10 */		\
		(1 << 11) | /* PB11 */		\
		(1 << 12) | /* PB12 */		\
		(1 << 13) | /* PB13 */		\
		(1 << 14) | /* PB14 */		\
		(1 << 15) | /* PB15 */		\
		(1 << 16) | /* PB8 */		\
		(1 << 17) | /* PA15 */		\
		(1 << 18) | /* PC0 */		\
		(1 << 19) | /* PC1 */		\
		(1 << 20) | /* PC2 */		\
		(1 << 21) | /* PC3 */		\
		(1 << 22) | /* PC4 */		\
		(1 << 23) | /* PC5 */		\
		(0 << 24) | /* PC6 */		\
		(0 << 25) | /* PC7 */		\
		(0 << 26) | /* PC8 */		\
		(0 << 27) | /* PC9 */		\
		(0 << 28) | /* PC10 or PD8 */	\
		(0 << 29) | /* PC11 or PD9 */	\
		(0 << 30) | /* PC12 or PD10 */	\
		(0 << 31))  /* PD2 or PD11 */

#define AO_LCD_STM_SEG_ENABLED_1 (		\
		(0 << 0) | /* PD12 */		\
		(0 << 1) | /* PD13 */		\
		(0 << 2) | /* PD14 */		\
		(0 << 3) | /* PD15 */		\
		(0 << 4) | /* PE0 */		\
		(0 << 5) | /* PE1 */		\
		(0 << 6) | /* PE2 */		\
		(0 << 7))  /* PE3 */

#define AO_LCD_STM_COM_ENABLED (		\
		(1 << 0) | /* PA8 */		\
		(0 << 1) | /* PA9 */		\
		(0 << 2) | /* PA10 */		\
		(0 << 3) | /* PB9 */		\
		(0 << 4) | /* PC10 */		\
		(0 << 5) | /* PC11 */		\
		(0 << 6)) /* PC12 */

#define AO_LCD_28_ON_C	0

#define AO_LCD_DUTY	STM_LCD_CR_DUTY_STATIC

#define AO_LCD_PER_DIGIT	1

#define AO_LCD_DIGITS		3
#define AO_LCD_SEGMENTS		8

#define AO_SEGMENT_MAP {			\
		/* pad segments */		\
		{ 0, 14 },			\
		{ 0, 13 },			\
		{ 0, 15 },			\
		{ 0, 17 },			\
		{ 0, 16 },			\
		{ 0, 8 },			\
		{ 0, 9 },			\
		{ 0, 7 },			\
		/* box1 segments */		\
		{ 0, 10 },			\
		{ 0, 6 },			\
		{ 0, 11 },			\
		{ 0, 12 },			\
		{ 0, 21 },			\
		{ 0, 19 },			\
		{ 0, 20 },			\
		{ 0, 18 },			\
		/* box0 segments */		\
		{ 0, 22 },			\
		{ 0, 4 },			\
		{ 0, 23 },			\
		{ 0, 5 },			\
		{ 0, 3 },			\
		{ 0, 1 },			\
		{ 0, 2 },			\
		{ 0, 0 },			\
}

/*
 * Use event queue for input devices
 */

#define AO_EVENT		1

/*
 * Knobs
 */

#define AO_QUADRATURE_COUNT	1
#define AO_QUADRATURE_DEBOUNCE	0
#define AO_QUADRATURE_SINGLE_CODE	1

#define AO_QUADRATURE_0_PORT	&stm_gpioe
#define AO_QUADRATURE_0_A	15
#define AO_QUADRATURE_0_B	14

#define AO_QUADRATURE_SELECT	0

/*
 * Buttons
 */

#define AO_BUTTON_COUNT		9
#define AO_BUTTON_MODE		AO_EXTI_MODE_PULL_UP

#define AO_BUTTON_DRAG_MODE	0
#define AO_BUTTON_0_PORT	&stm_gpioe
#define AO_BUTTON_0		1

#define AO_BUTTON_DRAG_SELECT	1
#define AO_BUTTON_1_PORT	&stm_gpioe
#define AO_BUTTON_1		0

#define AO_BUTTON_SPARE1       	2
#define AO_BUTTON_2_PORT	&stm_gpiob
#define AO_BUTTON_2		9

#define AO_BUTTON_SPARE2      	3
#define AO_BUTTON_3_PORT	&stm_gpiob
#define AO_BUTTON_3		7

#define AO_BUTTON_SPARE3       	4
#define AO_BUTTON_4_PORT	&stm_gpiob
#define AO_BUTTON_4		6

#define AO_BUTTON_ARM		5
#define AO_BUTTON_5_PORT	&stm_gpioe
#define AO_BUTTON_5		12

#define AO_BUTTON_FIRE		6
#define AO_BUTTON_6_PORT	&stm_gpioa
#define AO_BUTTON_6		4

#define AO_BUTTON_SPARE4	7
#define AO_BUTTON_7_PORT	&stm_gpiod
#define AO_BUTTON_7		11

#define AO_BUTTON_ENCODER_SELECT	8
#define AO_BUTTON_8_PORT	&stm_gpioe
#define AO_BUTTON_8		13

/* ADC */

struct ao_adc {
	int16_t		v_batt;
};

#define AO_ADC_DUMP(p) \
	printf("batt: %5d\n", (p)->v_batt)

#define HAS_ADC_SINGLE		1
#define HAS_ADC_TEMP		0
#define HAS_BATTERY_REPORT	1

#define AO_ADC_V_BATT		0
#define AO_ADC_V_BATT_PORT	(&stm_gpioa)
#define AO_ADC_V_BATT_PIN	0

#define AO_ADC_RCC_AHBENR	(1 << STM_RCC_AHBENR_GPIOAEN)

#define AO_ADC_PIN0_PORT	AO_ADC_V_BATT_PORT
#define AO_ADC_PIN0_PIN		AO_ADC_V_BATT_PIN

#define AO_ADC_SQ1		AO_ADC_V_BATT

#define AO_NUM_ADC		1

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	15	/* 15k */
#define AO_BATTERY_DIV_MINUS	27	/* 27k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

#endif /* _AO_PINS_H_ */
