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
#include "ao_adxl375.h"

#define DEBUG_LOW	1
#define DEBUG_HIGH	2

#define DEBUG		0

#if DEBUG
#define PRINTD(l, ...) do { if (DEBUG & (l)) { printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } } while(0)
#else
#define PRINTD(l,...)
#endif

#define AO_ADXL375_SPI_SPEED	ao_spi_speed(5000000)

struct ao_adxl375_sample	ao_adxl375_current;

static void
ao_adxl375_start(void) {
	ao_spi_get_bit(AO_ADXL375_CS_PORT,
		       AO_ADXL375_CS_PIN,
		       AO_ADXL375_SPI_INDEX,
		       AO_ADXL375_SPI_SPEED);
}

static void
ao_adxl375_stop(void) {
	ao_spi_put_bit(AO_ADXL375_CS_PORT,
		       AO_ADXL375_CS_PIN,
		       AO_ADXL375_SPI_INDEX);
}

static uint8_t
ao_adxl375_reg_read(uint8_t addr)
{
	uint8_t	d[2];

	d[0] = addr | AO_ADXL375_READ;
	ao_adxl375_start();
	ao_spi_duplex(d, d, 2, AO_ADXL375_SPI_INDEX);
	ao_adxl375_stop();

	PRINTD(DEBUG_LOW, "read %x = %x\n", addr, d);

	return d[1];
}

static void
ao_adxl375_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2];

	PRINTD(DEBUG_LOW, "write %x %x\n", addr, value);
	d[0] = addr;
	d[1] = value;
	ao_adxl375_start();
	ao_spi_send(d, 2, AO_ADXL375_SPI_INDEX);
	ao_adxl375_stop();

#if DEBUG & DEBUG_LOW
	d[0] = addr | AO_ADXL375_READ
	d[1] = 0;
	ao_adxl375_start();
	ao_spi_duplex(d, d, 2, AO_ADXL375_SPI_INDEX);
	ao_adxl375_stop();
	PRINTD(DEBUG_LOW, "readback %x %x\n", d[0], d[1]);
#endif
}

static void
ao_adxl375_value(struct ao_adxl375_sample *value)
{
	uint8_t	d[7];

	d[0] = AO_ADXL375_DATAX0 | AO_ADXL375_READ | AO_ADXL375_MULTI_BYTE;
	ao_adxl375_start();
	ao_spi_duplex(d, d, 7, AO_ADXL375_SPI_INDEX);
	ao_adxl375_stop();
	memcpy(value, &d[1], 6);
}

struct ao_adxl375_total {
	int32_t	x;
	int32_t	y;
	int32_t	z;
};

#define AO_ADXL375_SELF_TEST_SAMPLES	10
#define AO_ADXL375_SELF_TEST_SETTLE	4

#define MIN_LSB_G	18.4
#define MAX_LSB_G	22.6
#define SELF_TEST_MIN_G	5.0
#define SELF_TEST_MAX_G	6.8

#define MIN_SELF_TEST	((int32_t) (MIN_LSB_G * SELF_TEST_MIN_G * AO_ADXL375_SELF_TEST_SAMPLES + 0.5))
#define MAX_SELF_TEST	((int32_t) (MAX_LSB_G * SELF_TEST_MAX_G * AO_ADXL375_SELF_TEST_SAMPLES + 0.5))

static void
ao_adxl375_total_value(struct ao_adxl375_total *total, int samples)
{
	struct ao_adxl375_sample	value;

	*total = (struct ao_adxl375_total) { 0, 0, 0 };
	for (int i = 0; i < samples; i++) {
		ao_adxl375_value(&value);
		total->x += value.x;
		total->y += value.y;
		total->z += value.z;
		ao_delay(AO_MS_TO_TICKS(10));
	}
}

#define AO_ADXL375_DATA_FORMAT_SETTINGS(self_test) (			\
		AO_ADXL375_DATA_FORMAT_FIXED |				\
		(self_test << AO_ADXL375_DATA_FORMAT_SELF_TEST) |	\
		(AO_ADXL375_DATA_FORMAT_SPI_4_WIRE << AO_ADXL375_DATA_FORMAT_SPI_4_WIRE) | \
		(0 << AO_ADXL375_DATA_FORMAT_INT_INVERT) |		\
		(0 << AO_ADXL375_DATA_FORMAT_JUSTIFY))

static int32_t	self_test_value;

static void
ao_adxl375_setup(void)
{
	/* Get the device into 4-wire SPI mode before proceeding */
	ao_adxl375_reg_write(AO_ADXL375_DATA_FORMAT,
			     AO_ADXL375_DATA_FORMAT_SETTINGS(0));


	uint8_t	devid = ao_adxl375_reg_read(AO_ADXL375_DEVID);
	if (devid != AO_ADXL375_DEVID_ID)
		AO_SENSOR_ERROR(AO_DATA_ADXL375);

	/* Set the data rate */
	ao_adxl375_reg_write(AO_ADXL375_BW_RATE,
			     (0 << AO_ADXL375_BW_RATE_LOW_POWER) |
			     (AO_ADXL375_BW_RATE_RATE_200 << AO_ADXL375_BW_RATE_RATE));

	/* Set the offsets all to zero */
	ao_adxl375_reg_write(AO_ADXL375_OFSX, 0);
	ao_adxl375_reg_write(AO_ADXL375_OFSY, 0);
	ao_adxl375_reg_write(AO_ADXL375_OFSZ, 0);

	/* Clear interrupts */
	ao_adxl375_reg_write(AO_ADXL375_INT_ENABLE, 0);

	/* Configure FIFO (disable) */
	ao_adxl375_reg_write(AO_ADXL375_FIFO_CTL,
			     (AO_ADXL375_FIFO_CTL_FIFO_MODE_BYPASS << AO_ADXL375_FIFO_CTL_FIFO_MODE) |
			     (0 << AO_ADXL375_FIFO_CTL_TRIGGER) |
			     (0 << AO_ADXL375_FIFO_CTL_SAMPLES));

	/* Place part in measurement mode */
	ao_adxl375_reg_write(AO_ADXL375_POWER_CTL,
			     (0 << AO_ADXL375_POWER_CTL_LINK) |
			     (0 << AO_ADXL375_POWER_CTL_AUTO_SLEEP) |
			     (1 << AO_ADXL375_POWER_CTL_MEASURE) |
			     (0 << AO_ADXL375_POWER_CTL_SLEEP) |
			     (AO_ADXL375_POWER_CTL_WAKEUP_8 << AO_ADXL375_POWER_CTL_WAKEUP));

	/* Perform self-test */

	struct ao_adxl375_total	self_test_off, self_test_on;

	/* Discard some samples to let it settle down */
	ao_adxl375_total_value(&self_test_off, AO_ADXL375_SELF_TEST_SETTLE);

	/* Get regular values */
	ao_adxl375_total_value(&self_test_off, AO_ADXL375_SELF_TEST_SAMPLES);

	/* Turn on self test */
	ao_adxl375_reg_write(AO_ADXL375_DATA_FORMAT,
			     AO_ADXL375_DATA_FORMAT_SETTINGS(1));

	/* Discard at least 4 samples to let the device settle */
	ao_adxl375_total_value(&self_test_on, AO_ADXL375_SELF_TEST_SETTLE);

	/* Read self test samples */
	ao_adxl375_total_value(&self_test_on, AO_ADXL375_SELF_TEST_SAMPLES);

	/* Reset back to normal mode */

	ao_adxl375_reg_write(AO_ADXL375_DATA_FORMAT,
			     AO_ADXL375_DATA_FORMAT_SETTINGS(0));

	/* Verify Z axis value is in range */

	int32_t	z_change = self_test_on.z - self_test_off.z;

	self_test_value = z_change;

	if (z_change < MIN_SELF_TEST)
		AO_SENSOR_ERROR(AO_DATA_ADXL375);

	/* This check is commented out as maximum self test is unreliable

	   if (z_change > MAX_SELF_TEST)
		AO_SENSOR_ERROR(AO_DATA_ADXL375);

	*/

	/* Discard some samples to let it settle down */
	ao_adxl375_total_value(&self_test_off, AO_ADXL375_SELF_TEST_SETTLE);
}

static void
ao_adxl375(void)
{
	ao_adxl375_setup();
	for (;;) {
		ao_adxl375_value(&ao_adxl375_current);
		ao_arch_critical(
			AO_DATA_PRESENT(AO_DATA_ADXL375);
			AO_DATA_WAIT();
			);
	}
}

static struct ao_task ao_adxl375_task;

static void
ao_adxl375_dump(void)
{
	printf ("ADXL375 value %d %d %d self test %ld min %ld max %ld\n",
		ao_adxl375_current.x,
		ao_adxl375_current.y,
		ao_adxl375_current.z,
		(long) self_test_value,
		(long) MIN_SELF_TEST,
		(long) MAX_SELF_TEST);
}

const struct ao_cmds ao_adxl375_cmds[] = {
	{ ao_adxl375_dump,	"A\0Display ADXL375 data" },
	{ 0, NULL },
};

void
ao_adxl375_init(void)
{
	ao_cmd_register(ao_adxl375_cmds);
	ao_spi_init_cs(AO_ADXL375_CS_PORT, (1 << AO_ADXL375_CS_PIN));

	ao_add_task(&ao_adxl375_task, ao_adxl375, "adxl375");
}
