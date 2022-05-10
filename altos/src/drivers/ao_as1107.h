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

#ifndef _AO_AS1107_H_
#define _AO_AS1107_H_

#define AO_AS1107_NO_OP		0x00
#define AO_AS1107_DIGIT(n)	(0x01 + (n))
#define AO_AS1107_DECODE_MODE	0x09
#define AO_AS1107_INTENSITY	0x0a
#define AO_AS1107_SCAN_LIMIT	0x0b
#define AO_AS1107_SHUTDOWN	0x0c
#define  AO_AS1107_SHUTDOWN_SHUTDOWN_RESET	0x00
#define  AO_AS1107_SHUTDOWN_SHUTDOWN_NOP	0x80
#define  AO_AS1107_SHUTDOWN_NORMAL_RESET	0x01
#define  AO_AS1107_SHUTDOWN_NORMAL_NOP		0x81

#define AO_AS1107_FEATURE	0x0e
#define  AO_AS1107_FEATURE_CLK_EN	0	/* external clock enable */
#define  AO_AS1107_FEATURE_REG_RES	1
#define  AO_AS1107_FEATURE_DECODE_SEL	2	/* select HEX decode */
#define  AO_AS1107_FEATURE_SPI_EN	3
#define  AO_AS1107_FEATURE_BLINK_EN	4
#define  AO_AS1107_FEATURE_BLINK_FREQ	5
#define  AO_AS1107_FEATURE_SYNC		6
#define  AO_AS1107_FEATURE_BLINK_START	7
#define AO_AS1107_DISPLAY_TEST	0x0f

void ao_as1107_init(void);

void
ao_as1107_write(uint8_t start, uint8_t count, uint8_t *values);

void
ao_as1107_write_8(uint8_t start, uint8_t value);

void
ao_as1107_write_16(uint8_t start, uint16_t value);

#ifndef AO_AS1107_DECODE
#error "must define AO_AS1107_DECODE"
#endif

#ifndef AO_AS1107_NUM_DIGITS
#error "must define AO_AS1107_NUM_DIGITS"
#endif

#endif /* _AO_AS1107_H_ */
