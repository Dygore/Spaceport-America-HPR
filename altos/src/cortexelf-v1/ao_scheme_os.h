/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#ifndef _AO_SCHEME_OS_H_
#define _AO_SCHEME_OS_H_

#include "ao.h"

#define AO_SCHEME_POOL_TOTAL		16384
#define AO_SCHEME_SAVE			1

#ifndef __BYTE_ORDER
#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#define __BYTE_ORDER	__LITTLE_ENDIAN
#endif

static inline int
ao_scheme_getc() {
	static uint8_t	at_eol;
	int c;

	if (at_eol) {
		ao_cmd_readline();
		at_eol = 0;
	}
	c = ao_cmd_lex();
	if (c == '\n')
		at_eol = 1;
	return c;
}

static inline void
ao_scheme_os_flush(void)
{
	flush();
}

static inline void
ao_scheme_abort(void)
{
	ao_panic(1);
}

static inline void
ao_scheme_os_led(int led)
{
	(void) led;
}

#define AO_SCHEME_JIFFIES_PER_SECOND	AO_HERTZ

static inline void
ao_scheme_os_delay(int delay)
{
	ao_delay(delay);
}

static inline int
ao_scheme_os_jiffy(void)
{
	return ao_tick_count;
}

#endif
