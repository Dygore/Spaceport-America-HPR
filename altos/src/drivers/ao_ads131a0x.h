/*
 * Copyright © 2019 Bdale Garbee <bdale@gag.com>
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

#ifndef _AO_ADS131A0X_H_
#define _AO_ADS131A0X_H_

/* control commands */
#define AO_ADS131A0X_NOP		0x00
#define AO_ADS131A0X_WAKEUP		0x02
#define AO_ADS131A0X_POWERDOWN		0x04
#define AO_ADS131A0X_RESET		0x06
#define AO_ADS131A0X_START		0x08
#define AO_ADS131A0X_STOP		0x0a

/* calibration commands */
#define AO_ADS131A0X_SYOCAL		0x16
#define AO_ADS131A0X_SYGCAL		0x17
#define AO_ADS131A0X_SFOCAL		0x19

/* data read command */
#define AO_ADS131A0X_RDATA		0x12

/* register read and write commands */
#define AO_ADS131A0X_RREG		0x20
#define AO_ADS131A0X_WREG		0x40

/* configuration register map */
#define AO_ADS131A0X_ID			0x00
#define AO_ADS131A0X_ID_ADS131A08		0x00
#define AO_ADS131A0X_ID_ADS131A06		0x01
#define AO_ADS131A0X_STATUS		0x01
#define AO_ADS131A0X_INPMUX		0x02
#define AO_ADS131A0X_PGA		0x03
#define AO_ADS131A0X_DATARATE		0x04
#define AO_ADS131A0X_REF		0x05
#define AO_ADS131A0X_IDACMAG		0x06
#define AO_ADS131A0X_IDACMUX		0x07
#define AO_ADS131A0X_VBIAS		0x08
#define AO_ADS131A0X_SYS		0x09
#define AO_ADS131A0X_OFCAL0		0x0a
#define AO_ADS131A0X_OFCAL1		0x0b
#define AO_ADS131A0X_OFCAL2		0x0c
#define AO_ADS131A0X_FSCAL0		0x0d
#define AO_ADS131A0X_FSCAL1		0x0e
#define AO_ADS131A0X_FSCAL2		0x0f
#define AO_ADS131A0X_GPIODAT		0x10
#define AO_ADS131A0X_GPIOCON		0x11

struct ao_ads131a0x_sample {
	int32_t	ain[AO_ADS131A0X_CHANNELS];
};

extern struct ao_ads131a0x_sample	ao_ads131a0x_current;

void
ao_ads131a0x_init(void);

#endif /* _AO_ADS131A0X_H_ */
