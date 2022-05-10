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

#ifndef _AO_MMC5983_H_
#define _AO_MMC5983_H_

#define MMC5983_READ		0x80

#define MMC5983_I2C_ADDR	0x60

#define MMC5983_CONFIG_A	0

#define  MMC5983_CONFIG_A_MA		5
#define  MMC5983_CONFIG_A_MA_1			0
#define  MMC5983_CONFIG_A_MA_2			1
#define  MMC5983_CONFIG_A_MA_4			2
#define  MMC5983_CONFIG_A_MA_8			3
#define  MMC5983_CONFIG_A_MA_MASK		3

#define  MMC5983_CONFIG_A_DO		2
#define   MMC5983_CONFIG_A_DO_0_75		0
#define   MMC5983_CONFIG_A_DO_1_5		1
#define   MMC5983_CONFIG_A_DO_3			2
#define   MMC5983_CONFIG_A_DO_7_5		3
#define   MMC5983_CONFIG_A_DO_15		4
#define   MMC5983_CONFIG_A_DO_30		5
#define   MMC5983_CONFIG_A_DO_75		6
#define   MMC5983_CONFIG_A_DO_MASK		7

#define MMC5983_CONFIG_A_MS		0
#define  MMC5983_CONFIG_A_MS_NORMAL		0
#define  MMC5983_CONFIG_A_MS_POSITIVE_BIAS	1
#define  MMC5983_CONFIG_A_MS_NEGATIVE_BIAS	2
#define  MMC5983_CONFIG_A_MS_MASK		3

#define MMC5983_CONFIG_B	1

#define MMC5983_CONFIG_B_GN		5
#define  MMC5983_CONFIG_B_GN_0_88		0
#define  MMC5983_CONFIG_B_GN_1_3		1
#define  MMC5983_CONFIG_B_GN_1_9		2
#define  MMC5983_CONFIG_B_GN_2_5		3
#define  MMC5983_CONFIG_B_GN_4_0		4
#define  MMC5983_CONFIG_B_GN_4_7		5
#define  MMC5983_CONFIG_B_GN_5_6		6
#define  MMC5983_CONFIG_B_GN_8_1		7
#define  MMC5983_CONFIG_B_GN_MASK		7

#define MMC5983_MODE		2
#define  MMC5983_MODE_CONTINUOUS	0
#define  MMC5983_MODE_SINGLE		1
#define  MMC5983_MODE_IDLE		2

#define MMC5983_X_OUT_0		0
#define MMC5983_X_OUT_1		1
#define MMC5983_Y_OUT_0		2
#define MMC5983_Y_OUT_1		3
#define MMC5983_Z_OUT_0		4
#define MMC5983_Z_OUT_1		5
#define MMC5983_XYZ_OUT_2	6
#define MMC5983_T_OUT		7

#define MMC5983_STATUS		8
# define MMC5983_STATUS_OTP_READ_DONE		4
# define MMC5983_STATUS_MEAS_T_DONE		1
# define MMC5983_STATUS_MEAS_M_DONE		0

#define MMC5983_CONTROL_0	9
# define MMC5983_CONTROL_0_OTP_READ		6
# define MMC5983_CONTROL_0_AUTO_SR_EN		5
# define MMC5983_CONTROL_0_RESET		4
# define MMC5983_CONTROL_0_SET			3
# define MMC5983_CONTROL_0_INT_MEAS_DONE_EN	2
# define MMC5983_CONTROL_0_TM_T			1
# define MMC5983_CONTROL_0_TM_M			0

#define MMC5983_CONTROL_1	0xa
# define MMC5983_CONTROL_1_SW_RST		7
# define MMC5983_CONTROL_1_YZ_INHIBIT		3
# define MMC5983_CONTROL_1_X_INHIBIT		2
# define MMC5983_CONTROL_1_BW			0
# define MMC5983_CONTROL_1_BW_100			0
# define MMC5983_CONTROL_1_BW_200			1
# define MMC5983_CONTROL_1_BW_400			2
# define MMC5983_CONTROL_1_BW_800			3
#define MMC5983_CONTROL_2	0xb
# define MMC5983_CONTROL_2_EN_PRD_SET		7
# define MMC5983_CONTROL_2_PRD_SET		4
# define MMC5983_CONTROL_2_PRD_SET_1			0
# define MMC5983_CONTROL_2_PRD_SET_25			1
# define MMC5983_CONTROL_2_PRD_SET_75			2
# define MMC5983_CONTROL_2_PRD_SET_100			3
# define MMC5983_CONTROL_2_PRD_SET_250			4
# define MMC5983_CONTROL_2_PRD_SET_500			5
# define MMC5983_CONTROL_2_PRD_SET_1000			6
# define MMC5983_CONTROL_2_PRD_SET_2000			7
# define MMC5983_CONTROL_2_CMM_EN		3
# define MMC5983_CONTROL_2_CM_FREQ		0
# define MMC5983_CONTROL_2_CM_FREQ_OFF			0
# define MMC5983_CONTROL_2_CM_FREQ_1HZ			1
# define MMC5983_CONTROL_2_CM_FREQ_10HZ			2
# define MMC5983_CONTROL_2_CM_FREQ_20HZ			3
# define MMC5983_CONTROL_2_CM_FREQ_50HZ			4
# define MMC5983_CONTROL_2_CM_FREQ_100HZ		5
# define MMC5983_CONTROL_2_CM_FREQ_200HZ		6
# define MMC5983_CONTROL_2_CM_FREQ_1000HZ		7

#define MMC5983_CONTROL_3	0xc
#define  MMC5983_CONTROL_3_SPI_3W		6
#define  MMC5983_CONTROL_3_ST_ENM		2
#define  MMC5983_CONTROL_3_ST_ENP		1

#define MMC5983_PRODUCT_ID	0x2f
#define MMC5983_PRODUCT_ID_PRODUCT_I2C	0x30
#define MMC5983_PRODUCT_ID_PRODUCT_SPI	0x31


struct ao_mmc5983_sample {
	int16_t		x, y, z;
};

struct ao_mmc5983_raw {
	uint8_t		addr;
	uint8_t		x0;
	uint8_t		x1;
	uint8_t		y0;
	uint8_t		y1;
	uint8_t		z0;
	uint8_t		z1;
};

extern struct ao_mmc5983_sample	ao_mmc5983_current;

void
ao_mmc5983_init(void);

#endif /* _AO_MMC5983_H_ */
