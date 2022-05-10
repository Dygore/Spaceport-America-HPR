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
#include <ao_lisp.h>
#include <ao_beep.h>

static void lisp_cmd() {
	ao_lisp_read_eval_print();
}

static void beep() {
	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
}

static const struct ao_cmds blink_cmds[] = {
	{ lisp_cmd,	"l\0Run lisp interpreter" },
	{ beep,		"b\0Beep" },
	{ 0, 0 }
};

void main(void)
{
	ao_led_init(LEDS_AVAILABLE);
	ao_clock_init();
	ao_task_init();
	ao_timer_init();
	ao_dma_init();
	ao_usb_init();
	ao_serial_init();
	ao_beep_init();
	ao_cmd_init();
	ao_cmd_register(blink_cmds);
	ao_start_scheduler();
}


