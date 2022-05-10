/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU  8000000UL	// 8 MHz

/*
 * AVR definitions and code fragments for AltOS
 */

#define AO_STACK_SIZE	116

#define AO_PORT_TYPE	uint8_t

#define AO_LED_TYPE	uint8_t

#ifndef AO_TICK_TYPE
#define AO_TICK_TYPE	uint16_t
#define AO_TICK_SIGNED	int16_t
#endif

/* Various definitions to make GCC look more like SDCC */

#define ao_arch_naked_declare	__attribute__((naked))
#define ao_arch_naked_define
#define __interrupt(n)
#define __at(n)

#define ao_arch_reboot()	/* XXX */

#define ao_arch_nop()		asm("nop")

#define ao_arch_interrupt(n)	/* nothing */

#define AO_ROMCONFIG_SYMBOL __attribute__((section(".romconfig"))) const

#undef putchar
#undef getchar
#define putchar(c)	ao_putchar(c)
#define getchar		ao_getchar

#define ao_arch_wait_interrupt() do {		\
		sleep_enable();			\
		sei();				\
		sleep_cpu();			\
		sleep_disable();		\
	} while (0)

#define ao_arch_critical(b) do { cli(); do { b } while (0); sei(); } while (0)

#define ao_arch_block_interrupts()	cli()
#define ao_arch_release_interrupts()	sei()

void
ao_delay_until(AO_TICK_TYPE target);

/* We can't hit 100 Hz, but we can hit 125 */
#define AO_HERTZ	125

void
ao_eeprom_read(uint16_t addr, void *buf, uint16_t len);

void
ao_eeprom_write(uint16_t addr, void *buf, uint16_t len);

uint16_t
ao_adc_read(uint8_t mux);

#endif /* _AO_ARCH_H_ */
