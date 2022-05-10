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

#include <ao.h>
#include "stm32l.h"
#include <string.h>
#include <ao_boot.h>

extern void main(void);

/* Interrupt functions */

void stm_halt_isr(void)
{
	ao_panic(AO_PANIC_CRASH);
}

void stm_ignore_isr(void)
{
}

uint32_t
stm_flash_size(void) {
	uint16_t	dev_id = stm_dev_id();
	uint16_t	kbytes = 0;

	switch (dev_id) {
	case 0x416:	/* cat 1 */
		kbytes = stm_flash_size_medium.f_size;
		break;
	case 0x429:	/* cat 2 */
		kbytes = stm_flash_size_medium.f_size & 0xff;
		break;
	case 0x427:	/* cat 3 */
		kbytes = stm_flash_size_large.f_size;
		break;
	case 0x436:	/* cat 4 */
		switch (stm_flash_size_large.f_size) {
		case 0:
			kbytes = 256;
			break;
		case 1:
			kbytes = 384;
			break;
		}
		break;
	case 0x437:	/* cat 5 */
		kbytes = stm_flash_size_large.f_size;
		break;
	}
	return (uint32_t) kbytes * 1024;
}

#define STRINGIFY(x) #x

#define isr(name) \
	void __attribute__ ((weak)) stm_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak stm_ ## name ## _isr = stm_ignore_isr))

#define isr_halt(name) \
	void __attribute__ ((weak)) stm_ ## name ## _isr(void); \
	_Pragma(STRINGIFY(weak stm_ ## name ## _isr = stm_halt_isr))

isr(nmi)
isr_halt(hardfault)
isr_halt(memmanage)
isr_halt(busfault)
isr_halt(usagefault)
isr(svc)
isr(debugmon)
isr(pendsv)
isr(systick)
isr(wwdg)
isr(pvd)
isr(tamper_stamp)
isr(rtc_wkup)
isr(flash)
isr(rcc)
isr(exti0)
isr(exti1)
isr(exti2)
isr(exti3)
isr(exti4)
isr(dma1_channel1)
isr(dma1_channel2)
isr(dma1_channel3)
isr(dma1_channel4)
isr(dma1_channel5)
isr(dma1_channel6)
isr(dma1_channel7)
isr(adc1)
isr(usb_hp)
isr(usb_lp)
isr(dac)
isr(comp)
isr(exti9_5)
isr(lcd)
isr(tim9)
isr(tim10)
isr(tim11)
isr(tim2)
isr(tim3)
isr(tim4)
isr(i2c1_ev)
isr(i2c1_er)
isr(i2c2_ev)
isr(i2c2_er)
isr(spi1)
isr(spi2)
isr(usart1)
isr(usart2)
isr(usart3)
isr(exti15_10)
isr(rtc_alarm)
isr(usb_fs_wkup)
isr(tim6)
isr(tim7)

#define i(addr,name)	[(addr)/4] = stm_ ## name ## _isr

extern char __stack[];
void _start(void) __attribute__((__noreturn__));
void main(void) __attribute__((__noreturn__));
void ao_setup(void) __attribute__((constructor));

/* This must be exactly 256 bytes long so that the configuration data
 * gets loaded at the right place
 */

__attribute__ ((section(".init")))
const void * const __interrupt_vector[64] = {
	[0] = &__stack,
	[1] = _start,
	i(0x08, nmi),
	i(0x0c, hardfault),
	i(0x10, memmanage),
	i(0x14, busfault),
	i(0x18, usagefault),
	i(0x2c, svc),
	i(0x30, debugmon),
	i(0x38, pendsv),
	i(0x3c, systick),
	i(0x40, wwdg),
	i(0x44, pvd),
	i(0x48, tamper_stamp),
	i(0x4c, rtc_wkup),
	i(0x50, flash),
	i(0x54, rcc),
	i(0x58, exti0),
	i(0x5c, exti1),
	i(0x60, exti2),
	i(0x64, exti3),
	i(0x68, exti4),
	i(0x6c, dma1_channel1),
	i(0x70, dma1_channel2),
	i(0x74, dma1_channel3),
	i(0x78, dma1_channel4),
	i(0x7c, dma1_channel5),
	i(0x80, dma1_channel6),
	i(0x84, dma1_channel7),
	i(0x88, adc1),
	i(0x8c, usb_hp),
	i(0x90, usb_lp),
	i(0x94, dac),
	i(0x98, comp),
	i(0x9c, exti9_5),
	i(0xa0, lcd),
	i(0xa4, tim9),
	i(0xa8, tim10),
	i(0xac, tim11),
	i(0xb0, tim2),
	i(0xb4, tim3),
	i(0xb8, tim4),
	i(0xbc, i2c1_ev),
	i(0xc0, i2c1_er),
	i(0xc4, i2c2_ev),
	i(0xc8, i2c2_er),
	i(0xcc, spi1),
	i(0xd0, spi2),
	i(0xd4, usart1),
	i(0xd8, usart2),
	i(0xdc, usart3),
	i(0xe0, exti15_10),
	i(0xe4, rtc_alarm),
	i(0xe8, usb_fs_wkup),
	i(0xec, tim6),
	i(0xf0, tim7),
};

void __attribute__((constructor)) ao_setup(void) {
#ifdef AO_BOOT_CHAIN
	if (ao_boot_check_chain()) {
#ifdef AO_BOOT_PIN
		if (ao_boot_check_pin())
#endif
		{
			ao_boot_chain(AO_BOOT_APPLICATION_BASE);
		}
	}
#endif
	/* Set interrupt vector table offset */
	stm_nvic.vto = (uint32_t) &__interrupt_vector;
}
