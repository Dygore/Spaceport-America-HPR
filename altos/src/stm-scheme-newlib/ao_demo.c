/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

#include "ao.h"
#include <ao_exti.h>
#include <ao_boot.h>
#include <ao_scheme.h>

static void scheme_cmd() {
	ao_scheme_read_eval_print();
}


const struct ao_cmds ao_demo_cmds[] = {
	{ scheme_cmd, "l\0Run scheme interpreter" },
	{ 0, NULL }
};

int
main(void)
{
	ao_clock_init();

	ao_task_init();

	ao_led_init(LEDS_AVAILABLE);
	ao_timer_init();
	ao_dma_init();
	ao_cmd_init();
	ao_usb_init();

	ao_cmd_register(&ao_demo_cmds[0]);

	ao_start_scheduler();
	return 0;
}
