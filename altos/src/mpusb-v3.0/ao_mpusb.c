/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_serial.h>

static struct ao_task ao_report_task;

static void
ao_report(void)
{
	int	c;
	ao_up_set_speed(AO_SERIAL_SPEED_9600);
	for (;;) {
		ao_arch_block_interrupts();
		c = _ao_up_pollchar();
		ao_arch_release_interrupts();
		if (c == AO_READ_AGAIN) {
			flush();
			c = ao_up_getchar();
		}
		putchar(c);
	}
}

int
main(void)
{
	ao_clock_init();

	ao_task_init();

	ao_timer_init();

	ao_serial_init();

	ao_dma_init();

	ao_cmd_init();

	ao_usb_init();

	ao_add_task(&ao_report_task, ao_report, "report");

	ao_start_scheduler();
	return 0;
}
