/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_scheme.h>

static void scheme_cmd(void) {
	ao_scheme_read_eval_print(stdin, stdout, false);
}

static const struct ao_cmds blink_cmds[] = {
	{ scheme_cmd,	"l\0Run scheme interpreter" },
	{ 0, 0 }
};

int
_ao_scheme_getc(void)
{
	static uint8_t	at_eol;
	int c;

	if (at_eol) {
		ao_cmd_readline(ao_scheme_read_list ? "Λ " : "λ ");
		at_eol = 0;
	}
	c = (unsigned char) ao_cmd_lex();
	if (c == '\n')
		at_eol = 1;
	return c;
}

void main(void)
{
#ifdef LEDS_AVAILABLE
	ao_led_init();
#endif
	ao_clock_init();
	ao_timer_init();
	ao_usb_init();
	ao_cmd_init();
	ao_cmd_register(blink_cmds);
	ao_cmd();
}


