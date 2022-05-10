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

#ifndef _AO_ADXL375_H_
#define _AO_ADXL375_H_

#define AO_ADXL375_READ			0x80	/* read mode */
#define AO_ADXL375_MULTI_BYTE		0x40	/* multi-byte mode */

#define AO_ADXL375_DEVID		0x00
#define  AO_ADXL375_DEVID_ID			0xe5
#define AO_ADXL375_THRESH_SHOCK		0x1d
#define AO_ADXL375_OFSX			0x1e
#define AO_ADXL375_OFSY			0x1f
#define AO_ADXL375_OFSZ			0x20
#define AO_ADXL375_DUR			0x21
#define AO_ADXL375_LATENT		0x22
#define AO_ADXL375_WINDOW		0x23
#define AO_ADXL375_THRESH_ACT		0x24
#define AO_ADXL375_THRESH_INACT		0x25
#define AO_ADXL375_TIME_INACT		0x26
#define AO_ADXL375_ACT_INACT_CTL	0x27
#define AO_ADXL375_SHOCK_AXES		0x2a
#define AO_ADXL375_ACT_SHOCK_STATUS	0x2b
#define AO_ADXL375_BW_RATE		0x2c

#define  AO_ADXL375_BW_RATE_LOW_POWER		4
#define  AO_ADXL375_BW_RATE_RATE		0
#define   AO_ADXL375_BW_RATE_RATE_3200		0xf
#define   AO_ADXL375_BW_RATE_RATE_1600		0xe
#define   AO_ADXL375_BW_RATE_RATE_800		0xd
#define   AO_ADXL375_BW_RATE_RATE_400		0xc
#define   AO_ADXL375_BW_RATE_RATE_200		0xb
#define   AO_ADXL375_BW_RATE_RATE_100		0xa
#define   AO_ADXL375_BW_RATE_RATE_50		0x9
#define   AO_ADXL375_BW_RATE_RATE_25		0x8
#define   AO_ADXL375_BW_RATE_RATE_12_5		0x7
#define   AO_ADXL375_BW_RATE_RATE_6_25		0x6
#define   AO_ADXL375_BW_RATE_RATE_3_13		0x5
#define   AO_ADXL375_BW_RATE_RATE_1_56		0x4
#define   AO_ADXL375_BW_RATE_RATE_0_78		0x3
#define   AO_ADXL375_BW_RATE_RATE_0_39		0x2
#define   AO_ADXL375_BW_RATE_RATE_0_20		0x1
#define   AO_ADXL375_BW_RATE_RATE_0_10		0x0

#define AO_ADXL375_POWER_CTL		0x2d
#define  AO_ADXL375_POWER_CTL_LINK		5
#define  AO_ADXL375_POWER_CTL_AUTO_SLEEP	4
#define  AO_ADXL375_POWER_CTL_MEASURE		3
#define  AO_ADXL375_POWER_CTL_SLEEP		2
#define  AO_ADXL375_POWER_CTL_WAKEUP		0
#define  AO_ADXL375_POWER_CTL_WAKEUP_8			0
#define  AO_ADXL375_POWER_CTL_WAKEUP_4			1
#define  AO_ADXL375_POWER_CTL_WAKEUP_2			2
#define  AO_ADXL375_POWER_CTL_WAKEUP_1			3

#define AO_ADXL375_INT_ENABLE		0x2e
#define AO_ADXL375_INT_MAP		0x2f
#define AO_ADXL375_INT_SOURCE		0x30
#define AO_ADXL375_DATA_FORMAT		0x31
# define AO_ADXL375_DATA_FORMAT_FIXED		0x0b	/* these bits must be set to 1 */
# define AO_ADXL375_DATA_FORMAT_SELF_TEST	7
# define AO_ADXL375_DATA_FORMAT_SPI		6
# define  AO_ADXL375_DATA_FORMAT_SPI_3_WIRE		0
# define  AO_ADXL375_DATA_FORMAT_SPI_4_WIRE		1
# define AO_ADXL375_DATA_FORMAT_INT_INVERT	5
# define AO_ADXL375_DATA_FORMAT_JUSTIFY		2
#define AO_ADXL375_DATAX0		0x32
#define AO_ADXL375_DATAX1		0x33
#define AO_ADXL375_DATAY0		0x34
#define AO_ADXL375_DATAY1		0x35
#define AO_ADXL375_DATAZ0		0x36
#define AO_ADXL375_DATAZ1		0x37
#define AO_ADXL375_FIFO_CTL		0x38
#define  AO_ADXL375_FIFO_CTL_FIFO_MODE		6
#define   AO_ADXL375_FIFO_CTL_FIFO_MODE_BYPASS		0
#define   AO_ADXL375_FIFO_CTL_FIFO_MODE_FIFO		1
#define   AO_ADXL375_FIFO_CTL_FIFO_MODE_STREAM		2
#define   AO_ADXL375_FIFO_CTL_FIFO_MODE_TRIGGER		3
#define  AO_ADXL375_FIFO_CTL_TRIGGER		5
#define  AO_ADXL375_FIFO_CTL_SAMPLES		0

#define AO_ADXL375_FIFO_STATUS		0x39

#define ADXL375_ACCEL_FULLSCALE		200

struct ao_adxl375_sample {
	int16_t	x;
	int16_t	y;
	int16_t	z;
};

extern struct ao_adxl375_sample	ao_adxl375_current;

#define ao_adxl375_accel_to_sample(accel) ((accel_t) (accel) * (4095.0f / (ADXL375_ACCEL_FULLSCALE * GRAVITY)))

void
ao_adxl375_init(void);

#endif /* _AO_ADXL375_H_ */
