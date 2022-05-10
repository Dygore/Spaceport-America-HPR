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

/* Using TeleDongle v3.0 board */

#ifndef _AO_PINS_H_
#define _AO_PINS_H_

#define AO_STACK_SIZE	320


#define IS_FLASH_LOADER	0

/* Crystal on the board */
#define AO_LPC_CLKIN	12000000

/* Main clock frequency. 48MHz for USB so we don't use the USB PLL */
#define AO_LPC_CLKOUT	48000000

/* System clock frequency */
#define AO_LPC_SYSCLK	24000000

#define HAS_EEPROM		0
#define USE_INTERNAL_FLASH	0
#define USE_STORAGE_CONFIG	0
#define USE_EEPROM_CONFIG	0

#define HAS_USB			1
#define HAS_USB_CONNECT		0
#define HAS_USB_VBUS		0
#define HAS_USB_PULLUP		1
#define AO_USB_PULLUP_PORT	0
#define AO_USB_PULLUP_PIN	20

#define HAS_BEEP		0
#define HAS_RADIO		1
#define HAS_TELEMETRY		0
#define HAS_RSSI		0

#define HAS_SPI_0		1
#define SPI_SCK0_P0_6		1

#define PACKET_HAS_SLAVE	0
#define PACKET_HAS_MASTER	1

#define LOW_LEVEL_DEBUG		0

#define LED_PORT		0
#define LED_PIN_RED		14
#define LED_PIN_GREEN		7
#define AO_LED_RED		(1 << LED_PIN_RED)
#define AO_LED_GREEN		(1 << LED_PIN_GREEN)

#define LEDS_AVAILABLE		(AO_LED_RED | AO_LED_GREEN)

#define HAS_GPS			0
#define HAS_FLIGHT		0
#define HAS_ADC			0
#define HAS_LOG			0

/*
 * Telemetry monitoring
 */
#define HAS_MONITOR		1
#define LEGACY_MONITOR		0
#define HAS_MONITOR_PUT		1
#define AO_MONITOR_LED		AO_LED_GREEN
#define AO_MONITOR_BAD		AO_LED_RED

/*
 * Radio (cc1200)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT 	5695733

#define AO_FEC_DEBUG		0
#define AO_CC1200_SPI_CS_PORT	0
#define AO_CC1200_SPI_CS_PIN	3
#define AO_CC1200_SPI_BUS	0
#define AO_CC1200_SPI		0

#define AO_CC1200_INT_PORT	0
#define AO_CC1200_INT_PIN	2

#define AO_CC1200_INT_GPIO	2
#define AO_CC1200_INT_GPIO_IOCFG	CC1200_IOCFG2

/*
 * Profiling Viterbi decoding
 */

#ifndef AO_PROFILE
#define AO_PROFILE	       	0
#endif

#endif /* _AO_PINS_H_ */
