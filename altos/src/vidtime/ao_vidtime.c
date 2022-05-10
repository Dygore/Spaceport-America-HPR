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
#include <ao_exti.h>

static uint8_t	sensor_value;
static uint8_t	vidtime_monitor;

static void
vidtime(void)
{
	uint8_t	old = 0, got;

	ao_exti_enable(AO_SENSOR_PORT, AO_SENSOR_PIN);
	for (;;) {
		while (!vidtime_monitor)
			ao_sleep(&vidtime_monitor);
		while ((got = sensor_value) == old)
			ao_sleep(&sensor_value);
		printf("%d\n", got);
		flush();
		old = got;
	}
}

static void
sensor_interrupt(void)
{
	sensor_value = ao_gpio_get(AO_SENSOR_PORT, AO_SENSOR_PIN, foo);
	ao_wakeup(&sensor_value);
}

static struct ao_task	vidtime_task;

static void
ao_init_vidtime(void)
{
	ao_enable_port(AO_SENSOR_PORT);
	ao_exti_setup(AO_SENSOR_PORT, AO_SENSOR_PIN,
		      AO_EXTI_MODE_RISING|
		      AO_EXTI_MODE_FALLING|
		      AO_EXTI_MODE_PULL_NONE|
		      AO_EXTI_PRIORITY_MED,
		      sensor_interrupt);
	ao_add_task(&vidtime_task, vidtime, "vidtime");
}

static void
ao_set_vidtime(void)
{
	uint16_t r = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success) {
		vidtime_monitor = r != 0;
		ao_wakeup(&vidtime_monitor);
	}
}

const struct ao_cmds	ao_vidtime_cmds[] = {
	{ ao_set_vidtime, "V <0 off, 1 on>\0Enable/disable timing monitor" },
	{ 0, NULL }
};

void main(void)
{
	ao_clock_init();

	ao_task_init();

	ao_timer_init();

	ao_dma_init();

	ao_init_vidtime();

	ao_usb_init();

	ao_cmd_init();

	ao_cmd_register(&ao_vidtime_cmds[0]);

	ao_start_scheduler();
}
