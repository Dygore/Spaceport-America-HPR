/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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
#include <ao_crc.h>

#ifndef AO_CRC_WIDTH
#error "Must define AO_CRC_WIDTH"
#endif

/* Only the STM32F07x and ST32F09x series have
 * programmable CRC units. Others can only do the ANSI CRC-32 computation
 */

#if !AO_HAVE_PROGRAMMABLE_CRC_UNIT && AO_CRC_WIDTH != 32
#error "Target hardware does not have programmable CRC unit"
#endif

#ifndef AO_CRC_POLY
#if AO_CRC_WIDTH == 16
#define AO_CRC_POLY	AO_CRC_16_DEFAULT
#endif
#if AO_CRC_WIDTH == 32
#define AO_CRC_POLY	AO_CRC_32_DEFAULT
#endif
#endif

#if !AO_HAVE_PROGRAMMABLE_CRC_UNIT && (AO_CRC_WIDTH != 32 || AO_CRC_POLY != AO_CRC_32_ANSI)
#error "Target hardware does not have programmable CRC unit"
#endif

#if AO_CRC_WIDTH == 32
#define AO_CRC_CR_POLYSIZE	STM_CRC_CR_POLYSIZE_32
#endif

#if AO_CRC_WIDTH == 16
#define AO_CRC_CR_POLYSIZE	STM_CRC_CR_POLYSIZE_16
#endif

#if AO_CRC_WIDTH == 8
#define AO_CRC_CR_POLYSIZE	STM_CRC_CR_POLYSIZE_8
#endif

#if AO_CRC_WIDTH == 7
#define AO_CRC_CR_POLYSIZE	STM_CRC_CR_POLYSIZE_7
#endif

#ifndef AO_CRC_INIT
#define AO_CRC_INIT	0xffffffff
#endif

void
ao_crc_reset(void)
{
	stm_crc.cr |= (1 << STM_CRC_CR_RESET);
	while ((stm_crc.cr & (1 << STM_CRC_CR_RESET)) != 0)
		;
}

void
ao_crc_init(void)
{
	/* Turn on the CRC clock */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_CRCEN);

	/* Need to initialize CR even on non-programmable hardware,
	 * the write to the POLYSIZE bits will be ignored in that
	 * case
	 */
	stm_crc.cr = (AO_CRC_CR_POLYSIZE << STM_CRC_CR_POLYSIZE);
	stm_crc.init = AO_CRC_INIT;
#if AO_HAVE_PROGRAMMABLE_CRC_UNIT
	stm_crc.pol = AO_CRC_POLY;
#endif
	ao_crc_reset();
}
