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
#include <ao_as1107.h>

static uint8_t	as1107_configured;
static uint8_t	as1107_mutex;

static void
ao_as1107_start(void) {
	ao_spi_get_bit(AO_AS1107_CS_PORT, AO_AS1107_CS_PIN, AO_AS1107_CS, AO_AS1107_SPI_INDEX, AO_AS1107_SPI_SPEED);
}

static void
ao_as1107_stop(void) {
	ao_spi_put_bit(AO_AS1107_CS_PORT, AO_AS1107_CS_PIN, AO_AS1107_CS, AO_AS1107_SPI_INDEX);
}

static void
_ao_as1107_cmd(uint8_t addr, uint8_t value)
{
	uint8_t	packet[2] = { addr, value };

	ao_as1107_start();
	ao_spi_send(packet, 2, AO_AS1107_SPI_INDEX);
	ao_as1107_stop();
}

static void
_ao_as1107_setup(void)
{
	if (!as1107_configured) {
		as1107_configured = 1;
		_ao_as1107_cmd(AO_AS1107_SHUTDOWN, AO_AS1107_SHUTDOWN_SHUTDOWN_RESET);
		_ao_as1107_cmd(AO_AS1107_SHUTDOWN, AO_AS1107_SHUTDOWN_SHUTDOWN_NOP);
		_ao_as1107_cmd(AO_AS1107_DECODE_MODE, AO_AS1107_DECODE);
		_ao_as1107_cmd(AO_AS1107_SCAN_LIMIT, AO_AS1107_NUM_DIGITS - 1);
		_ao_as1107_cmd(AO_AS1107_INTENSITY, 0x0f);
		_ao_as1107_cmd(AO_AS1107_FEATURE,
			       (0 << AO_AS1107_FEATURE_CLK_EN) |
			       (0 << AO_AS1107_FEATURE_REG_RES) |
			       (1 << AO_AS1107_FEATURE_DECODE_SEL) |
			       (1 << AO_AS1107_FEATURE_SPI_EN) |
			       (0 << AO_AS1107_FEATURE_BLINK_EN) |
			       (0 << AO_AS1107_FEATURE_BLINK_FREQ) |
			       (0 << AO_AS1107_FEATURE_SYNC) |
			       (0 << AO_AS1107_FEATURE_BLINK_START));
		_ao_as1107_cmd(AO_AS1107_SHUTDOWN, AO_AS1107_SHUTDOWN_NORMAL_NOP);
	}
}

void
ao_as1107_write(uint8_t start, uint8_t count, uint8_t *values)
{
	uint8_t i;
	ao_mutex_get(&as1107_mutex);
	_ao_as1107_setup();
	for (i = 0; i < count; i++)
	{
		_ao_as1107_cmd(AO_AS1107_DIGIT(start + i),
			       values[i]);
	}
	ao_mutex_put(&as1107_mutex);
}

void
ao_as1107_write_8(uint8_t start, uint8_t value)
{
	uint8_t	values[2];

	values[0] = (value >> 4);
	values[1] = value & 0xf;
	ao_as1107_write(start, 2, values);
}

void
ao_as1107_write_16(uint8_t start, uint16_t value)
{
	uint8_t	values[4];

	values[0] = (value >> 12);
	values[1] = (value >> 8) & 0xf;
	values[2] = (value >> 4) & 0xf;
	values[3] = (value) & 0xf;
	ao_as1107_write(start, 4, values);
}

void
ao_as1107_init(void)
{
	as1107_configured = 0;
	ao_spi_init_cs(AO_AS1107_CS_PORT, (1 << AO_AS1107_CS_PIN));
}
