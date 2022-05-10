/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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
#include "stm32f4.h"
#include <string.h>
#include <ao_boot.h>

extern void main(void);
extern char __stack__;
extern char __text_start__, __text_end__;
extern char _start__, _end__;
extern char __bss_start__, __bss_end__;

/* Interrupt functions */

void stm_halt_isr(void)
{
	ao_panic(AO_PANIC_CRASH);
}

void stm_ignore_isr(void)
{
}

const void *stm_interrupt_vector[];

void start(void)
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
	/* Enable FPU */
	stm_scb.cpacr |= ((STM_SCB_CPACR_FULL << STM_SCB_CPACR_FP0) |
			  (STM_SCB_CPACR_FULL << STM_SCB_CPACR_FP1));
	ao_arch_nop();
	/* Set interrupt vector table offset */
	stm_scb.vtor = (uint32_t) &stm_interrupt_vector;
	memcpy(&_start__, &__text_end__, &_end__ - &_start__);
	memset(&__bss_start__, '\0', &__bss_end__ - &__bss_start__);
	main();
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
isr(dma1_stream0)
isr(dma1_stream1)
isr(dma1_stream2)
isr(dma1_stream3)
isr(dma1_stream4)
isr(dma1_stream5)
isr(dma1_stream6)
isr(adc)
isr(can1_tx)
isr(can1_rx0)
isr(can1_rx1)
isr(can1_sce)
isr(exti9_5)
isr(tim1_brk_tim9)
isr(tim1_up_tim10)
isr(tim_trg_com_tim11)
isr(tim1_cc)
isr(tim2)
isr(tim3)
isr(tim4)
isr(i2c1_evt)
isr(i2c1_err)
isr(i2c2_evt)
isr(i2c2_err)
isr(spi1)
isr(spi2)
isr(usart1)
isr(usart2)
isr(usart3)
isr(exti15_10)
isr(rtc_alarm)
isr(otg_fs_wkup)
isr(tim8_brk_tim12)
isr(tim8_up_tim13)
isr(tim8_trg_com_tim14)
isr(tim8_cc)
isr(dma1_stream7)
isr(fsmc)
isr(sdio)
isr(tim5)
isr(spi3)
isr(uart4)
isr(uart5)
isr(tim6_glb_it)
isr(tim7)
isr(dma2_stream0)
isr(dma2_stream1)
isr(dma2_stream2)
isr(dma2_stream3)
isr(dma2_stream4)
isr(dfsdm1_flt0)
isr(dfsdm1_flt1)
isr(can2_tx)
isr(can2_rx0)
isr(can2_rx1)
isr(can2_sce)
isr(otg_fs)
isr(dma2_stream5)
isr(dma2_stream6)
isr(dma2_stream7)
isr(usart6)
isr(i2c3_ev)
isr(i2c3_er)
isr(can3_tx)
isr(can3_rx0)
isr(can3_rx1)
isr(can3_sce)
isr(crypto)
isr(rng)
isr(fpu)
isr(uart7)
isr(uart8)
isr(spi4)
isr(spi5)
isr(sai1)
isr(uart9)
isr(uart10)
isr(quad_spi)
isr(i2cfmp1_ev)
isr(i2cfmp1_er)
isr(exti23)
isr(dfsdm2_flt0)
isr(dfsdm2_flt1)
isr(dfsdm2_flt2)
isr(dfsdm2_flt3)

#define i(addr,name)	[(addr)/4] = stm_ ## name ## _isr

__attribute__ ((section(".interrupt")))
const void *stm_interrupt_vector[] = {
	[0] = &__stack__,
	[1] = start,
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
	i(0x6c, dma1_stream0),
	i(0x70, dma1_stream1),
	i(0x74, dma1_stream2),
	i(0x78, dma1_stream3),
	i(0x7c, dma1_stream4),
	i(0x80, dma1_stream5),
	i(0x84, dma1_stream6),
	i(0x88, adc),
	i(0x8c, can1_tx),
	i(0x90, can1_rx0),
	i(0x94, can1_rx1),
	i(0x98, can1_sce),
	i(0x9c, exti9_5),
	i(0xa0, tim1_brk_tim9),
	i(0xa4, tim1_up_tim10),
	i(0xa8, tim_trg_com_tim11),
	i(0xac, tim1_cc),
	i(0xb0, tim2),
	i(0xb4, tim3),
	i(0xb8, tim4),
	i(0xbc, i2c1_evt),
	i(0xc0, i2c1_err),
	i(0xc4, i2c2_evt),
	i(0xc8, i2c2_err),
	i(0xcc, spi1),
	i(0xd0, spi2),
	i(0xd4, usart1),
	i(0xd8, usart2),
	i(0xdc, usart3),
	i(0xe0, exti15_10),
	i(0xe4, rtc_alarm),
	i(0xe8, otg_fs_wkup),
	i(0xec, tim8_brk_tim12),
	i(0xf0, tim8_up_tim13),
	i(0xf4, tim8_trg_com_tim14),
	i(0xf8, tim8_cc),
	i(0xfc, dma1_stream7),
	i(0x100, fsmc),
	i(0x104, sdio),
	i(0x108, tim5),
	i(0x10c, spi3),
	i(0x110, uart4),
	i(0x114, uart5),
	i(0x118,tim6_glb_it),
	i(0x11c, tim7),
	i(0x120, dma2_stream0),
	i(0x124, dma2_stream1),
	i(0x128, dma2_stream2),
	i(0x12c, dma2_stream3),
	i(0x130, dma2_stream4),
	i(0x134, dfsdm1_flt0),
	i(0x138, dfsdm1_flt1),
	i(0x13c, can2_tx),
	i(0x140, can2_rx0),
	i(0x144, can2_rx1),
	i(0x148, can2_sce),
	i(0x14c, otg_fs),
	i(0x150, dma2_stream5),
	i(0x154, dma2_stream6),
	i(0x158, dma2_stream7),
	i(0x15c, usart6),
	i(0x160, i2c3_ev),
	i(0x164, i2c3_er),
	i(0x168, can3_tx),
	i(0x16c, can3_rx0),
	i(0x170, can3_rx1),
	i(0x174, can3_sce),
	i(0x17c, crypto),
	i(0x180, rng),
	i(0x184, fpu),
	i(0x188, uart7),
	i(0x18c, uart8),
	i(0x190, spi4),
	i(0x194, spi5),
	i(0x19c, sai1),
	i(0x1a0, uart9),
	i(0x1a4, uart10),
	i(0x1b0, quad_spi),
	i(0x1bc, i2cfmp1_ev),
	i(0x1c0, i2cfmp1_er),
	i(0x1c4, exti23),
	i(0x1c8, dfsdm2_flt0),
	i(0x1cc, dfsdm2_flt1),
	i(0x1d0, dfsdm2_flt2),
	i(0x1d4, dfsdm2_flt3),
};
