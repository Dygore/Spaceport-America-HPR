/*
 * Copyright © 2021 Keith Packard <keithp@keithp.com>
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
#include <ao_mmc5983.h>
#include <ao_exti.h>

#if HAS_MMC5983

#define DEBUG_MMC5983	0

struct ao_mmc5983_sample	ao_mmc5983_current;
static struct ao_mmc5983_sample	ao_mmc5983_offset;

static uint8_t	ao_mmc5983_configured;

#ifdef MMC5983_I2C
#include <ao_i2c_bit.h>

static void
ao_mmc5983_reg_write(uint8_t addr, uint8_t data)
{
	uint8_t d[2];

	d[0] = addr;
	d[1] = data;

	ao_i2c_bit_start(MMC5983_I2C_ADDR);
	ao_i2c_bit_send(d, 2);
	ao_i2c_bit_stop();
}

static uint8_t
ao_mmc5983_reg_read(uint8_t addr)
{
	uint8_t d[1];

	ao_i2c_bit_start(MMC5983_I2C_ADDR);
	d[0] = addr;
	ao_i2c_bit_send(d, 1);
	ao_i2c_bit_restart(MMC5983_I2C_ADDR | 1);
	ao_i2c_bit_recv(d, 1);
	ao_i2c_bit_stop();
	return d[0];
}

static void
ao_mmc5983_raw(struct ao_mmc5983_raw *raw)
{
	ao_i2c_bit_start(MMC5983_I2C_ADDR);
	raw->addr = MMC5983_X_OUT_0;
	ao_i2c_bit_send(&(raw->addr), 1);
	ao_i2c_bit_restart(MMC5983_I2C_ADDR | 1);
	ao_i2c_bit_recv(&(raw->x0), sizeof(*raw) - 1);
	ao_i2c_bit_stop();
}

#else
#define AO_MMC5983_SPI_SPEED	ao_spi_speed(2000000)

static void
ao_mmc5983_start(void) {
	ao_spi_get_bit(AO_MMC5983_SPI_CS_PORT,
		       AO_MMC5983_SPI_CS_PIN,
		       AO_MMC5983_SPI_INDEX,
		       AO_MMC5983_SPI_SPEED);
}

static void
ao_mmc5983_stop(void) {
	ao_spi_put_bit(AO_MMC5983_SPI_CS_PORT,
		       AO_MMC5983_SPI_CS_PIN,
		       AO_MMC5983_SPI_INDEX);
}


static void
ao_mmc5983_reg_write(uint8_t addr, uint8_t data)
{
	uint8_t	d[2];

	d[0] = addr;
	d[1] = data;
	ao_mmc5983_start();
	ao_spi_send(d, 2, AO_MMC5983_SPI_INDEX);
	ao_mmc5983_stop();
}

static uint8_t
ao_mmc5983_reg_read(uint8_t addr)
{
	uint8_t	d[2];

	d[0] = addr | MMC5983_READ;
	ao_mmc5983_start();
	ao_spi_duplex(d, d, 2, AO_MMC5983_SPI_INDEX);
	ao_mmc5983_stop();

	return d[1];
}

static void
ao_mmc5983_duplex(uint8_t *dst, uint8_t len)
{
	ao_mmc5983_start();
	ao_spi_duplex(dst, dst, len, AO_MMC5983_SPI_INDEX);
	ao_mmc5983_stop();
}

static void
ao_mmc5983_raw(struct ao_mmc5983_raw *raw)
{
	raw->addr = MMC5983_X_OUT_0 | MMC5983_READ;
	ao_mmc5983_duplex((uint8_t *) raw, sizeof (*raw));
}
#endif

/* Saturating subtraction. Keep the result within range
 * of an int16_t
 */
static int16_t
sat_sub(int16_t a, int16_t b)
{
	int32_t v = (int32_t) a - (int32_t) b;
	if (v < -32768)
		v = -32768;
	if (v > 32767)
		v = 32767;
	return (int16_t) v;
}

/* Wait for a synchronous sample to finish */
static void
ao_mmc5983_wait(void)
{
	for (;;) {
		uint8_t status = ao_mmc5983_reg_read(MMC5983_STATUS);
		if ((status & (1 << MMC5983_STATUS_MEAS_M_DONE)) != 0)
			break;
		ao_delay(0);
	}
}

static void
ao_mmc5983_set(void)
{
	ao_mmc5983_reg_write(MMC5983_CONTROL_0,
			     (1 << MMC5983_CONTROL_0_SET));
}

static void
ao_mmc5983_reset(void)
{
	ao_mmc5983_reg_write(MMC5983_CONTROL_0,
			     (1 << MMC5983_CONTROL_0_RESET));
}

static struct ao_mmc5983_raw raw;

/* Read the sensor values and convert to a sample struct */
static void
ao_mmc5983_sample(struct ao_mmc5983_sample *s)
{
	ao_mmc5983_raw(&raw);

	/* Bias by 32768 to convert from uint16_t to int16_t */
	s->x = (int16_t) ((((uint16_t) raw.x0 << 8) | raw.x1) - 32768);
	s->y = (int16_t) ((((uint16_t) raw.y0 << 8) | raw.y1) - 32768);
	s->z = (int16_t) ((((uint16_t) raw.z0 << 8) | raw.z1) - 32768);
}

/* Synchronously sample the sensors */
static void
ao_mmc5983_sync_sample(struct ao_mmc5983_sample *v)
{
	ao_mmc5983_reg_write(MMC5983_CONTROL_0,
			     (1 << MMC5983_CONTROL_0_TM_M));
	ao_mmc5983_wait();
	ao_mmc5983_sample(v);
}

static struct ao_mmc5983_sample	set, reset;

/* Calibrate the device by finding the zero point */
static void
ao_mmc5983_cal(void)
{
	/* Compute offset */

	ao_delay(AO_MS_TO_TICKS(100));

	/* Measure in 'SET' mode */
	ao_mmc5983_set();
	ao_delay(AO_MS_TO_TICKS(100));
	ao_mmc5983_sync_sample(&set);

	ao_delay(AO_MS_TO_TICKS(100));

	/* Measure in 'RESET' mode */
	ao_mmc5983_reset();
	ao_delay(AO_MS_TO_TICKS(100));
	ao_mmc5983_sync_sample(&reset);

	/* The zero point is the average of SET and RESET values */
	ao_mmc5983_offset.x = (int16_t) (((int32_t) set.x + (int32_t) reset.x) / 2);
	ao_mmc5983_offset.y = (int16_t) (((int32_t) set.y + (int32_t) reset.y) / 2);
	ao_mmc5983_offset.z = (int16_t) (((int32_t) set.z + (int32_t) reset.z) / 2);
}

/* Configure the device to automatically sample at 200Hz */
static void
ao_mmc5983_run(void)
{
	/* Set bandwidth to 200Hz */
	ao_mmc5983_reg_write(MMC5983_CONTROL_1,
			     MMC5983_CONTROL_1_BW_200 << MMC5983_CONTROL_1_BW);

	/* Measure at 200Hz so we get recent samples by just reading
	 * the registers
	 */
	ao_mmc5983_reg_write(MMC5983_CONTROL_2,
			     (1 << MMC5983_CONTROL_2_CMM_EN) |
			     (MMC5983_CONTROL_2_CM_FREQ_200HZ << MMC5983_CONTROL_2_CM_FREQ) |
			     (0 << MMC5983_CONTROL_2_EN_PRD_SET) |
			     (MMC5983_CONTROL_2_PRD_SET_1000));
	ao_mmc5983_configured = 1;
}

/* Reboot the device by setting the SW_RST bit and waiting 10ms */
static void
ao_mmc5983_reboot(void)
{
	ao_mmc5983_configured = 0;

	ao_mmc5983_reg_write(MMC5983_CONTROL_1,
			     1 << MMC5983_CONTROL_1_SW_RST);

	/* Delay for power up time (10ms) */
	ao_delay(AO_MS_TO_TICKS(10));
}

/* Configure the device for operation */
static uint8_t
ao_mmc5983_setup(void)
{
	uint8_t product_id;

	/* Reboot the device */
	ao_mmc5983_reboot();

	/* Check product ID */
	product_id = ao_mmc5983_reg_read(MMC5983_PRODUCT_ID);
	if (product_id != MMC5983_PRODUCT_ID_PRODUCT_I2C &&
	    product_id != MMC5983_PRODUCT_ID_PRODUCT_SPI)
	{
		AO_SENSOR_ERROR(AO_DATA_MMC5983);
	}

	/* Calibrate */
	ao_mmc5983_cal();

	/* Start automatic sampling */
	ao_mmc5983_run();

	return 1;
}

struct ao_mmc5983_sample ao_mmc5983_current;

static void
ao_mmc5983(void)
{
	struct ao_mmc5983_sample	sample;
	ao_mmc5983_setup();
	for (;;) {
		if (ao_mmc5983_configured)
			ao_mmc5983_sample(&sample);
		sample.x = sat_sub(sample.x, ao_mmc5983_offset.x);
		sample.y = sat_sub(sample.y, ao_mmc5983_offset.y);
		sample.z = sat_sub(sample.z, ao_mmc5983_offset.z);
		ao_arch_block_interrupts();
		ao_mmc5983_current = sample;
		AO_DATA_PRESENT(AO_DATA_MMC5983);
		AO_DATA_WAIT();
		ao_arch_release_interrupts();
	}
}

static struct ao_task ao_mmc5983_task;

static void
ao_mmc5983_show(void)
{
#if DEBUG_MMC5983
	printf ("x0 %02x x1 %02x y0 %02x y1 %02x z0 %02x z1 %02x\n",
		raw.x0, raw.x1, raw.y0, raw.y1, raw.z0, raw.z1);

	printf ("set.x %d set.y %d set.z %d\n",
		set.x, set.y, set.z);

	printf ("reset.x %d reset.y %d reset.z %d\n",
		reset.x, reset.y, reset.z);

	printf ("offset.x %d offset.y %d offset.z %d\n",
		ao_mmc5983_offset.x,
		ao_mmc5983_offset.y,
		ao_mmc5983_offset.z);
#endif
	printf ("MMC5983: %d %d %d\n",
		ao_mmc5983_along(&ao_mmc5983_current),
		ao_mmc5983_across(&ao_mmc5983_current),
		ao_mmc5983_through(&ao_mmc5983_current));
}

#if DEBUG_MMC5983
static void
ao_mmc5983_recal(void)
{
	printf("recal\n"); fflush(stdout);
	ao_mmc5983_reboot();
	printf("reboot\n"); fflush(stdout);
	ao_mmc5983_cal();
	printf("cal\n"); fflush(stdout);
	ao_mmc5983_show();
	printf("show\n"); fflush(stdout);
	ao_mmc5983_run();
	printf("run\n"); fflush(stdout);
}
#endif

static const struct ao_cmds ao_mmc5983_cmds[] = {
	{ ao_mmc5983_show,	"M\0Show MMC5983 status" },
#if DEBUG_MMC5983
	{ ao_mmc5983_recal,	"m\0Recalibrate MMC5983" },
#endif
	{ 0, NULL }
};

void
ao_mmc5983_init(void)
{
	ao_mmc5983_configured = 0;

#ifdef MMC5983_I2C
	ao_enable_output(AO_MMC5983_SPI_CS_PORT, AO_MMC5983_SPI_CS_PIN, 1);
#else
	ao_enable_input(AO_MMC5983_SPI_MISO_PORT,
			AO_MMC5983_SPI_MISO_PIN,
			AO_EXTI_MODE_PULL_NONE);

	ao_enable_output(AO_MMC5983_SPI_CLK_PORT,
			 AO_MMC5983_SPI_CLK_PIN,
			 1);

	ao_enable_output(AO_MMC5983_SPI_MOSI_PORT,
			 AO_MMC5983_SPI_MOSI_PIN,
			 0);

	ao_spi_init_cs(AO_MMC5983_SPI_CS_PORT, (1 << AO_MMC5983_SPI_CS_PIN));
#endif

	ao_add_task(&ao_mmc5983_task, ao_mmc5983, "mmc5983");
	ao_cmd_register(&ao_mmc5983_cmds[0]);
}

#endif
