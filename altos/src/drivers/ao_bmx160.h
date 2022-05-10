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

#ifndef _AO_BMX160_H_
#define _AO_BMX160_H_

#include <math.h>

struct ao_bmx160_sample {
	int16_t		mag_x;
	int16_t		mag_y;
	int16_t		mag_z;
	uint16_t	rhall;
	int16_t		gyr_x;
	int16_t		gyr_y;
	int16_t		gyr_z;
	int16_t		acc_x;
	int16_t		acc_y;
	int16_t		acc_z;
};

extern struct ao_bmx160_sample	ao_bmx160_current;

struct ao_bmx160_offset {
	int8_t		off_acc_x;
	int8_t		off_acc_y;
	int8_t		off_acc_z;
	int8_t		off_gyr_x;
	int8_t		off_gyr_y;
	int8_t		off_gyr_z;
	uint8_t		offset_6;
};

struct ao_bmm150_trim {
	int8_t		dig_x1;
	int8_t		dig_y1;
	int8_t		dig_x2;
	int8_t		dig_y2;
	uint16_t	dig_z1;
	int16_t		dig_z2;
	int16_t		dig_z3;
	int16_t		dig_z4;
	uint8_t		dig_xy1;
	int8_t		dig_xy2;
	uint16_t	dig_xyz1;
};

void
ao_bmx160_init(void);

#define BMX160_CHIPID			0x00
#define  BMX160_CHIPID_BMX160			0xd8
#define BMX160_ERR_REG			0x02
#define BMX160_PMU_STATUS		0x03
#define  BMX160_PMU_STATUS_MAG_IF_PMU_STATUS	0
#define   BMX160_PMU_STATUS_MAG_IF_PMU_STATUS_SUSPEND		0
#define   BMX160_PMU_STATUS_MAG_IF_PMU_STATUS_NORMAL		1
#define   BMX160_PMU_STATUS_MAG_IF_PMU_STATUS_LOW_POWER		2
#define  BMX160_PMU_STATUS_GYR_PMU_STATUS	2
#define   BMX160_PMU_STATUS_GYR_PMU_STATUS_SUSPEND		0
#define   BMX160_PMU_STATUS_GYR_PMU_STATUS_NORMAL		1
#define   BMX160_PMU_STATUS_GYR_PMU_STATUS_FAST_START_UP	3
#define   BMX160_PMU_STATUS_GYR_PMU_STATUS_MASK			3
#define  BMX160_PMU_STATUS_ACC_PMU_STATUS	4
#define   BMX160_PMU_STATUS_ACC_PMU_STATUS_SUSPEND		0
#define   BMX160_PMU_STATUS_ACC_PMU_STATUS_NORMAL		1
#define   BMX160_PMU_STATUS_ACC_PMU_STATUS_LOW_POWER		2
#define   BMX160_PMU_STATUS_ACC_PMU_STATUS_MASK			3
#define BMX160_DATA_0			0x04
#define BMX160_MAG_X_0_7		0x04
#define BMX160_MAG_X_8_15		0x05
#define BMX160_MAG_Y_0_7		0x06
#define BMX160_MAG_Y_8_15		0x07
#define BMX160_MAG_Z_0_7		0x08
#define BMX160_MAG_Z_8_15		0x09
#define BMX160_RHALL_0_7		0x0A
#define BMX160_RHALL_8_15		0x0B
#define BMX160_GYRO_X_0_7		0x0C
#define BMX160_GYRO_X_8_15		0x0D
#define BMX160_GYRO_Y_0_7		0x0E
#define BMX160_GYRO_Y_8_15		0x0F
#define BMX160_GYRO_Z_0_7		0x10
#define BMX160_GYRO_Z_8_15		0x11
#define BMX160_ACCEL_X_0_7		0x12
#define BMX160_ACCEL_X_8_15		0x13
#define BMX160_ACCEL_Y_0_7		0x14
#define BMX160_ACCEL_Y_8_15		0x15
#define BMX160_ACCEL_Z_0_7		0x16
#define BMX160_ACCEL_Z_8_15		0x17
#define BMX160_SENSORTIME_0_7		0x18
#define BMX160_SENSORTIME_8_15		0x19
#define BMX160_SENSORTIME_16_23		0x1A
#define BMX160_STATUS			0x1B
#define  BMX160_STATUS_GYR_SELF_TEST_OK		1
#define  BMX160_STATUS_MAG_MAN_OP		2
#define  BMX160_STATUS_FOC_RDY			3
#define  BMX160_STATUS_NVM_RDY			4
#define  BMX160_STATUS_DRDY_MAG			5
#define  BMX160_STATUS_DRDY_GYR			6
#define  BMX160_STATUS_DRDY_ACC			7
#define BMX160_INT_STATUS_0		0x1C-0x1F
#define BMX160_INT_STATUS_1		0x1D
#define BMX160_INT_STATUS_2		0x1E
#define BMX160_INT_STATUS_3		0x1F
#define BMX160_TEMPERATURE_0_7		0x20
#define BMX160_TEMPERATURE_8_15		0x21
#define BMX160_FIFO_LENGTH_0_7		0x22
#define BMX160_FIFO_LENGTH_8_15		0x23
#define BMX160_FIFO_DATA		0x24
#define BMX160_ACC_CONF			0x40
#define  BMX160_ACC_CONF_ACC_ODR		0
#define   BMX160_ACC_CONF_ACC_ODR_25_32			0x1
#define   BMX160_ACC_CONF_ACC_ODR_25_16			0x2
#define   BMX160_ACC_CONF_ACC_ODR_25_8			0x3
#define   BMX160_ACC_CONF_ACC_ODR_25_4			0x4
#define   BMX160_ACC_CONF_ACC_ODR_25_2			0x5
#define   BMX160_ACC_CONF_ACC_ODR_25			0x6
#define   BMX160_ACC_CONF_ACC_ODR_50			0x7
#define   BMX160_ACC_CONF_ACC_ODR_100			0x8
#define   BMX160_ACC_CONF_ACC_ODR_200			0x9
#define   BMX160_ACC_CONF_ACC_ODR_400			0xa
#define   BMX160_ACC_CONF_ACC_ODR_800			0xb
#define   BMX160_ACC_CONF_ACC_ODR_1600			0xc
#define  BMX160_ACC_CONF_ACC_BWP		4
#define   BMX160_ACC_CONF_ACC_BWP_NORMAL			0x2
#define   BMX160_ACC_CONF_ACC_BWP_OSR2				0x1
#define   BMX160_ACC_CONF_ACC_BWP_OSR4				0x0
#define  BMX160_ACC_CONF_ACC_US			7
#define BMX160_ACC_RANGE		0x41
#define  BMX160_ACC_RANGE_2G			0x3
#define  BMX160_ACC_RANGE_4G			0x5
#define  BMX160_ACC_RANGE_8G			0x8
#define  BMX160_ACC_RANGE_16G			0xc
#define BMX160_GYR_CONF			0x42
#define  BMX160_GYR_CONF_GYR_ODR		0
#define   BMX160_GYR_CONF_GYR_ODR_25			0x6
#define   BMX160_GYR_CONF_GYR_ODR_50			0x7
#define   BMX160_GYR_CONF_GYR_ODR_100			0x8
#define   BMX160_GYR_CONF_GYR_ODR_200			0x9
#define   BMX160_GYR_CONF_GYR_ODR_400			0xa
#define   BMX160_GYR_CONF_GYR_ODR_800			0xb
#define   BMX160_GYR_CONF_GYR_ODR_1600			0xc
#define   BMX160_GYR_CONF_GYR_ODR_3200			0xd
#define  BMX160_GYR_CONF_GYR_BWP		4
#define   BMX160_GYR_CONF_GYR_BWP_NORMAL			0x2
#define   BMX160_GYR_CONF_GYR_BWP_OSR2				0x1
#define   BMX160_GYR_CONF_GYR_BWP_OSR4				0x0
#define BMX160_GYR_RANGE		0x43
#define  BMX160_GYR_RANGE_2000					0x0
#define  BMX160_GYR_RANGE_1000					0x1
#define  BMX160_GYR_RANGE_500					0x2
#define  BMX160_GYR_RANGE_250					0x3
#define  BMX160_GYR_RANGE_125					0x4
#define BMX160_MAG_CONF			0x44
#define  BMX160_MAG_CONF_MAG_ODR			0
#define   BMX160_MAG_CONF_MAG_ODR_25_32				0x1
#define   BMX160_MAG_CONF_MAG_ODR_25_16				0x2
#define   BMX160_MAG_CONF_MAG_ODR_25_8				0x3
#define   BMX160_MAG_CONF_MAG_ODR_25_4				0x4
#define   BMX160_MAG_CONF_MAG_ODR_25_2				0x5
#define   BMX160_MAG_CONF_MAG_ODR_25				0x6
#define   BMX160_MAG_CONF_MAG_ODR_50				0x7
#define   BMX160_MAG_CONF_MAG_ODR_100				0x8
#define   BMX160_MAG_CONF_MAG_ODR_200				0x9
#define   BMX160_MAG_CONF_MAG_ODR_400				0xa
#define   BMX160_MAG_CONF_MAG_ODR_800				0xb
#define BMX160_FIFO_DOWNS		0x45
#define BMX160_FIFO_CONFIG_0		0x46
#define BMX160_FIFO_CONFIG_1		0x47
#define BMX160_MAG_IF_0			0x4C
#define  BMX160_MAG_IF_0_MAG_RD_BURST		0
#define  BMX160_MAG_IF_0_MAG_OFFSET		2
#define  BMX160_MAG_IF_0_MAG_MANUAL_EN		7
#define BMX160_MAG_IF_1			0x4D
#define BMX160_MAG_IF_2			0x4E
#define BMX160_MAG_IF_3			0x4F
#define BMX160_INT_EN			0x50-0x52
#define BMX160_INT_OUT_CTRL		0x53
#define BMX160_INT_LATCH		0x54
#define BMX160_INT_MAP			0x55-0x57
#define BMX160_INT_DATA			0x58-0x59
#define BMX160_INT_LOWHIGH		0x5A-0x5E
#define BMX160_INT_MOTION		0x5F-0x62
#define BMX160_INT_TAP			0x63-0x64
#define BMX160_INT_ORIENT		0x65-0x66
#define BMX160_INT_FLAT			0x67-0x68
#define BMX160_FOC_CONF			0x69
#define BMX160_CONF			0x6A
#define BMX160_IF_CONF			0x6B
#define  BMX160_IF_CONF_IF_MODE			4
#define  BMX160_IF_CONF_IF_MODE_AUTO_MAG		0x02
#define BMX160_PMU_TRIGGER		0x6C
#define BMX160_SELF_TEST		0x6D
#define BMX160_NV_CONF			0x70
#define  BMX160_NV_CONF_SPI_EN			0
#define  BMX160_NV_CONF_I2C_WDT_SEL		1
#define  BMX160_NV_CONF_I2C_WDT_EN		2
#define BMX160_OFFSET			0x71-0x77
#define BMX160_STEP_CNT			0x78-0x79
#define BMX160_STEP_CONF		0x7A-0x7B
#define BMX160_CMD			0x7E
#define  BMX160_CMD_START_FOC			0x03
#define  BMX160_CMD_ACC_SET_PMU_MODE(n)		(0x10 | (n))
#define  BMX160_CMD_GYR_SET_PMU_MODE(n)		(0x14 | (n))
#define  BMX160_CMD_MAG_IF_SET_PMU_MODE(n)	(0x18 | (n))
#define  BMX160_CMD_PROG_NVM			0xa0
#define  BMX160_CMD_FIFO_FLUSH			0xb0
#define  BMX160_CMD_INT_RESET			0xb1
#define  BMX160_CMD_SOFTRESET			0xb6
#define  BMX160_CMD_STEP_CNT_CLR		0xb2

#define BMM150_CHIP_ID				0x40
#define BMM150_DATA_X_0_4			0x42
#define BMM150_DATA_X_5_12			0x43
#define BMM150_DATA_Y_0_4			0x44
#define BMM150_DATA_Y_5_12			0x45
#define BMM150_DATA_Z_0_6			0x46
#define BMM150_DATA_Z_7_14			0x47
#define BMM150_RHALL_0_5			0x48
#define BMM150_RHALL_6_13			0x49
#define BMM150_INT_STATUS			0x4a
#define BMM150_POWER_MODE			0x4b
#define  BMM150_POWER_MODE_SOFT_RESET_HI		7
#define  BMM150_POWER_MODE_SPI3EN			2
#define  BMM150_POWER_MODE_SOFT_RESET_LO		1
#define  BMM150_POWER_MODE_POWER_CONTROL		0
#define BMM150_CONTROL				0x4c
#define  BMM150_CONTROL_ADV_ST_1			7
#define  BMM150_CONTROL_ADV_ST_0			6
#define  BMM150_CONTROL_DATA_RATE			3
#define   BMM150_CONTROL_DATA_RATE_10				0
#define   BMM150_CONTROL_DATA_RATE_2				1
#define   BMM150_CONTROL_DATA_RATE_6				2
#define   BMM150_CONTROL_DATA_RATE_8				3
#define   BMM150_CONTROL_DATA_RATE_15				4
#define   BMM150_CONTROL_DATA_RATE_20				5
#define   BMM150_CONTROL_DATA_RATE_25				6
#define   BMM150_CONTROL_DATA_RATE_30				7
#define  BMM150_CONTROL_OP_MODE				1
#define   BMM150_CONTROL_OP_MODE_NORMAL				0
#define   BMM150_CONTROL_OP_MODE_FORCED				1
#define   BMM150_CONTROL_OP_MODE_SLEEP				3
#define  BMM150_CONTROL_SELF_TEST			0
#define BMM150_INT_EN				0x4d
#define BMM150_INT_CONF				0x4e
#define  BMM150_INT_CONF_X_DISABLE			3
#define  BMM150_INT_CONF_Y_DISABLE			4
#define  BMM150_INT_CONF_Z_DISABLE			5
#define BMM150_LOW_THRESHOLD			0x4f
#define BMM150_HIGH_THRESHOLD			0x50
#define BMM150_REPXY				0x51
#define  BMM150_REPXY_VALUE(n)				(((n)-1) >> 1)
#define BMM150_REPZ				0x52
#define  BMM150_REPZ_VALUE(n)				((n) -1)

/* Trim Extended Registers */
#define BMM150_DIG_X1                        0x5D
#define BMM150_DIG_Y1                        0x5E
#define BMM150_DIG_Z4_LSB                    0x62
#define BMM150_DIG_Z4_MSB                    0x63
#define BMM150_DIG_X2                        0x64
#define BMM150_DIG_Y2                        0x65
#define BMM150_DIG_Z2_LSB                    0x68
#define BMM150_DIG_Z2_MSB                    0x69
#define BMM150_DIG_Z1_LSB                    0x6A
#define BMM150_DIG_Z1_MSB                    0x6B
#define BMM150_DIG_XYZ1_LSB                  0x6C
#define BMM150_DIG_XYZ1_MSB                  0x6D
#define BMM150_DIG_Z3_LSB                    0x6E
#define BMM150_DIG_Z3_MSB                    0x6F
#define BMM150_DIG_XY2                       0x70
#define BMM150_DIG_XY1                       0x71

#define BMM150_XYAXES_FLIP_OVERFLOW_ADCVAL   -4096
#define BMM150_ZAXIS_HALL_OVERFLOW_ADCVAL    -16384
#define BMM150_OVERFLOW_OUTPUT               -32768
#define BMM150_NEGATIVE_SATURATION_Z         -32767
#define BMM150_POSITIVE_SATURATION_Z         32767

#define BMX160_GYRO_FULLSCALE	((float) 2000 * M_PI/180.0)

static inline float
ao_bmx160_gyro(float sensor) {
	return sensor * ((float) (BMX160_GYRO_FULLSCALE / 32767.0));
}

#define BMX160_ACCEL_FULLSCALE	16

static inline float
ao_bmx160_accel(int16_t sensor) {
	return (float) sensor * ((float) (BMX160_ACCEL_FULLSCALE * GRAVITY / 32767.0));
}

#define ao_bmx_accel_to_sample(accel) ((accel_t) (accel) * (32767.0f / (BMX160_ACCEL_FULLSCALE * GRAVITY)))

#endif /* _BMX160_H_ */
