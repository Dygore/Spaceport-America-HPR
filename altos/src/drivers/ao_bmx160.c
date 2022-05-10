/*
 * Copyright © 2019 Keith Packard <keithp@keithp.com>
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
#include <ao_bmx160.h>
#include <ao_exti.h>

static uint8_t	ao_bmx160_configured;

static struct ao_bmm150_trim ao_bmm150_trim;

#define AO_BMX160_SPI_SPEED	ao_spi_speed(10000000)

#define ao_bmx160_spi_get()	ao_spi_get(AO_BMX160_SPI_BUS, AO_BMX160_SPI_SPEED)
#define ao_bmx160_spi_put()	ao_spi_put(AO_BMX160_SPI_BUS)

#define ao_bmx160_spi_start() 	ao_spi_set_cs(AO_BMX160_SPI_CS_PORT,	\
					      (1 << AO_BMX160_SPI_CS_PIN))

#define ao_bmx160_spi_end() 	ao_spi_clr_cs(AO_BMX160_SPI_CS_PORT,	\
					      (1 << AO_BMX160_SPI_CS_PIN))

static void
_ao_bmx160_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[2] = { addr, value };
	ao_bmx160_spi_start();
	ao_spi_send(d, 2, AO_BMX160_SPI_BUS);
	ao_bmx160_spi_end();
}

static void
_ao_bmx160_read(uint8_t addr, void *data, uint8_t len)
{
	addr |= 0x80;
	ao_bmx160_spi_start();
	ao_spi_send(&addr, 1, AO_BMX160_SPI_BUS);
	ao_spi_recv(data, len, AO_BMX160_SPI_BUS);
	ao_bmx160_spi_end();
}

static uint8_t
_ao_bmx160_reg_read(uint8_t addr)
{
	uint8_t	value;
	addr |= 0x80;
	ao_bmx160_spi_start();
	ao_spi_send(&addr, 1, AO_BMX160_SPI_BUS);
	ao_spi_recv(&value, 1, AO_BMX160_SPI_BUS);
	ao_bmx160_spi_end();
	return value;
}

static void
_ao_bmx160_cmd(uint8_t cmd)
{
	int i;
	_ao_bmx160_reg_write(BMX160_CMD, cmd);
	for (i = 0; i < 50; i++) {
		uint8_t cmd_read;
		cmd_read = _ao_bmx160_reg_read(BMX160_CMD);
		if (cmd_read != cmd)
			break;
	}
}

static void
_ao_bmm150_wait_manual(void)
{
	while (_ao_bmx160_reg_read(BMX160_STATUS) & (1 << BMX160_STATUS_MAG_MAN_OP))
		;
}

static void
_ao_bmm150_reg_write(uint8_t addr, uint8_t data)
{
	_ao_bmx160_reg_write(BMX160_MAG_IF_3, data);
	_ao_bmx160_reg_write(BMX160_MAG_IF_2, addr);
	_ao_bmm150_wait_manual();
}

static uint8_t
_ao_bmm150_reg_read(uint8_t addr)
{
	_ao_bmx160_reg_write(BMX160_MAG_IF_1, addr);
	_ao_bmm150_wait_manual();
	uint8_t ret = _ao_bmx160_reg_read(BMX160_DATA_0);
	return ret;
}

static uint16_t
_ao_bmm150_reg_read2(uint8_t lo_addr, uint8_t hi_addr)
{
	uint8_t lo = _ao_bmm150_reg_read(lo_addr);
	uint8_t hi = _ao_bmm150_reg_read(hi_addr);

	return (uint16_t) (((uint16_t) hi << 8) | (uint16_t) lo);
}

/*
 * The compensate functions are taken from the BMM150 sample
 * driver which has the following copyright:
 *
 * Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file bmm150.c
 * @date 10/01/2020
 * @version  1.0.3
 *
 */

/*!
 * @brief This internal API is used to obtain the compensated
 * magnetometer X axis data(micro-tesla) in int16_t.
 */
static int16_t compensate_x(int16_t mag_data_x, uint16_t data_rhall)
{
    int16_t retval;
    uint16_t process_comp_x0 = 0;
    int32_t process_comp_x1;
    uint16_t process_comp_x2;
    int32_t process_comp_x3;
    int32_t process_comp_x4;
    int32_t process_comp_x5;
    int32_t process_comp_x6;
    int32_t process_comp_x7;
    int32_t process_comp_x8;
    int32_t process_comp_x9;
    int32_t process_comp_x10;

    /* Overflow condition check */
    if (mag_data_x != BMM150_XYAXES_FLIP_OVERFLOW_ADCVAL)
    {
        if (data_rhall != 0)
        {
            /* Availability of valid data*/
            process_comp_x0 = data_rhall;
	    //printf("using data_rhall %d\n", data_rhall);
        }
        else if (ao_bmm150_trim.dig_xyz1 != 0)
        {
            process_comp_x0 = ao_bmm150_trim.dig_xyz1;
	    //printf("using trim value %d\n", process_comp_x0);
        }
        else
        {
            process_comp_x0 = 0;
	    //printf("no valid rhall\n");
        }
        if (process_comp_x0 != 0)
        {
            /* Processing compensation equations*/
            process_comp_x1 = ((int32_t)ao_bmm150_trim.dig_xyz1) * 16384;
	    //printf("comp_x1 %d\n", process_comp_x1);
            process_comp_x2 = ((uint16_t)(process_comp_x1 / process_comp_x0)) - ((uint16_t)0x4000);
	    //printf("comp_x2 %d\n", process_comp_x2);
            retval = ((int16_t)process_comp_x2);
            process_comp_x3 = (((int32_t)retval) * ((int32_t)retval));
	    //printf("comp_x3 %d\n", process_comp_x3);
            process_comp_x4 = (((int32_t)ao_bmm150_trim.dig_xy2) * (process_comp_x3 / 128));
	    //printf("comp_x4 %d\n", process_comp_x4);
            process_comp_x5 = (int32_t)(((int16_t)ao_bmm150_trim.dig_xy1) * 128);
	    //printf("comp_x5 %d\n", process_comp_x5);
            process_comp_x6 = ((int32_t)retval) * process_comp_x5;
	    //printf("comp_x6 %d\n", process_comp_x6);
            process_comp_x7 = (((process_comp_x4 + process_comp_x6) / 512) + ((int32_t)0x100000));
	    //printf("comp_x7 %d\n", process_comp_x7);
            process_comp_x8 = ((int32_t)(((int16_t)ao_bmm150_trim.dig_x2) + ((int16_t)0xA0)));
	    //printf("comp_x8 %d\n", process_comp_x8);
            process_comp_x9 = ((process_comp_x7 * process_comp_x8) / 4096);
	    //printf("comp_x9 %d\n", process_comp_x9);
            process_comp_x10 = ((int32_t)mag_data_x) * process_comp_x9;
	    //printf("comp_x10 %d\n", process_comp_x10);
            retval = ((int16_t)(process_comp_x10 / 8192));
	    //printf("ret 1 %d\n", retval);
            retval = (int16_t) ((retval + (((int16_t)ao_bmm150_trim.dig_x1) * 8)) / 16);
	    //printf("final %d\n", retval);
        }
        else
        {
            retval = BMM150_OVERFLOW_OUTPUT;
        }
    }
    else
    {
        /* Overflow condition */
        retval = BMM150_OVERFLOW_OUTPUT;
    }

    return retval;
}

/*!
 * @brief This internal API is used to obtain the compensated
 * magnetometer Y axis data(micro-tesla) in int16_t.
 */
static int16_t compensate_y(int16_t mag_data_y, uint16_t data_rhall)
{
    int16_t retval;
    uint16_t process_comp_y0 = 0;
    int32_t process_comp_y1;
    uint16_t process_comp_y2;
    int32_t process_comp_y3;
    int32_t process_comp_y4;
    int32_t process_comp_y5;
    int32_t process_comp_y6;
    int32_t process_comp_y7;
    int32_t process_comp_y8;
    int32_t process_comp_y9;

    /* Overflow condition check */
    if (mag_data_y != BMM150_XYAXES_FLIP_OVERFLOW_ADCVAL)
    {
        if (data_rhall != 0)
        {
            /* Availability of valid data*/
            process_comp_y0 = data_rhall;
        }
        else if (ao_bmm150_trim.dig_xyz1 != 0)
        {
            process_comp_y0 = ao_bmm150_trim.dig_xyz1;
        }
        else
        {
            process_comp_y0 = 0;
        }
        if (process_comp_y0 != 0)
        {
            /*Processing compensation equations*/
            process_comp_y1 = (((int32_t)ao_bmm150_trim.dig_xyz1) * 16384) / process_comp_y0;
            process_comp_y2 = ((uint16_t)process_comp_y1) - ((uint16_t)0x4000);
            retval = ((int16_t)process_comp_y2);
            process_comp_y3 = ((int32_t) retval) * ((int32_t)retval);
            process_comp_y4 = ((int32_t)ao_bmm150_trim.dig_xy2) * (process_comp_y3 / 128);
            process_comp_y5 = ((int32_t)(((int16_t)ao_bmm150_trim.dig_xy1) * 128));
            process_comp_y6 = ((process_comp_y4 + (((int32_t)retval) * process_comp_y5)) / 512);
            process_comp_y7 = ((int32_t)(((int16_t)ao_bmm150_trim.dig_y2) + ((int16_t)0xA0)));
            process_comp_y8 = (((process_comp_y6 + ((int32_t)0x100000)) * process_comp_y7) / 4096);
            process_comp_y9 = (((int32_t)mag_data_y) * process_comp_y8);
            retval = (int16_t)(process_comp_y9 / 8192);
            retval = (int16_t) ((retval + (((int16_t)ao_bmm150_trim.dig_y1) * 8)) / 16);
        }
        else
        {
            retval = BMM150_OVERFLOW_OUTPUT;
        }
    }
    else
    {
        /* Overflow condition*/
        retval = BMM150_OVERFLOW_OUTPUT;
    }

    return retval;
}

/*!
 * @brief This internal API is used to obtain the compensated
 * magnetometer Z axis data(micro-tesla) in int16_t.
 */
static int16_t compensate_z(int16_t mag_data_z, uint16_t data_rhall)
{
    int32_t retval;
    int16_t process_comp_z0;
    int32_t process_comp_z1;
    int32_t process_comp_z2;
    int32_t process_comp_z3;
    int16_t process_comp_z4;

    if (mag_data_z != BMM150_ZAXIS_HALL_OVERFLOW_ADCVAL)
    {
        if ((ao_bmm150_trim.dig_z2 != 0) && (ao_bmm150_trim.dig_z1 != 0) && (data_rhall != 0) &&
            (ao_bmm150_trim.dig_xyz1 != 0))
        {
            /*Processing compensation equations*/
            process_comp_z0 = ((int16_t)data_rhall) - ((int16_t) ao_bmm150_trim.dig_xyz1);
            process_comp_z1 = (((int32_t)ao_bmm150_trim.dig_z3) * ((int32_t)(process_comp_z0))) / 4;
            process_comp_z2 = (((int32_t)(mag_data_z - ao_bmm150_trim.dig_z4)) * 32768);
            process_comp_z3 = ((int32_t)ao_bmm150_trim.dig_z1) * (((int16_t)data_rhall) * 2);
            process_comp_z4 = (int16_t)((process_comp_z3 + (32768)) / 65536);
            retval = ((process_comp_z2 - process_comp_z1) / (ao_bmm150_trim.dig_z2 + process_comp_z4));

            /* saturate result to +/- 2 micro-tesla */
            if (retval > BMM150_POSITIVE_SATURATION_Z)
            {
                retval = BMM150_POSITIVE_SATURATION_Z;
            }
            else if (retval < BMM150_NEGATIVE_SATURATION_Z)
            {
                retval = BMM150_NEGATIVE_SATURATION_Z;
            }

            /* Conversion of LSB to micro-tesla*/
            retval = retval / 16;
        }
        else
        {
            retval = BMM150_OVERFLOW_OUTPUT;
        }
    }
    else
    {
        /* Overflow condition*/
        retval = BMM150_OVERFLOW_OUTPUT;
    }

    return (int16_t)retval;
}

static void
_ao_bmx160_sample(struct ao_bmx160_sample *sample)
{
	_ao_bmx160_read(BMX160_MAG_X_0_7, sample, sizeof (*sample));
#if __BYTE_ORDER != __LITTLE_ENDIAN
	int		i = sizeof (*sample) / 2;
	uint16_t	*d = (uint16_t *) sample;

	/* byte swap */
	while (i--) {
		uint16_t	t = *d;
		*d++ = (t >> 8) | (t << 8);
	}
#endif
	uint16_t rhall = sample->rhall >> 2;
	sample->mag_x = compensate_x(sample->mag_x >> 3, rhall);
	sample->mag_y = compensate_y(sample->mag_y >> 3, rhall);
	sample->mag_z = compensate_z(sample->mag_z >> 1, rhall);
}

#define G	981	/* in cm/s² */

#if 0
static int16_t /* cm/s² */
ao_bmx160_accel(int16_t v)
{
	return (int16_t) ((v * (int32_t) (16.0 * 980.665 + 0.5)) / 32767);
}

static int16_t	/* deg*10/s */
ao_bmx160_gyro(int16_t v)
{
	return (int16_t) ((v * (int32_t) 20000) / 32767);
}

static uint8_t
ao_bmx160_accel_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < BMX160_ST_ACCEL(16) / 4) {
		return 1;
	}
	if (diff > BMX160_ST_ACCEL(16) * 4) {
		return 1;
	}
	return 0;
}

static uint8_t
ao_bmx160_gyro_check(int16_t normal, int16_t test)
{
	int16_t	diff = test - normal;

	if (diff < 0)
		diff = -diff;
	if (diff < BMX160_ST_GYRO(2000) / 4) {
		return 1;
	}
	if (diff > BMX160_ST_GYRO(2000) * 4) {
		return 1;
	}
	return 0;
}
#endif

static void
_ao_bmx160_wait_alive(void)
{
	uint8_t	i;

	/* Wait for the chip to wake up */
	for (i = 0; i < 30; i++) {
		ao_delay(AO_MS_TO_TICKS(100));
		if (_ao_bmx160_reg_read(BMX160_CHIPID) == BMX160_CHIPID_BMX160)
			break;
	}
	if (i == 30)
		ao_panic(AO_PANIC_SELF_TEST_BMX160);
}

#define ST_TRIES	10
#define MAG_TRIES	10

static void
_ao_bmx160_setup(void)
{
	int r;

	if (ao_bmx160_configured)
		return;

	/* Dummy read of 0x7f register to enable SPI interface */
	(void) _ao_bmx160_reg_read(0x7f);

	/* Make sure the chip is responding */
	_ao_bmx160_wait_alive();

	/* Force SPI mode */
	_ao_bmx160_reg_write(BMX160_NV_CONF, 1 << BMX160_NV_CONF_SPI_EN);

	/* Enable acc and gyr
	 */

	_ao_bmx160_cmd(BMX160_CMD_ACC_SET_PMU_MODE(BMX160_PMU_STATUS_ACC_PMU_STATUS_NORMAL));

	for (r = 0; r < 20; r++) {
		ao_delay(AO_MS_TO_TICKS(100));
		if (((_ao_bmx160_reg_read(BMX160_PMU_STATUS)
		      >> BMX160_PMU_STATUS_ACC_PMU_STATUS)
		     & BMX160_PMU_STATUS_ACC_PMU_STATUS_MASK)
		    == BMX160_PMU_STATUS_ACC_PMU_STATUS_NORMAL)
		{
			r = 0;
			break;
		}
	}
	if (r != 0)
		AO_SENSOR_ERROR(AO_DATA_BMX160);

	_ao_bmx160_cmd(BMX160_CMD_GYR_SET_PMU_MODE(BMX160_PMU_STATUS_GYR_PMU_STATUS_NORMAL));

	for (r = 0; r < 20; r++) {
		ao_delay(AO_MS_TO_TICKS(100));
		if (((_ao_bmx160_reg_read(BMX160_PMU_STATUS)
		      >> BMX160_PMU_STATUS_GYR_PMU_STATUS)
		     & BMX160_PMU_STATUS_GYR_PMU_STATUS_MASK)
		    == BMX160_PMU_STATUS_GYR_PMU_STATUS_NORMAL)
		{
			r = 0;
			break;
		}
	}
	if (r != 0)
		AO_SENSOR_ERROR(AO_DATA_BMX160);

	/* Configure accelerometer:
	 *
	 * 	undersampling disabled
	 * 	normal filter
	 *	200Hz sampling rate
	 *	16g range
	 *
	 * This yields a 3dB cutoff frequency of 80Hz
	 */
	_ao_bmx160_reg_write(BMX160_ACC_CONF,
			     (0 << BMX160_ACC_CONF_ACC_US) |
			     (BMX160_ACC_CONF_ACC_BWP_NORMAL << BMX160_ACC_CONF_ACC_BWP) |
			     (BMX160_ACC_CONF_ACC_ODR_200 << BMX160_ACC_CONF_ACC_ODR));
	_ao_bmx160_reg_write(BMX160_ACC_RANGE,
			     BMX160_ACC_RANGE_16G);

	for (r = 0x3; r <= 0x1b; r++)
		(void) _ao_bmx160_reg_read((uint8_t) r);

	/* Configure gyro:
	 *
	 * 	200Hz sampling rate
	 *	Normal filter mode
	 *	±2000°/s
	 */
	_ao_bmx160_reg_write(BMX160_GYR_CONF,
			     (BMX160_GYR_CONF_GYR_BWP_NORMAL << BMX160_GYR_CONF_GYR_BWP) |
			     (BMX160_GYR_CONF_GYR_ODR_200 << BMX160_GYR_CONF_GYR_ODR));
	_ao_bmx160_reg_write(BMX160_GYR_RANGE,
			     BMX160_GYR_RANGE_2000);


	/* Configure magnetometer:
	 *
	 *	30Hz sampling rate
	 *	power on
	 *	axes enabled
	 */
	_ao_bmx160_cmd(BMX160_CMD_MAG_IF_SET_PMU_MODE(BMX160_PMU_STATUS_MAG_IF_PMU_STATUS_NORMAL));

	_ao_bmx160_reg_write(BMX160_IF_CONF,
			     (BMX160_IF_CONF_IF_MODE_AUTO_MAG << BMX160_IF_CONF_IF_MODE));

	/* Enter setup mode */
	_ao_bmx160_reg_write(BMX160_MAG_IF_0,
			     (1 << BMX160_MAG_IF_0_MAG_MANUAL_EN) |
			     (0 << BMX160_MAG_IF_0_MAG_OFFSET) |
			     (0 << BMX160_MAG_IF_0_MAG_RD_BURST));

	/* Place in suspend mode to reboot the chip */
	_ao_bmm150_reg_write(BMM150_POWER_MODE,
			     (0 << BMM150_POWER_MODE_POWER_CONTROL));

	/* Power on */
	_ao_bmm150_reg_write(BMM150_POWER_MODE,
			     (1 << BMM150_POWER_MODE_POWER_CONTROL));

	/* Set data rate and place in sleep mode */
	_ao_bmm150_reg_write(BMM150_CONTROL,
			     (BMM150_CONTROL_DATA_RATE_30 << BMM150_CONTROL_DATA_RATE) |
			     (BMM150_CONTROL_OP_MODE_SLEEP << BMM150_CONTROL_OP_MODE));

	/* enable all axes (should already be enabled) */
	_ao_bmm150_reg_write(BMM150_INT_CONF,
			     (0 << BMM150_INT_CONF_X_DISABLE) |
			     (0 << BMM150_INT_CONF_Y_DISABLE) |
			     (0 << BMM150_INT_CONF_Z_DISABLE));

	/* Set repetition values (?) */
	_ao_bmm150_reg_write(BMM150_REPXY, BMM150_REPXY_VALUE(9));
	_ao_bmm150_reg_write(BMM150_REPZ, BMM150_REPZ_VALUE(15));

	/* Read Trim values */
	ao_bmm150_trim.dig_x1   = (int8_t) _ao_bmm150_reg_read(BMM150_DIG_X1);
	ao_bmm150_trim.dig_y1   = (int8_t) _ao_bmm150_reg_read(BMM150_DIG_Y1);
	ao_bmm150_trim.dig_z4   = (int8_t) _ao_bmm150_reg_read2(BMM150_DIG_Z4_LSB, BMM150_DIG_Z4_MSB);
	ao_bmm150_trim.dig_x2   = (int8_t) _ao_bmm150_reg_read(BMM150_DIG_X2);
	ao_bmm150_trim.dig_y2   = (int8_t) _ao_bmm150_reg_read(BMM150_DIG_Y2);
	ao_bmm150_trim.dig_z2   = (int8_t) _ao_bmm150_reg_read2(BMM150_DIG_Z2_LSB, BMM150_DIG_Z2_MSB);
	ao_bmm150_trim.dig_z1   = _ao_bmm150_reg_read2(BMM150_DIG_Z1_LSB, BMM150_DIG_Z1_MSB);
	ao_bmm150_trim.dig_xyz1 = _ao_bmm150_reg_read2(BMM150_DIG_XYZ1_LSB, BMM150_DIG_XYZ1_MSB);
	ao_bmm150_trim.dig_z3   = (int8_t) _ao_bmm150_reg_read2(BMM150_DIG_Z3_LSB, BMM150_DIG_Z3_MSB);
	ao_bmm150_trim.dig_xy2  = (int8_t) _ao_bmm150_reg_read(BMM150_DIG_XY2);
	ao_bmm150_trim.dig_xy1  = _ao_bmm150_reg_read(BMM150_DIG_XY1);

	/* To get data out of the magnetometer, set the control op mode to 'forced', then read
	 * from the data registers
	 */
	_ao_bmx160_reg_write(BMX160_MAG_IF_3,
			     (BMM150_CONTROL_DATA_RATE_30 << BMM150_CONTROL_DATA_RATE) |
			     (BMM150_CONTROL_OP_MODE_FORCED << BMM150_CONTROL_OP_MODE));
	_ao_bmx160_reg_write(BMX160_MAG_IF_2, BMM150_CONTROL);
	_ao_bmx160_reg_write(BMX160_MAG_IF_1, BMM150_DATA_X_0_4);

	/* Put magnetometer interface back into 'normal mode'
	 */
	_ao_bmx160_reg_write(BMX160_MAG_IF_0,
			     (0 << BMX160_MAG_IF_0_MAG_MANUAL_EN) |
			     (0 << BMX160_MAG_IF_0_MAG_OFFSET) |
			     (3 << BMX160_MAG_IF_0_MAG_RD_BURST));

	/* Set data rate to 200Hz */
	_ao_bmx160_reg_write(BMX160_MAG_CONF,
			     (BMX160_MAG_CONF_MAG_ODR_200 << BMX160_MAG_CONF_MAG_ODR));

	ao_bmx160_configured = 1;
}

struct ao_bmx160_sample	ao_bmx160_current;

static void
ao_bmx160(void)
{
	struct ao_bmx160_sample	sample;

	/* ao_bmx160_init already grabbed the SPI bus and mutex */
	_ao_bmx160_setup();
	ao_bmx160_spi_put();
	for (;;)
	{
		ao_bmx160_spi_get();
		_ao_bmx160_sample(&sample);
		ao_bmx160_spi_put();
		ao_arch_block_interrupts();
		ao_bmx160_current = sample;
		AO_DATA_PRESENT(AO_DATA_BMX160);
		AO_DATA_WAIT();
		ao_arch_release_interrupts();
	}
}

static struct ao_task ao_bmx160_task;

static void
ao_bmx160_show(void)
{
	printf ("Accel: %7d %7d %7d Gyro: %7d %7d %7d Mag: %7d %7d %7d\n",
		ao_bmx160_current.acc_x,
		ao_bmx160_current.acc_y,
		ao_bmx160_current.acc_z,
		ao_bmx160_current.gyr_x,
		ao_bmx160_current.gyr_y,
		ao_bmx160_current.gyr_z,
		ao_bmx160_current.mag_x,
		ao_bmx160_current.mag_y,
		ao_bmx160_current.mag_z);
}

#if BMX160_TEST

static void
ao_bmx160_read(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_bmx160_spi_get();
	val = _ao_bmx160_reg_read(addr);
	ao_bmx160_spi_put();
	printf("Addr %02x val %02x\n", addr, val);
}

static void
ao_bmx160_write(void)
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
	ao_bmx160_spi_get();
	_ao_bmx160_reg_write(addr, val);
	ao_bmx160_spi_put();
}

static void
ao_bmm150_read(void)
{
	uint8_t	addr;
	uint8_t val;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_bmx160_spi_get();
	val = _ao_bmm150_reg_read(addr);
	ao_bmx160_spi_put();
	printf("Addr %02x val %02x\n", addr, val);
}

static void
ao_bmm150_write(void)
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
	ao_bmx160_spi_get();
	_ao_bmm150_reg_write(addr, val);
	ao_bmx160_spi_put();
}

#endif /* BMX160_TEST */

static const struct ao_cmds ao_bmx160_cmds[] = {
	{ ao_bmx160_show,	"I\0Show BMX160 status" },
#if BMX160_TEST
	{ ao_bmx160_read,	"R <addr>\0Read BMX160 register" },
	{ ao_bmx160_write,	"W <addr> <val>\0Write BMX160 register" },
	{ ao_bmm150_read,	"M <addr>\0Read BMM150 register" },
	{ ao_bmm150_write,	"N <addr> <val>\0Write BMM150 register" },
#endif
	{ 0, NULL }
};

void
ao_bmx160_init(void)
{
	ao_spi_init_cs(AO_BMX160_SPI_CS_PORT, (1 << AO_BMX160_SPI_CS_PIN));

	ao_add_task(&ao_bmx160_task, ao_bmx160, "bmx160");

	/* Pretend to be the bmx160 task. Grab the SPI bus right away and
	 * hold it for the task so that nothing else uses the SPI bus before
	 * we get the I2C mode disabled in the chip
	 */

	ao_cur_task = &ao_bmx160_task;
	ao_bmx160_spi_get();
	ao_cur_task = NULL;

	ao_cmd_register(&ao_bmx160_cmds[0]);
}
