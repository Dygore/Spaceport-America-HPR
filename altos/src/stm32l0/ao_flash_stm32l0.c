/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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
#include <ao_flash.h>

static uint8_t
ao_flash_pecr_is_locked(void)
{
	return (stm_flash.pecr & (1 << STM_FLASH_PECR_PE_LOCK)) != 0;
}

static uint8_t
ao_flash_pgr_is_locked(void)
{
	return (stm_flash.pecr & (1 << STM_FLASH_PECR_PRG_LOCK)) != 0;
}

static void
ao_flash_pecr_unlock(void)
{
	if (!ao_flash_pecr_is_locked())
		return;

	/* Unlock Data EEPROM and FLASH_PECR register */
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY1;
	stm_flash.pekeyr = STM_FLASH_PEKEYR_PEKEY2;
	if (ao_flash_pecr_is_locked())
		ao_panic(AO_PANIC_FLASH);
}

static void
ao_flash_pgr_unlock(void)
{
	if (!ao_flash_pgr_is_locked())
		return;

	/* Unlock program memory */
	stm_flash.prgkeyr = STM_FLASH_PRGKEYR_PRGKEY1;
	stm_flash.prgkeyr = STM_FLASH_PRGKEYR_PRGKEY2;
	if (ao_flash_pgr_is_locked())
		ao_panic(AO_PANIC_FLASH);
}

static void
ao_flash_lock(void)
{
	stm_flash.pecr |= (1 << STM_FLASH_PECR_OPT_LOCK) | (1 << STM_FLASH_PECR_PRG_LOCK) | (1 << STM_FLASH_PECR_PE_LOCK);
}

static void
ao_flash_wait_bsy(void)
{
	while (stm_flash.sr & (1 << STM_FLASH_SR_BSY))
		;
}

static void __attribute__ ((section(".sdata2.flash"), noinline))
_ao_flash_erase_page(uint32_t *page)
{
	stm_flash.pecr |= (1 << STM_FLASH_PECR_ERASE) | (1 << STM_FLASH_PECR_PROG);

	*page = 0x00000000;

	ao_flash_wait_bsy();
}

void
ao_flash_erase_page(uint32_t *page)
{
	ao_arch_block_interrupts();
	ao_flash_pecr_unlock();
	ao_flash_pgr_unlock();

	_ao_flash_erase_page(page);

	ao_flash_lock();
	ao_arch_release_interrupts();
}

#if 0
static void __attribute__ ((section(".sdata2.flash"), noinline))
_ao_flash_half_page(uint32_t *dst, uint32_t *src)
{
	uint8_t		i;

	stm_flash.pecr |= (1 << STM_FLASH_PECR_FPRG);
	stm_flash.pecr |= (1 << STM_FLASH_PECR_PROG);

	ao_flash_wait_bsy();

	for (i = 0; i < 32; i++) {
		*dst++ = *src++;
	}

	while (stm_flash.sr & (1 << STM_FLASH_SR_BSY))
		;
}

void
ao_flash_page(uint32_t *page, uint32_t *src)
{
	uint8_t		h;

	ao_flash_erase_page(page);

	ao_arch_block_interrupts();
	ao_flash_pecr_unlock();
	ao_flash_pgr_unlock();
	for (h = 0; h < 2; h++) {
		_ao_flash_half_page(page, src);
		page += 32;
		src += 32;
	}
	ao_flash_lock();
	ao_arch_release_interrupts();
}
#endif

static ao_pos_t	write_pos;
static uint8_t write_pending;
static union {
	uint32_t	u32;
	uint8_t		u8[4];
} write_buf;

void
ao_storage_flush(void)
{
	if (write_pending) {
		ao_flash_pecr_unlock();
		ao_flash_pgr_unlock();
		__storage[write_pos] = write_buf.u32;
		ao_flash_lock();
		write_pending = 0;
	}
}

/* Read data within a storage unit */
uint8_t
ao_storage_device_read(ao_pos_t pos, void *buf, uint16_t len)
{
	memcpy(buf, ((uint8_t *) __storage) + pos, len);
	return 1;
}

static void
flash_write_select(ao_pos_t pos)
{
	/* Flush any pending writes to another word */
	if (write_pending) {
		if (pos == write_pos)
			return;
		__storage[write_pos] = write_buf.u32;
		write_pending = 0;
	}
	write_pos = pos;
}

/* Write data within a storage unit */
uint8_t
ao_storage_device_write(ao_pos_t pos, void *buf, uint16_t len)
{
	uint8_t	*b8 = buf;

	ao_flash_pecr_unlock();
	ao_flash_pgr_unlock();
	while (len) {
		ao_pos_t	this_pos = pos >> 2;

		flash_write_select(this_pos);

		/* Update write buffer with new contents */
		uint16_t this_word = 4 - (pos & 3);
		if (this_word > len)
			this_word = len;
		memcpy(&write_buf.u8[pos & 3], b8, this_word);
		pos += this_word;
		len -= this_word;
		b8 += this_word;

		/* If we filled the buffer, flush it out */
		if ((pos & 3) == 0) {
			__storage[write_pos] = write_buf.u32;
		} else {
			write_pending = 1;
		}
	}
	ao_flash_lock();
	return 1;
}

bool
ao_storage_device_is_erased(uint32_t pos)
{
	uint8_t *m = ((uint8_t *) __storage) + pos;
	uint32_t i;

	for (i = 0; i < STM_FLASH_PAGE_SIZE; i++)
		if (*m++ != AO_STORAGE_ERASED_BYTE)
			return false;
	return true;
}

/* Erase device from pos through pos + ao_storage_block */
uint8_t
ao_storage_device_erase(uint32_t pos)
{
	ao_flash_erase_page(__storage + (pos >> 2));
	return 1;
}
