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
#include <string.h>
#include <ao_boot.h>

/* Interrupt functions */

void stm_halt_isr(void)
{
	ao_panic(AO_PANIC_CRASH);
}

void stm_ignore_isr(void)
{
}

void _start(void);

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
isr(rtc)
isr(flash)
isr(rcc_crs)
isr(exti1_0)
isr(exti3_2)
isr(exti15_4)
isr(dma1_channel1)
isr(dma1_channel3_2)
isr(dma1_channel7_4)
isr(adc_comp)
isr(lptim1)
isr(usart4_usart5)
isr(tim2)
isr(tim3)
isr(tim4)
isr(tim6)
isr(tim7)
isr(tim21)
isr(i2c3)
isr(tim22)
isr(i2c1)
isr(i2c2)
isr(spi1)
isr(spi2)
isr(usart1)
isr(usart2)
isr(usart3)
isr(lpuart1_aes)

#define i(addr,name)	[(addr)/4] = stm_ ## name ## _isr

extern char __stack[];
void _start(void) __attribute__((__noreturn__));
void main(void) __attribute__((__noreturn__));

__attribute__ ((section(".init")))
const void *const __interrupt_vector[] = {
	[0] = &__stack,
	[1] = _start,
	i(0x08, nmi),
	i(0x0c, hardfault),
	i(0x2c, svc),
	i(0x38, pendsv),
	i(0x3c, systick),
	i(0x40, wwdg),
	i(0x44, pvd),
	i(0x48, rtc),
	i(0x4c, flash),
	i(0x50, rcc_crs),
	i(0x54, exti1_0),
	i(0x58, exti3_2),
	i(0x5c, exti15_4),
	i(0x64, dma1_channel1),
	i(0x68, dma1_channel3_2),
	i(0x6c, dma1_channel7_4),
	i(0x70, adc_comp),
	i(0x74, lptim1),
	i(0x78, usart4_usart5),
	i(0x7c, tim2),
	i(0x80, tim3),
	i(0x84, tim6),
	i(0x88, tim7),
	i(0x90, tim21),
	i(0x94, i2c3),
	i(0x98, tim22),
	i(0x9c, i2c1),
	i(0xa0, i2c2),
	i(0xa4, spi1),
	i(0xa8, spi2),
	i(0xac, usart1),
	i(0xb0, usart2),
	i(0xb4, lpuart1_aes),
};

extern char __data_source[];
extern char __data_start[];
extern char __data_size[];
extern char __bss_start[];
extern char __bss_size[];

void _start(void)
{
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
	memcpy(__data_start, __data_source, (uintptr_t) __data_size);
	memset(__bss_start, '\0', (uintptr_t) __bss_size);

	main();
}
