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
#include <ao_ms5607.h>
#include <ao_async.h>
#include <ao_log_micro.h>

static struct ao_ms5607_value	value;

alt_t		ground_alt, max_alt;
alt_t		ao_max_height;

void
ao_pa_get(void)
{
	ao_ms5607_sample(&ao_ms5607_current);
	ao_ms5607_convert(&ao_ms5607_current, &value);
	pa = value.pres;
}
static void
ao_pips(void)
{
	uint8_t	i;
	for (i = 0; i < 10; i++) {
		ao_led_toggle(AO_LED_REPORT);
		ao_delay(AO_MS_TO_TICKS(80));
	}
	ao_delay(AO_MS_TO_TICKS(200));
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

struct header {
	uint32_t	pa_ground_offset;
	uint32_t	pa_min_offset;
	N_SAMPLES_TYPE	nsamples;
};

static const struct header head = {
	.pa_ground_offset = 0xfffefdfc,
	.pa_min_offset = 0xfbfaf9f8,
	.nsamples = 64
};

static void
ao_test_micro_dump(void)
{
	N_SAMPLES_TYPE	n_samples;
	uint16_t	nbytes;
	uint8_t		byte;
	uint16_t	b;
	uint16_t	crc = 0xffff;

	n_samples = head.nsamples;
	nbytes = STARTING_LOG_OFFSET + sizeof (uint16_t) * n_samples;

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
		if (b < sizeof (head))
			byte = ((uint8_t *) &head)[b];
		else
			byte = b;
#if AO_LOG_ID
		if (N_SAMPLES_OFFSET <= b && b < (N_SAMPLES_OFFSET + sizeof(n_samples))) {
			byte = n_samples >> ((b - N_SAMPLES_OFFSET) << 3);
		}
#endif
		ao_log_hex(byte);
		crc = ao_log_micro_crc(crc, byte);
	}
	ao_log_newline();
	crc = ~crc;
	ao_log_hex(crc >> 8);
	ao_log_hex(crc);
	ao_log_newline();
	ao_async_stop();
}

int
main(void)
{
	ao_led_init();
	ao_timer_init();

	/* Init external hardware */
	ao_spi_init();
	ao_ms5607_init();
	ao_ms5607_setup();

	for (;;)
	{
		ao_delay(AO_MS_TO_TICKS(1000));

		ao_pips();
		ao_test_micro_dump();
	}
}
