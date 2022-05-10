/*
 * Copyright © 2017 Bdale Garbee <bdale@gag.com>
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

#define LED_PORT_ENABLE STM_RCC_AHBENR_IOPBEN
#define LED_PORT        (&stm_gpiob)
#define LED_PIN_GREEN   5
#define AO_LED_GREEN    (1 << LED_PIN_GREEN)
#define AO_LED_PANIC	AO_LED_GREEN
#define AO_LED_GPS_LOCK	AO_LED_GREEN

#define LEDS_AVAILABLE  (AO_LED_GREEN)

#define AO_STACK_SIZE		512

#define IS_FLASH_LOADER		0
#define HAS_BEEP 	       0

#define AO_HSE                  16000000
#define AO_RCC_CFGR_PLLMUL      STM_RCC_CFGR_PLLMUL_3
#define AO_RCC_CFGR2_PLLDIV	STM_RCC_CFGR2_PREDIV_1
#define AO_PLLMUL               3
#define AO_PLLDIV               1

/* HCLK = 48MHz */
#define AO_AHB_PRESCALER        1
#define AO_RCC_CFGR_HPRE_DIV    STM_RCC_CFGR_HPRE_DIV_1

/* APB = 48MHz */
#define AO_APB_PRESCALER        1
#define AO_RCC_CFGR_PPRE_DIV    STM_RCC_CFGR_PPRE_DIV_1

#define HAS_USB                         1
#define AO_USB_DIRECTIO                 0
#define AO_PA11_PA12_RMP                1
#define HAS_USB_CONNECT			1
#define AO_USB_CONNECT_PORT		(&stm_gpiob)
#define AO_USB_CONNECT_PIN		3

#define IS_FLASH_LOADER 0

/* ADC */

#define HAS_ADC			1
#define AO_ADC_PIN0_PORT        (&stm_gpiob)
#define AO_ADC_PIN0_PIN         1
#define AO_ADC_PIN0_CH          9

#define AO_ADC_RCC_AHBENR       ((1 << STM_RCC_AHBENR_IOPBEN))

#define ao_telemetry_battery_convert(a)	((a) << 3)

#define AO_NUM_ADC              1

#define AO_DATA_RING		4

/*
 * Voltage divider on ADC battery sampler
 */
#define AO_BATTERY_DIV_PLUS	56	/* 5.6k */
#define AO_BATTERY_DIV_MINUS	100	/* 10k */

/*
 * ADC reference in decivolts
 */
#define AO_ADC_REFERENCE_DV	33

struct ao_adc {
        int16_t                 v_batt;
};

#define AO_ADC_DUMP(p) \
        printf("tick: %5lu batt: %5d\n", \
               (p)->tick, \
               (p)->adc.v_batt)

/* SPI */
#define HAS_SPI_1               1
#define HAS_SPI_2               0
#define SPI_1_PA5_PA6_PA7       1
#define SPI_1_PB3_PB4_PB5       0
#define SPI_1_OSPEEDR           STM_OSPEEDR_HIGH

/* Flash */

#define M25_MAX_CHIPS           1
#define AO_M25_SPI_CS_PORT      (&stm_gpiob)
#define AO_M25_SPI_CS_MASK      (1 << 0)
#define AO_M25_SPI_BUS          AO_SPI_1_PA5_PA6_PA7

/* Serial */
#define HAS_SERIAL_1		0
#define SERIAL_1_PB6_PB7	1
#define USE_SERIAL_1_STDIN	0

#define HAS_SERIAL_2	       	1
#define SERIAL_2_PA2_PA3	1
#define USE_SERIAL_2_STDIN	0
#define USE_SERIAL_2_FLOW       0
#define USE_SERIAL_2_SW_FLOW    0

#define ao_gps_getchar		ao_serial2_getchar
#define ao_gps_putchar		ao_serial2_putchar
#define ao_gps_set_speed	ao_serial2_set_speed

#define HAS_EEPROM		1
#define USE_INTERNAL_FLASH	0
#define HAS_RADIO		1
#define HAS_TELEMETRY		1
#define HAS_RDF			1
#define HAS_APRS		1

#define HAS_GPS			1
#define HAS_FLIGHT		0
#define HAS_LOG			1
#define FLIGHT_LOG_APPEND	1
#define HAS_TRACKER		1
#define LOG_ADC			0

#define AO_CONFIG_DEFAULT_APRS_INTERVAL		0
#define AO_CONFIG_DEFAULT_RADIO_POWER		0xc0
#define AO_LOG_FORMAT				AO_LOG_FORMAT_TELEGPS

/*
 * GPS
 */

#define AO_SERIAL_SPEED_UBLOX	AO_SERIAL_SPEED_9600


/*
 * Radio (cc1120)
 */

/* gets pretty close to 434.550 */

#define AO_RADIO_CAL_DEFAULT    5695733

#define AO_FEC_DEBUG            0
#define AO_CC1200_SPI_CS_PORT   (&stm_gpioa)
#define AO_CC1200_SPI_CS_PIN    1
#define AO_CC1200_SPI_BUS       AO_SPI_1_PA5_PA6_PA7
#define AO_CC1200_SPI           stm_spi1

#define AO_CC1200_INT_PORT              (&stm_gpioa)
#define AO_CC1200_INT_PIN               4

#define AO_CC1200_INT_GPIO      2
#define AO_CC1200_INT_GPIO_IOCFG        CC1200_IOCFG2

#define HAS_BOOT_RADIO          0

#endif /* _AO_PINS_H_ */
