/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

/* External crystal at 8MHz */
#define AO_HSE		8000000

#define AO_WATCHDOG_PORT	(&stm_gpiod)
#define AO_WATCHDOG_PORT_BIT	(1 << STM_RCC_AHBENR_GPIODEN)
#define AO_WATCHDOG_BIT		3
#define AO_WATCHDOG_VALUE	0

#define AO_RADIO_CONTROL_PORT		(&stm_gpioe)
#define AO_RADIO_CONTROL_PORT_BIT	(1 << STM_RCC_AHBENR_GPIOEEN)
#define AO_RADIO_CONTROL_BIT		12
#define AO_RADIO_CONTROL_VALUE		1

#define AO_FLASH_LOADER_INIT do {					\
		stm_rcc.ahbenr |= AO_WATCHDOG_PORT_BIT | AO_RADIO_CONTROL_PORT_BIT; \
									\
		stm_gpio_set(AO_WATCHDOG_PORT, AO_WATCHDOG_BIT, AO_WATCHDOG_VALUE); \
		stm_moder_set(AO_WATCHDOG_PORT, AO_WATCHDOG_BIT, STM_MODER_OUTPUT); \
									\
		stm_gpio_set(AO_RADIO_CONTROL_PORT, AO_RADIO_CONTROL_BIT, AO_RADIO_CONTROL_VALUE); \
		stm_moder_set(AO_RADIO_CONTROL_PORT, AO_RADIO_CONTROL_BIT, STM_MODER_OUTPUT); \
	} while (0)

#define AO_TIMER_HOOK	do {						\
		AO_WATCHDOG_PORT->odr ^= (1 << AO_WATCHDOG_BIT);	\
	} while (0)

#define HAS_TICK		0
#include <ao_flash_stm_pins.h>

/* Attached signal, PB8 */

#define AO_BOOT_PIN		1
#define AO_BOOT_APPLICATION_GPIO	stm_gpiob
#define AO_BOOT_APPLICATION_PIN		8
#define AO_BOOT_APPLICATION_VALUE	0
#define AO_BOOT_APPLICATION_MODE	0

#endif /* _AO_PINS_H_ */
