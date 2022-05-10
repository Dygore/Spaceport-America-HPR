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


#define IS_FLASH_LOADER	0

#define AO_POWER_MANAGEMENT	0

/* 48MHz clock based on USB */
#define AO_HSI48	1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER	1
#define AO_RCC_CFGR_HPRE_DIV	STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER	1
#define AO_RCC_CFGR_PPRE_DIV	STM_RCC_CFGR_PPRE_DIV_1

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define USE_STORAGE_CONFIG	0
#define USE_EEPROM_CONFIG	0

#define HAS_BEEP		0

#define HAS_USB			1
#define AO_USB_DIRECTIO		0
#define AO_PA11_PA12_RMP	1

#define HAS_SPI_0		0

#define AO_SENSOR_PORT		(&stm_gpioa)
#define AO_SENSOR_PIN		3

#define LOW_LEVEL_DEBUG		0

#endif /* _AO_PINS_H_ */
