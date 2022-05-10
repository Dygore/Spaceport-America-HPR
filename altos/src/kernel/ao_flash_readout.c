/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <ao.h>
#include <ao_usb.h>
#include <ao_flash_readout.h>

#ifndef AO_FLASH_READOUT_BASE
#define AO_FLASH_READOUT_BASE	AO_BOOT_LOADER_BASE
#define AO_FLASH_READOUT_BOUND	AO_BOOT_APPLICATION_BOUND
#endif

static void
ao_flash_readout(void)
{
	uint8_t	*base = (uint8_t *) AO_FLASH_READOUT_BASE;
	uint8_t	*bound = (uint8_t *) AO_FLASH_READOUT_BOUND;
	uint8_t	*p = base;

	for (;;) {
		ao_arch_block_interrupts();
		while (!ao_usb_running) {
			p = base;
			ao_sleep(&ao_usb_running);
		}
		ao_arch_release_interrupts();
		ao_flash_readout_putchar(*p++);
		if (p == bound)
			p = base;
	}
}

static struct ao_task	ao_flash_readout_task;

void
ao_flash_readout_init(void)
{
	ao_add_task(&ao_flash_readout_task, ao_flash_readout, "flash_readout");
}
