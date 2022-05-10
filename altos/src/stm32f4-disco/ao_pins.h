/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#ifndef _AO_PINS_H_
#define _AO_PINS_H_

/* Clock tree configuration */
#define AO_HSE		8000000	/* fed from st/link processor */
#define AO_HSE_BYPASS	1	/* no xtal, directly fed */

#define AO_PLL_M	8	/* down to 1MHz */

#define AO_PLL1_R	2	/* down to 96MHz */
#define AO_PLL1_N	192	/* up to 192MHz */
#define AO_PLL1_P	2	/* down to 96MHz */
#define AO_PLL1_Q	4	/* down to 48MHz for USB and SDIO */

#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

#define AO_APB1_PRESCALER	2
#define AO_RCC_CFGR_PPRE1_DIV	STM_RCC_CFGR_PPRE1_DIV_2

#define AO_APB2_PRESCALER	1
#define AO_RCC_CFGR_PPRE2_DIV	STM_RCC_CFGR_PPRE2_DIV_1

#define DEBUG_THE_CLOCK	1

#define HAS_BEEP	0

#define B_USER_PORT	(&stm_gpioa)
#define B_USER_PIN	0

/* LEDs */

#define HAS_LED		1

#define LED_0_PORT	(&stm_gpioc)
#define LED_0_PIN	5
#define LED_GREEN	AO_LED_0

#define LED_1_PORT	(&stm_gpioe)
#define LED_1_PIN	3
#define LED_RED		AO_LED_0

#define AO_LED_PANIC	LED_RED

#define AO_CMD_LEN	128

/* USART */

#define HAS_SERIAL_6		1
#define SERIAL_6_RX_PORT	(&stm_gpiog)
#define SERIAL_6_RX_PIN		9

#define SERIAL_6_TX_PORT	(&stm_gpiog)
#define SERIAL_6_TX_PIN		14

#define USE_SERIAL_6_STDIN	1
#define DELAY_SERIAL_6_STDIN	0
#define USE_SERIAL_6_FLOW	0
#define USE_SERIAL_6_SW_FLOW	0

/* USB */

#define HAS_USB			1
#define USE_USB_STDIO		0

#endif /* _AO_PINS_H_ */
