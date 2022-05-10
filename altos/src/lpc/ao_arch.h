/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_ARCH_H_
#define _AO_ARCH_H_

#include <lpc.h>

/*
 * LPC11U14 definitions and code fragments for AltOS
 */

#ifndef AO_STACK_SIZE
#define AO_STACK_SIZE	512
#endif

#define AO_PORT_TYPE	uint32_t

#define AO_LED_TYPE	AO_PORT_TYPE

/* Various definitions to make GCC look more like SDCC */

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define
#define __interrupt(n)
#define __at(n)

#define ao_arch_reboot() arm_scb.aircr = ((0x05fa << 16) |	\
					  (0 << 15) |		\
					  (1 << 2) |		\
					  (0 << 1))

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

/*
 * ao_romconfig.c
 */

#define AO_ROMCONFIG_SYMBOL __attribute__((section(".init.1"))) const
#define AO_USBCONFIG_SYMBOL __attribute__((section(".init.2"))) const

#define ao_arch_block_interrupts()	asm("cpsid i")
#define ao_arch_release_interrupts()	asm("cpsie i")

/*
 * For now, we're running at a weird frequency
 */

#if AO_HSE
#define AO_PLLSRC	AO_HSE
#else
#define AO_PLLSRC	STM_HSI_FREQ
#endif

#define AO_PLLVCO	(AO_PLLSRC * AO_PLLMUL)
#define AO_SYSCLK	(AO_PLLVCO / AO_PLLDIV)
#define AO_HCLK		(AO_SYSCLK / AO_AHB_PRESCALER)
#define AO_PCLK1	(AO_HCLK / AO_APB1_PRESCALER)
#define AO_PCLK2	(AO_HCLK / AO_APB2_PRESCALER)

#if AO_APB1_PRESCALER == 1
#define AO_TIM23467_CLK		AO_PCLK1
#else
#define AO_TIM23467_CLK		(2 * AO_PCLK1)
#endif

#if AO_APB2_PRESCALER == 1
#define AO_TIM91011_CLK		AO_PCLK2
#else
#define AO_TIM91011_CLK		(2 * AO_PCLK2)
#endif

#define AO_LPC_NVIC_HIGH_PRIORITY	0
#define AO_LPC_NVIC_CLOCK_PRIORITY	1
#define AO_LPC_NVIC_MED_PRIORITY	2
#define AO_LPC_NVIC_LOW_PRIORITY	3

void
ao_adc_init(void);

#define AO_USB_OUT_EP	2
#define AO_USB_IN_EP	3

void
ao_serial_init(void);

/* SPI definitions */

#define _AO_SPI_SPEED_12MHz		4
#define _AO_SPI_SPEED_8MHz		6
#define _AO_SPI_SPEED_6MHz		8
#define _AO_SPI_SPEED_4MHz		12
#define _AO_SPI_SPEED_2MHz		24
#define _AO_SPI_SPEED_1MHz		48
#define _AO_SPI_SPEED_500kHz		96
#define _AO_SPI_SPEED_250kHz		192
#define _AO_SPI_SPEED_125kHz		384
#define _AO_SPI_SPEED_62500Hz		768

static inline uint32_t
ao_spi_speed(uint32_t hz)
{
	if (hz >= 4000000) return _AO_SPI_SPEED_4MHz;
	if (hz >= 2000000) return _AO_SPI_SPEED_2MHz;
	if (hz >= 1000000) return _AO_SPI_SPEED_1MHz;
	if (hz >=  500000) return _AO_SPI_SPEED_500kHz;
	if (hz >=  250000) return _AO_SPI_SPEED_250kHz;
	if (hz >=  125000) return _AO_SPI_SPEED_125kHz;
	return _AO_SPI_SPEED_62500Hz;
}

#define AO_BOOT_APPLICATION_BASE	((uint32_t *) 0x00001000)
#define AO_BOOT_APPLICATION_BOUND	((uint32_t *) (0x00000000 + 32 * 1024))
#define AO_BOOT_LOADER_BASE		((uint32_t *) 0x00000000)
#define HAS_BOOT_LOADER			1

/* ADC definitions */

#define AO_ADC_MAX	32767

#endif /* _AO_ARCH_H_ */
