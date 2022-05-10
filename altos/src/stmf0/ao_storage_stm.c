/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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
#include <ao_storage.h>

uint32_t	ao_storage_block;
ao_pos_t	ao_storage_total;
uint16_t	ao_storage_unit;

/* Note that the HSI clock must be running for this code to work.
 * Also, special care must be taken with the linker to ensure that the
 * functions marked 'ramtext' land in ram and not rom. An example of that
 * can be found in altos-loader.ld
 */

static uint8_t
ao_flash_is_locked(void)
{
	return (stm_flash.cr & (1 << STM_FLASH_CR_LOCK)) != 0;
}

static void
ao_flash_unlock(void)
{
	if (!ao_flash_is_locked())
		return;

	/* Unlock FLASH_CR register */
	stm_flash.keyr = STM_FLASH_KEYR_KEY1;
	stm_flash.keyr = STM_FLASH_KEYR_KEY2;
	if (ao_flash_is_locked())
		ao_panic(AO_PANIC_FLASH);
}

static void
ao_flash_lock(void)
{
	stm_flash.cr |= (1 << STM_FLASH_CR_LOCK);
}

static uint32_t
stm_flash_page_size(void)
{
	uint16_t	dev_id = stm_dev_id();

	switch (dev_id) {
	case 0x440:	/* stm32f05x */
	case 0x444:	/* stm32f03x */
	case 0x445:	/* stm32f04x */
		return 1024;
	case 0x442:	/* stm32f09x */
	case 0x448:	/* stm32f07x */
		return 2048;
	}
	ao_panic(AO_PANIC_FLASH);
	return 0;
}

#define ao_flash_wait_bsy() do { while (stm_flash.sr & (1 << STM_FLASH_SR_BSY)); } while (0)

static void __attribute__ ((section(".sdata2.flash"), noinline))
_ao_flash_erase_page(uint16_t *page)
{
	stm_flash.cr |= (1 << STM_FLASH_CR_PER);

	stm_flash.ar = (uintptr_t) page;

	stm_flash.cr |= (1 << STM_FLASH_CR_STRT);

	ao_flash_wait_bsy();

	stm_flash.cr &= ~(1 << STM_FLASH_CR_PER);
}

#define _ao_flash_addr(pos)	((uint16_t *) (void *) ((uint8_t *) __flash__ + (pos)))

static void __attribute__ ((section(".sdata2.flash"), noinline))
_ao_flash_byte(uint32_t pos, uint8_t b)
{
	uint16_t	v;
	uint16_t	*a = _ao_flash_addr(pos & ~1);

	if (pos & 1)
		v = (*a & 0xff) | (b << 8);
	else
		v = (*a & 0xff00) | b;
	*a = v;
	ao_flash_wait_bsy();
}

static void __attribute__ ((section(".sdata2.flash"), noinline))
_ao_flash_write(uint32_t pos, void *sv, uint16_t len)
{
	uint8_t		*s = sv;
	uint16_t	*f16;
	uint16_t	v;

	stm_flash.cr |= (1 << STM_FLASH_CR_PG);

	if (pos & 1) {
		_ao_flash_byte(pos++, *s++);
		len--;
	}
	f16 = _ao_flash_addr(pos);
	while(len > 1) {
		v = s[0] | (s[1] << 8);
		s += 2;
		*f16++ = v;
		ao_flash_wait_bsy();
		len -= 2;
		pos += 2;
	}
	if (len)
		_ao_flash_byte(pos, *s++);

	stm_flash.cr &= ~(1 << STM_FLASH_CR_PG);
}

uint8_t
ao_storage_device_erase(uint32_t pos)
{
	ao_arch_block_interrupts();
	ao_flash_unlock();

	_ao_flash_erase_page(_ao_flash_addr(pos));

	ao_flash_lock();
	ao_arch_release_interrupts();
	return 1;
}

uint8_t
ao_storage_device_write(uint32_t pos, void *v, uint16_t len)
{
	if (len == 0)
		return 1;

	ao_arch_block_interrupts();
	ao_flash_unlock();

	_ao_flash_write(pos, v, len);

	ao_flash_lock();
	ao_arch_release_interrupts();
	return 1;
}

uint8_t
ao_storage_device_read(uint32_t pos, void *d, uint16_t len) 
{
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	memcpy(d, _ao_flash_addr(pos), len);
	return 1;
}

void
ao_storage_flush(void) 
{
}

void
ao_storage_setup(void)
{
	ao_storage_block = stm_flash_page_size();
	ao_storage_total = ((uint8_t *) __flash_end__) - ((uint8_t *) __flash__);
	ao_storage_unit = ao_storage_total;
}

void
ao_storage_device_info(void) 
{
	printf ("Using internal flash, page %ld bytes, total %ld bytes\n",
		(long) ao_storage_block, (long) ao_storage_total);
}

void
ao_storage_device_init(void)
{
}
