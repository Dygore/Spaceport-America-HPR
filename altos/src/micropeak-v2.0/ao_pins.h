/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

extern uint8_t ao_on_battery;

#define HAS_TASK	0

#define AO_SYSCLK	STM_MSI_FREQ_524288
#define AO_MSI_RANGE	STM_RCC_ICSCR_MSIRANGE_524288

#define LED_0_PORT	(&stm_gpioa)
#define LED_0_PIN	1
#define AO_LED_ORANGE	(1 << 0)
#define AO_LED_REPORT	AO_LED_ORANGE
#define AO_LED_PANIC	AO_LED_ORANGE

#define LEDS_AVAILABLE	(AO_LED_ORANGE)

#define AO_POWER_MANAGEMENT	0

/* HCLK = MSI (2.097MHz) */
#define AO_AHB_PRESCALER	(1)
#define AO_RCC_CFGR_HPRE_DIV	(STM_RCC_CFGR_HPRE_DIV_1)

/* APB = MSI */
#define AO_APB1_PRESCALER	(1)
#define AO_APB2_PRESCALER	(1)
#define AO_RCC_CFGR_PPRE_DIV	(STM_RCC_CFGR_PPRE_DIV_1)

#define PACKET_HAS_SLAVE	0
#define HAS_SERIAL_1		0
#define HAS_SERIAL_2		1
#define USE_SERIAL_2_STDIN	1
#define USE_SERIAL_2_FLOW	0
#define USE_SERIAL_2_SW_FLOW	0
#define SERIAL_2_PA9_PA10	1

#define HAS_LPUART_1		1
#define LPUART_1_PA0_PA1	1
#define USE_LPUART_1_STDIN	0
#define USE_LPUART_1_FLOW	0
#define USE_LPUART_1_SW_FLOW	0

#define IS_FLASH_LOADER		0

#define HAS_MS5607		1
#define HAS_SENSOR_ERRORS	0
#define HAS_MS5611		0
#define HAS_MS5607_TASK		0
#define HAS_EEPROM		1
#define HAS_CONFIG_SAVE		0
#define HAS_BEEP		0

/* Logging */
#define LOG_INTERVAL		1
#define SAMPLE_SLEEP		AO_MS_TO_TICKS(100)
#define BOOST_DELAY		AO_SEC_TO_TICKS(60)
#define AO_LOG_ID		AO_LOG_ID_MICROPEAK2
#define HAS_LOG			1
#define AO_LOG_FORMAT		AO_LOG_FORMAT_MICROPEAK2
#define FLIGHT_LOG_APPEND	1

/* Kalman filter */

#define AO_MK_STEP_100MS	1
#define AO_MK_STEP_96MS		0

/* SPI */
#define HAS_SPI_1		1
#define SPI_1_POWER_MANAGE	1
#define SPI_1_PA5_PA6_PA7	1
#define SPI_1_PB3_PB4_PB5	0
#define SPI_1_OSPEEDR		STM_OSPEEDR_MEDIUM

#define HAS_SPI_2		0

/* MS5607 */
#define HAS_MS5607		1
#define HAS_MS5611		0
#define AO_MS5607_PRIVATE_PINS	0
#define AO_MS5607_CS_PORT	(&stm_gpioa)
#define AO_MS5607_CS_PIN	4
#define AO_MS5607_CS_MASK	(1 << AO_MS5607_CS_PIN)
#define AO_MS5607_MISO_PORT	(&stm_gpioa)
#define AO_MS5607_MISO_PIN	6
#define AO_MS5607_MISO_MASK	(1 << AO_MS5607_MISO_PIN)
#define AO_MS5607_SPI_INDEX	AO_SPI_1_PA5_PA6_PA7

typedef int32_t alt_t;

#define AO_ALT_VALUE(x)		((x) * (alt_t) 10)

#define HAS_ADC			0
#define HAS_AO_DELAY		1

static inline void
ao_power_off(void) __attribute((noreturn));

static inline void
ao_power_off(void) {
	for (;;) {
	}
}

extern alt_t ao_max_height;

#define ao_async_stop()
#define ao_async_start()

#define LOG_MICRO_ASYNC 0

void ao_async_byte(char c);

#define ao_eeprom_read(pos, ptr, size) ao_storage_device_read(pos, ptr, size)
#define ao_eeprom_write(pos, ptr, size) ao_storage_device_write(pos, ptr, size)
#define N_SAMPLES_TYPE uint32_t
#define MAX_LOG_OFFSET	ao_storage_total
#define ao_storage_log_max ao_storage_total

extern uint32_t __flash__[];
extern uint32_t __flash_end__[];

#define AO_BOOT_APPLICATION_BOUND	((uint32_t *) __flash__)
#define USE_STORAGE_CONFIG	0

#define HAS_STORAGE_DEBUG 1

#endif /* _AO_PINS_H_ */
