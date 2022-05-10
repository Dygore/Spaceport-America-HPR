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

#ifndef _AO_LISP_OS_H_
#define _AO_LISP_OS_H_

#include "ao.h"

static inline int
ao_lisp_getc() {
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
ao_lisp_os_flush(void)
{
	flush();
}

static inline void
ao_lisp_abort(void)
{
	ao_panic(1);
}

static inline void
ao_lisp_os_led(int led)
{
	ao_led_set(led);
}

static inline void
ao_lisp_os_delay(int delay)
{
	ao_delay(AO_MS_TO_TICKS(delay));
}

#endif
