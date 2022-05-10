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

#ifndef _AO_CRC_H_
#define _AO_CRC_H_

#define AO_CRC_16_CCITT		0x1021
#define AO_CRC_16_CDMA2000	0xc867
#define AO_CRC_16_DECT		0x0589
#define AO_CRC_16_T10_DIF	0x8bb7
#define AO_CRC_16_DNP		0x3d65
#define AO_CRC_16_ANSI		0x8005
#define AO_CRC_16_DEFAULT	AO_CRC_16_ANSI

#define AO_CRC_32_ANSI		0x04c11db7
#define AO_CRC_32_C		0x1edc6f41

#define AO_CRC_32_DEFAULT	AO_CRC_32_ANSI

static inline uint16_t
ao_crc_in_32_out_16(uint32_t v) {
	stm_crc.dr.u32 = v;
	v = stm_crc.dr.u32;
	return (uint16_t) (v ^ (v >> 16));
}

static inline uint16_t
ao_crc_in_16_out_16(uint16_t v) {
	stm_crc.dr.u16 = v;
	return stm_crc.dr.u16;
}

static inline uint16_t
ao_crc_in_8_out_16(uint8_t v) {
	stm_crc.dr.u8 = v;
	return stm_crc.dr.u16;
}

void
ao_crc_reset(void);

void
ao_crc_init(void);

#endif /* _AO_CRC_H_ */
