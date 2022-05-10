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
#include <ao_micropeak.h>
#include <ao_log_micro.h>
#ifndef LOG_MICRO_ASYNC
#define LOG_MICRO_ASYNC 1
#endif
#if LOG_MICRO_ASYNC
#include <ao_async.h>
#else
#include <ao_serial.h>
#endif
#include <ao_storage.h>

static N_SAMPLES_TYPE ao_log_offset = STARTING_LOG_OFFSET;


void
ao_log_micro_save(void)
{
	N_SAMPLES_TYPE	n_samples = (ao_log_offset - STARTING_LOG_OFFSET) / sizeof (uint16_t);

	ao_eeprom_write(PA_GROUND_OFFSET, &pa_ground, sizeof (pa_ground));
	ao_eeprom_write(PA_MIN_OFFSET, &pa_min, sizeof (pa_min));
	ao_eeprom_write(N_SAMPLES_OFFSET, &n_samples, sizeof (n_samples));
}

void
ao_log_micro_restore(void)
{
	ao_eeprom_read(PA_GROUND_OFFSET, &pa_ground, sizeof (pa_ground));
	ao_eeprom_read(PA_MIN_OFFSET, &pa_min, sizeof (pa_min));
}

void
ao_log_micro_data(void)
{
	uint16_t	low_bits = (uint16_t) pa;

	if (ao_log_offset < MAX_LOG_OFFSET) {
		ao_eeprom_write(ao_log_offset, &low_bits, sizeof (low_bits));
		ao_log_offset += sizeof (low_bits);
	}
}

#define POLY 0x8408

static uint16_t
ao_log_micro_crc(uint16_t crc, uint8_t byte)
{
	uint8_t	i;

	for (i = 0; i < 8; i++) {
		if ((crc & 0x0001) ^ (byte & 0x0001))
			crc = (crc >> 1) ^ POLY;
		else
			crc = crc >> 1;
		byte >>= 1;
	}
	return crc;
}

static void
ao_log_hex_nibble(uint8_t b)
{
	if (b < 10)
		ao_async_byte((uint8_t) ('0' + b));
	else
		ao_async_byte((uint8_t) ('a' - 10 + b));
}

void
ao_log_hex(uint8_t b)
{
	ao_log_hex_nibble(b>>4);
	ao_log_hex_nibble(b&0xf);
}

void
ao_log_newline(void)
{
	ao_async_byte('\r');
	ao_async_byte('\n');
}

#define MAX_N_SAMPLES	((MAX_LOG_OFFSET - STARTING_LOG_OFFSET) / 2)

void
ao_log_micro_dump(void)
{
	N_SAMPLES_TYPE	n_samples;
	uint16_t	nbytes;
	uint8_t		byte;
	uint16_t	b;
	uint16_t	crc = 0xffff;

	ao_eeprom_read(N_SAMPLES_OFFSET, &n_samples, sizeof (n_samples));

	if (n_samples == (N_SAMPLES_TYPE) (~0))
		n_samples = 0;
	nbytes = (uint16_t) (STARTING_LOG_OFFSET + sizeof (uint16_t) * n_samples);

	/*
	 * Rewrite n_samples so that it includes the log ID value with
	 * 32-bit n_samples split into two chunks
	 */
	if (sizeof (n_samples) > 2) {
		N_SAMPLES_TYPE	n_samples_low;
		N_SAMPLES_TYPE	n_samples_high;
		n_samples_low = n_samples & ((1 << AO_LOG_ID_SHIFT) - 1);
		n_samples_high = (n_samples - n_samples_low) << AO_LOG_ID_WIDTH;
		n_samples = n_samples_low | n_samples_high;
	}
#if AO_LOG_ID
	n_samples |= AO_LOG_ID << AO_LOG_ID_SHIFT;
#endif
	ao_async_start();
	ao_async_byte('M');
	ao_async_byte('P');
	for (b = 0; b < nbytes; b++) {
		if ((b & 0xf) == 0)
			ao_log_newline();
		ao_eeprom_read(b, &byte, 1);
#if AO_LOG_ID
		if (N_SAMPLES_OFFSET <= b && b < (N_SAMPLES_OFFSET + sizeof(n_samples))) {
			byte = (uint8_t) (n_samples >> ((b - N_SAMPLES_OFFSET) << 3));
		}
#endif
		ao_log_hex(byte);
		crc = ao_log_micro_crc(crc, byte);
	}
	ao_log_newline();
	crc = ~crc;
	ao_log_hex((uint8_t) (crc >> 8));
	ao_log_hex((uint8_t) crc);
	ao_log_newline();
	ao_async_stop();
}
