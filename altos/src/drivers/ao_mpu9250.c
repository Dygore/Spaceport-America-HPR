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
#include <ao_mpu9250.h>
#include <ao_exti.h>

#if HAS_MPU9250

#define MPU9250_TEST	0

static uint8_t	ao_mpu9250_configured;

#ifndef AO_MPU9250_I2C_INDEX
#define AO_MPU9250_SPI	1
#else
#define AO_MPU9250_SPI	0
#endif

#if AO_MPU9250_SPI

#ifndef AO_MPU9250_SPI_SPEED
#define AO_MPU9250_SPI_SPEED	ao_spi_speed(1000000)	/* 1MHz max SCLK */
#endif

#define ao_mpu9250_spi_get()	ao_spi_get(AO_MPU9250_SPI_BUS, AO_MPU9250_SPI_SPEED)
#define ao_mpu9250_spi_put()	ao_spi_put(AO_MPU9250_SPI_BUS)

#define ao_mpu9250_spi_start() 	ao_spi_set_cs(AO_MPU9250_SPI_CS_PORT,	\
					      (1 << AO_MPU9250_SPI_CS_PIN))

#define ao_mpu9250_spi_end() 	ao_spi_clr_cs(AO_MPU9250_SPI_CS_PORT,	\
					      (1 << AO_MPU9250_SPI_CS_PIN))

#else

#define ao_mpu9250_spi_get()
#define ao_mpu9250_spi_put()

#endif

static void
_ao_mpu9250_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2] = { addr, value };
#if AO_MPU9250_SPI
	ao_mpu9250_spi_start();
	ao_spi_send(d, 2, AO_MPU9250_SPI_BUS);
	ao_mpu9250_spi_end();
#else
	ao_i2c_get(AO_MPU9250_I2C_INDEX);
	ao_i2c_start(AO_MPU9250_I2C_INDEX, MPU9250_ADDR_WRITE);
	ao_i2c_send(d, 2, AO_MPU9250_I2C_INDEX, true);
	ao_i2c_put(AO_MPU9250_I2C_INDEX);
#endif
}

static void
_ao_mpu9250_read(uint8_t addr, void *data, uint8_t len)
{
#if AO_MPU9250_SPI
	addr |= 0x80;
	ao_mpu9250_spi_start();
	ao_spi_send(&addr, 1, AO_MPU9250_SPI_BUS);
	ao_spi_recv(data, len, AO_MPU9250_SPI_BUS);
	ao_mpu9250_spi_end();
#else
	ao_i2c_get(AO_MPU9250_I2C_INDEX);
	ao_i2c_start(AO_MPU9250_I2C_INDEX, MPU9250_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU9250_I2C_INDEX, false);
	ao_i2c_start(AO_MPU9250_I2C_INDEX, MPU9250_ADDR_READ);
	ao_i2c_recv(data, len, AO_MPU9250_I2C_INDEX, true);
	ao_i2c_put(AO_MPU9250_I2C_INDEX);
#endif
}

static uint8_t
_ao_mpu9250_reg_read(uint8_t addr)
{
	uint8_t	value;
#if AO_MPU9250_SPI
	addr |= 0x80;
	ao_mpu9250_spi_start();
	ao_spi_send(&addr, 1, AO_MPU9250_SPI_BUS);
	ao_spi_recv(&value, 1, AO_MPU9250_SPI_BUS);
	ao_mpu9250_spi_end();
#else
	ao_i2c_get(AO_MPU9250_I2C_INDEX);
	ao_i2c_start(AO_MPU9250_I2C_INDEX, MPU9250_ADDR_WRITE);
	ao_i2c_send(&addr, 1, AO_MPU9250_I2C_INDEX, false);
	ao_i2c_start(AO_MPU9250_I2C_INDEX, MPU9250_ADDR_READ);
	ao_i2c_recv(&value, 1, AO_MPU9250_I2C_INDEX, true);
	ao_i2c_put(AO_MPU9250_I2C_INDEX);
#endif
	return value;
}

static void
_ao_mpu9250_slv4_setup(uint8_t addr, uint8_t reg)
{
	/* Set i2c slave address */
	_ao_mpu9250_reg_write(MPU9250_I2C_SLV4_ADDR,
			      addr);

	/* Set i2c register address */
	_ao_mpu9250_reg_write(MPU9250_I2C_SLV4_REG,
			      reg);
}

static void
_ao_mpu9250_slv4_run(void)
{
	uint8_t	ctrl;

	/* Start the transfer */
	_ao_mpu9250_reg_write(MPU9250_I2C_SLV4_CTRL,
			      (1 << MPU9250_I2C_SLV4_CTRL_I2C_SLV4_EN) |
			      (0 << MPU9250_I2C_SLV4_CTRL_SLV4_DONE_INT_EN) |
			      (0 << MPU9250_I2C_SLV4_CTRL_I2C_SLV4_REG_DIS) |
			      (0 << MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY));

	/* Poll for completion */
	for (;;) {
		ctrl = _ao_mpu9250_reg_read(MPU9250_I2C_SLV4_CTRL);
		if ((ctrl & (1 << MPU9250_I2C_SLV4_CTRL_I2C_SLV4_EN)) == 0)
			break;
		ao_delay(0);
	}
}

static uint8_t
_ao_mpu9250_mag_reg_read(uint8_t reg)
{
	_ao_mpu9250_slv4_setup((1 << 7) | MPU9250_MAG_ADDR, reg);

	_ao_mpu9250_slv4_run();

	return _ao_mpu9250_reg_read(MPU9250_I2C_SLV4_DI);
}

static void
_ao_mpu9250_mag_reg_write(uint8_t reg, uint8_t value)
{
	_ao_mpu9250_slv4_setup((0 << 7) | MPU9250_MAG_ADDR, reg);

	/* Set the data */
	_ao_mpu9250_reg_write(MPU9250_I2C_SLV4_DO,
			      value);

	_ao_mpu9250_slv4_run();
}

static void
_ao_mpu9250_sample(struct ao_mpu9250_sample *sample)
{
	uint16_t	*d = (uint16_t *) sample;
	int		i = sizeof (*sample) / 2;

	_ao_mpu9250_read(MPU9250_ACCEL_XOUT_H, sample, sizeof (*sample));
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* byte swap */
	while (i--) {
		uint16_t	t = *d;
		*d++ = (uint16_t) ((uint16_t) (t >> 8) | (uint16_t) (t << 8));
	}
#endif
}

#define G	981	/* in cm/s² */

#if 0
static int16_t /* cm/s² */
ao_mpu9250_accel(int16_t v)
{
	return (int16_t) ((v * (int32_t) (16.0 * 980.665 + 0.5)) / 32767);
}

static int16_t	/* deg*10/s */
ao_mpu9250_gyro(int16_t v)
{
	return (int16_t) ((v * (int32_t) 20000) / 32767);
}
#endif

static uint8_t
ao_mpu9250_accel_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < MPU9250_ST_ACCEL(16) / 4) {
		return 1;
	}
	if (diff > MPU9250_ST_ACCEL(16) * 4) {
		return 1;
	}
	return 0;
}

static uint8_t
ao_mpu9250_gyro_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < 0)
		diff = -diff;
	if (diff < MPU9250_ST_GYRO(2000) / 4) {
		return 1;
	}
	if (diff > MPU9250_ST_GYRO(2000) * 4) {
		return 1;
	}
	return 0;
}

static void
_ao_mpu9250_wait_alive(void)
{
	uint8_t	i;

	/* Wait for the chip to wake up */
	for (i = 0; i < 30; i++) {
		ao_delay(AO_MS_TO_TICKS(100));
		if (_ao_mpu9250_reg_read(MPU9250_WHO_AM_I) == MPU9250_I_AM_9250)
			break;
	}
	if (i == 30)
		ao_panic(AO_PANIC_SELF_TEST_MPU9250);
}

#define ST_TRIES	10
#define MAG_TRIES	10

static void
_ao_mpu9250_setup(void)
{
	struct ao_mpu9250_sample	normal_mode, test_mode;
	int				errors;
	int				st_tries;
	int				mag_tries;

	if (ao_mpu9250_configured)
		return;

	_ao_mpu9250_wait_alive();

	/* Reset the whole chip */

	_ao_mpu9250_reg_write(MPU9250_PWR_MGMT_1,
			      (1 << MPU9250_PWR_MGMT_1_DEVICE_RESET));

	/* Wait for it to reset. If we talk too quickly, it appears to get confused */

	_ao_mpu9250_wait_alive();

	/* Reset signal conditioning, disabling I2C on SPI systems */
	_ao_mpu9250_reg_write(MPU9250_USER_CTRL,
			      (0 << MPU9250_USER_CTRL_FIFO_EN) |
			      (1 << MPU9250_USER_CTRL_I2C_MST_EN) |
			      (AO_MPU9250_SPI << MPU9250_USER_CTRL_I2C_IF_DIS) |
			      (0 << MPU9250_USER_CTRL_FIFO_RESET) |
			      (0 << MPU9250_USER_CTRL_I2C_MST_RESET) |
			      (1 << MPU9250_USER_CTRL_SIG_COND_RESET));

	while (_ao_mpu9250_reg_read(MPU9250_USER_CTRL) & (1 << MPU9250_USER_CTRL_SIG_COND_RESET))
		ao_delay(AO_MS_TO_TICKS(10));

	/* Reset signal paths */
	_ao_mpu9250_reg_write(MPU9250_SIGNAL_PATH_RESET,
			      (1 << MPU9250_SIGNAL_PATH_RESET_GYRO_RESET) |
			      (1 << MPU9250_SIGNAL_PATH_RESET_ACCEL_RESET) |
			      (1 << MPU9250_SIGNAL_PATH_RESET_TEMP_RESET));

	_ao_mpu9250_reg_write(MPU9250_SIGNAL_PATH_RESET,
			      (0 << MPU9250_SIGNAL_PATH_RESET_GYRO_RESET) |
			      (0 << MPU9250_SIGNAL_PATH_RESET_ACCEL_RESET) |
			      (0 << MPU9250_SIGNAL_PATH_RESET_TEMP_RESET));

	/* Select clocks, disable sleep */
	_ao_mpu9250_reg_write(MPU9250_PWR_MGMT_1,
			      (0 << MPU9250_PWR_MGMT_1_DEVICE_RESET) |
			      (0 << MPU9250_PWR_MGMT_1_SLEEP) |
			      (0 << MPU9250_PWR_MGMT_1_CYCLE) |
			      (0 << MPU9250_PWR_MGMT_1_TEMP_DIS) |
			      (MPU9250_PWR_MGMT_1_CLKSEL_PLL_X_AXIS << MPU9250_PWR_MGMT_1_CLKSEL));

	/* Set I2C clock and options */
	_ao_mpu9250_reg_write(MPU9250_MST_CTRL,
			      (0 << MPU9250_MST_CTRL_MULT_MST_EN) |
			      (0 << MPU9250_MST_CTRL_WAIT_FOR_ES) |
			      (0 << MPU9250_MST_CTRL_SLV_3_FIFO_EN) |
			      (0 << MPU9250_MST_CTRL_I2C_MST_P_NSR) |
			      (MPU9250_MST_CTRL_I2C_MST_CLK_400 << MPU9250_MST_CTRL_I2C_MST_CLK));

	/* Set sample rate divider to sample at full speed */
	_ao_mpu9250_reg_write(MPU9250_SMPRT_DIV, 0);

	/* Disable filtering */
	_ao_mpu9250_reg_write(MPU9250_CONFIG,
			      (MPU9250_CONFIG_EXT_SYNC_SET_DISABLED << MPU9250_CONFIG_EXT_SYNC_SET) |
			      (MPU9250_CONFIG_DLPF_CFG_250 << MPU9250_CONFIG_DLPF_CFG));

	for (st_tries = 0; st_tries < ST_TRIES; st_tries++) {
		errors = 0;

		/* Configure accelerometer to +/-16G in self-test mode */
		_ao_mpu9250_reg_write(MPU9250_ACCEL_CONFIG,
				      (1 << MPU9250_ACCEL_CONFIG_XA_ST) |
				      (1 << MPU9250_ACCEL_CONFIG_YA_ST) |
				      (1 << MPU9250_ACCEL_CONFIG_ZA_ST) |
				      (MPU9250_ACCEL_CONFIG_AFS_SEL_16G << MPU9250_ACCEL_CONFIG_AFS_SEL));

		/* Configure gyro to +/- 2000°/s in self-test mode */
		_ao_mpu9250_reg_write(MPU9250_GYRO_CONFIG,
				      (1 << MPU9250_GYRO_CONFIG_XG_ST) |
				      (1 << MPU9250_GYRO_CONFIG_YG_ST) |
				      (1 << MPU9250_GYRO_CONFIG_ZG_ST) |
				      (MPU9250_GYRO_CONFIG_FS_SEL_2000 << MPU9250_GYRO_CONFIG_FS_SEL));

		ao_delay(AO_MS_TO_TICKS(200));
		_ao_mpu9250_sample(&test_mode);

		/* Configure accelerometer to +/-16G */
		_ao_mpu9250_reg_write(MPU9250_ACCEL_CONFIG,
				      (0 << MPU9250_ACCEL_CONFIG_XA_ST) |
				      (0 << MPU9250_ACCEL_CONFIG_YA_ST) |
				      (0 << MPU9250_ACCEL_CONFIG_ZA_ST) |
				      (MPU9250_ACCEL_CONFIG_AFS_SEL_16G << MPU9250_ACCEL_CONFIG_AFS_SEL));

		/* Configure gyro to +/- 2000°/s */
		_ao_mpu9250_reg_write(MPU9250_GYRO_CONFIG,
				      (0 << MPU9250_GYRO_CONFIG_XG_ST) |
				      (0 << MPU9250_GYRO_CONFIG_YG_ST) |
				      (0 << MPU9250_GYRO_CONFIG_ZG_ST) |
				      (MPU9250_GYRO_CONFIG_FS_SEL_2000 << MPU9250_GYRO_CONFIG_FS_SEL));

		ao_delay(AO_MS_TO_TICKS(200));
		_ao_mpu9250_sample(&normal_mode);

		errors += ao_mpu9250_accel_check(normal_mode.accel_x, test_mode.accel_x);
		errors += ao_mpu9250_accel_check(normal_mode.accel_y, test_mode.accel_y);
		errors += ao_mpu9250_accel_check(normal_mode.accel_z, test_mode.accel_z);

		errors += ao_mpu9250_gyro_check(normal_mode.gyro_x, test_mode.gyro_x);
		errors += ao_mpu9250_gyro_check(normal_mode.gyro_y, test_mode.gyro_y);
		errors += ao_mpu9250_gyro_check(normal_mode.gyro_z, test_mode.gyro_z);
		if (!errors)
			break;
	}

	if (st_tries == ST_TRIES)
		AO_SENSOR_ERROR(AO_DATA_MPU9250);

	/* Set up the mag sensor */

	/* make sure it's alive */
	for (mag_tries = 0; mag_tries < MAG_TRIES; mag_tries++) {
		if (_ao_mpu9250_mag_reg_read(MPU9250_MAG_WIA) == MPU9250_MAG_WIA_VALUE)
			break;
	}

	if (mag_tries == MAG_TRIES)
		AO_SENSOR_ERROR(AO_DATA_MPU9250);

	/* Select continuous mode 2 (100Hz), 16 bit samples */

	_ao_mpu9250_mag_reg_write(MPU9250_MAG_CNTL1,
				  (MPU9250_MAG_CNTL1_BIT_16 << MPU9250_MAG_CNTL1_BIT) |
				  (MPU9250_MAG_CNTL1_MODE_CONT_2 << MPU9250_MAG_CNTL1_MODE));

	/* Set i2c master to delay shadowing data until read is
	 * complete (avoids tearing the data) */

	_ao_mpu9250_reg_write(MPU9250_I2C_MST_DELAY_CTRL,
			      (1 << MPU9250_I2C_MST_DELAY_CTRL_DELAY_ES_SHADOW) |
			      (0 << MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV4_DLY_EN) |
			      (0 << MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV3_DLY_EN) |
			      (0 << MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV2_DLY_EN) |
			      (0 << MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV1_DLY_EN) |
			      (0 << MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV0_DLY_EN));

	/* Set up i2c slave 0 to read the mag registers starting at HXL (3) */

	_ao_mpu9250_reg_write(MPU9250_I2C_SLV0_ADDR,
			      (1 << 7) | MPU9250_MAG_ADDR);

	_ao_mpu9250_reg_write(MPU9250_I2C_SLV0_REG,
			      MPU9250_MAG_HXL);

	/* Byte swap so the mag values match the gyro/accel. Read 7 bytes
	 * to include the status register
	 */

	_ao_mpu9250_reg_write(MPU9250_I2C_SLV0_CTRL,
			      (1 << MPU9250_I2C_SLV0_CTRL_I2C_SLV0_EN) |
			      (1 << MPU9250_I2C_SLV0_CTRL_I2C_SLV0_BYTE_SW) |
			      (0 << MPU9250_I2C_SLV0_CTRL_I2C_SLV0_REG_DIS) |
			      (1 << MPU9250_I2C_SLV0_CTRL_I2C_SLV0_GRP) |
			      (MPU9250_MAG_ST2 - MPU9250_MAG_HXL + 1) << MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG);

	/* Filter to about 100Hz, which also sets the gyro rate to 1000Hz */
	_ao_mpu9250_reg_write(MPU9250_CONFIG,
			      (MPU9250_CONFIG_FIFO_MODE_REPLACE << MPU9250_CONFIG_FIFO_MODE) |
			      (MPU9250_CONFIG_EXT_SYNC_SET_DISABLED << MPU9250_CONFIG_EXT_SYNC_SET) |
			      (MPU9250_CONFIG_DLPF_CFG_92 << MPU9250_CONFIG_DLPF_CFG));

	/* Set sample rate divider to sample at 200Hz (v = gyro/rate - 1) */
	_ao_mpu9250_reg_write(MPU9250_SMPRT_DIV,
			      1000 / 200 - 1);

	ao_delay(AO_MS_TO_TICKS(100));
	ao_mpu9250_configured = 1;
}

struct ao_mpu9250_sample	ao_mpu9250_current;

static void
ao_mpu9250(void)
{
	struct ao_mpu9250_sample	sample;

	/* ao_mpu9250_init already grabbed the SPI bus and mutex */
	_ao_mpu9250_setup();
	ao_mpu9250_spi_put();
	for (;;)
	{
		ao_mpu9250_spi_get();
		_ao_mpu9250_sample(&sample);
		ao_mpu9250_spi_put();
		ao_arch_block_interrupts();
		ao_mpu9250_current = sample;
		AO_DATA_PRESENT(AO_DATA_MPU9250);
		AO_DATA_WAIT();
		ao_arch_release_interrupts();
	}
}

static struct ao_task ao_mpu9250_task;

static void
ao_mpu9250_show(void)
{
	printf ("Accel: %7d %7d %7d Gyro: %7d %7d %7d Mag: %7d %7d %7d\n",
		ao_mpu9250_current.accel_x,
		ao_mpu9250_current.accel_y,
		ao_mpu9250_current.accel_z,
		ao_mpu9250_current.gyro_x,
		ao_mpu9250_current.gyro_y,
		ao_mpu9250_current.gyro_z,
		ao_mpu9250_current.mag_x,
		ao_mpu9250_current.mag_y,
		ao_mpu9250_current.mag_z);
}

#if MPU9250_TEST

static void
ao_mpu9250_read(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_mpu9250_spi_get();
	val = _ao_mpu9250_reg_read(addr);
	ao_mpu9250_spi_put();
	printf("Addr %02x val %02x\n", addr, val);
}

static void
ao_mpu9250_write(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	val = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	printf("Addr %02x val %02x\n", addr, val);
	ao_mpu9250_spi_get();
	_ao_mpu9250_reg_write(addr, val);
	ao_mpu9250_spi_put();
}

static void
ao_mpu9250_mag_read(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_mpu9250_spi_get();
	val = _ao_mpu9250_mag_reg_read(addr);
	ao_mpu9250_spi_put();
	printf("Addr %02x val %02x\n", addr, val);
}

static void
ao_mpu9250_mag_write(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	val = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	printf("Addr %02x val %02x\n", addr, val);
	ao_mpu9250_spi_get();
	_ao_mpu9250_mag_reg_write(addr, val);
	ao_mpu9250_spi_put();
}

#endif /* MPU9250_TEST */

static const struct ao_cmds ao_mpu9250_cmds[] = {
	{ ao_mpu9250_show,	"I\0Show MPU9250 status" },
#if MPU9250_TEST
	{ ao_mpu9250_read,	"R <addr>\0Read MPU9250 register" },
	{ ao_mpu9250_write,	"W <addr> <val>\0Write MPU9250 register" },
	{ ao_mpu9250_mag_read,	"G <addr>\0Read MPU9250 Mag register" },
	{ ao_mpu9250_mag_write,	"P <addr> <val>\0Write MPU9250 Mag register" },
#endif
	{ 0, NULL }
};

void
ao_mpu9250_init(void)
{
	ao_mpu9250_configured = 0;

	ao_add_task(&ao_mpu9250_task, ao_mpu9250, "mpu9250");

#if AO_MPU9250_SPI
	ao_spi_init_cs(AO_MPU9250_SPI_CS_PORT, (1 << AO_MPU9250_SPI_CS_PIN));

	/* Pretend to be the mpu9250 task. Grab the SPI bus right away and
	 * hold it for the task so that nothing else uses the SPI bus before
	 * we get the I2C mode disabled in the chip
	 */

	ao_cur_task = &ao_mpu9250_task;
	ao_mpu9250_spi_get();
	ao_cur_task = NULL;
#endif
	ao_cmd_register(&ao_mpu9250_cmds[0]);
}
#endif
