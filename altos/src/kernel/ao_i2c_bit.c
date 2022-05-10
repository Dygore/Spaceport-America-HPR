/*
 * Copyright © 2021 Keith Packard <keithp@keithp.com>
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
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao_i2c_bit.h>
#include <ao_exti.h>

#define ao_i2c_bit_set_pin(port, pin) do {				\
		ao_enable_output(port, pin, 1);				\
		ao_gpio_set_output_mode(port, pin, AO_OUTPUT_OPEN_DRAIN); \
		ao_gpio_set_mode(port, pin, AO_EXTI_MODE_PULL_UP);	\
		stm_ospeedr_set(port, pin, STM_OSPEEDR_10MHz);		\
	} while(0)


/* 2µS per half-cycle, for a 125kHz i2c clock */
#define AO_I2C_TICK	10000

static void
tick(void)
{
	int i;

	for (i = 0; i < 12; i++)
		ao_arch_nop();
#if 0
	uint64_t	target = ao_time_ns() + AO_I2C_TICK;

	do {
		ao_yield();
	} while ((int64_t) (ao_time_ns() - target) < 0);
#endif
}

static void sda(uint8_t v)
{
	ao_gpio_set(AO_I2C_SDA_PORT, AO_I2C_SDA_PIN, v);
	tick();
}

static uint8_t in(void)
{
	uint8_t v = ao_gpio_get(AO_I2C_SDA_PORT, AO_I2C_SDA_PIN);
	tick();
	return v;
}

static void scl(uint8_t v)
{
	ao_gpio_set(AO_I2C_SCL_PORT, AO_I2C_SCL_PIN, v);
	tick();
}

static void
i2c_start(void)
{
	sda(0);
	scl(0);
}

static bool
ack(void)
{
	bool v;
	int j;

	sda(1);
	scl(1);
	v = false;
	for (j = 0; j < 100; j++) {
		if (in() == 0) {
			v = true;
			break;
		}
	}
	scl(0);
	sda(1);
	ao_yield();
	return v;
}

static bool
send(uint8_t byte)
{
	uint8_t bit;

	for (bit = 0; bit < 8; bit++) {
		sda(byte>>7);
		scl(1);
		tick();
		scl(0);
		byte <<= 1;
	}
	return ack();
}

static uint8_t
recv(void)
{
	uint8_t byte = 0;
	uint8_t bit;

	for (bit = 0; bit < 8; bit++) {
		sda(1);
		scl(1);
		byte = (byte << 1) | in();
		scl(0);
	}
	ack();
	return byte;
}

static void
stop(void)
{
	sda(0);
	scl(1);
	sda(1);
}

static void
restart(void)
{
	sda(1);
	scl(1);
	sda(0);
	scl(0);
}

static uint8_t
ao_i2c_bit_mutex;

void
ao_i2c_bit_get(void)
{
	ao_mutex_get(&ao_i2c_bit_mutex);
}

void
ao_i2c_bit_put(void)
{
	ao_mutex_put(&ao_i2c_bit_mutex);
}

bool
ao_i2c_bit_start(uint8_t addr)
{
	i2c_start();
	return send(addr);
}

bool
ao_i2c_bit_restart(uint8_t addr)
{
	restart();
	return send(addr);
}

void
ao_i2c_bit_stop(void)
{
	stop();
}

void
ao_i2c_bit_send(void *block, uint16_t len)
{
	uint8_t *b = block;
	while (len--)
		send(*b++);
}

void
ao_i2c_bit_recv(void *block, uint16_t len)
{
	uint8_t *b = block;

	while (len--)
		*b++ = recv();
}

void
ao_i2c_bit_init(void)
{
	ao_i2c_bit_set_pin(AO_I2C_SCL_PORT, AO_I2C_SCL_PIN);
	ao_i2c_bit_set_pin(AO_I2C_SDA_PORT, AO_I2C_SDA_PIN);
}
