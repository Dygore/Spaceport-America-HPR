/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_lco.h>
#include <ao_event.h>
#include <ao_button.h>
#include <ao_lco_func.h>
#include <ao_radio_cmac.h>

static uint8_t	ao_lco_suspended;

static void
ao_lco_suspend(void)
{
	if (!ao_lco_suspended) {
		PRINTD("suspend\n");
		ao_lco_suspended = 1;
		ao_lco_armed = 0;
		ao_wakeup(&ao_pad_query);
	}
}

static void
ao_lco_wakeup(void)
{
	if (ao_lco_suspended) {
		ao_lco_suspended = 0;
		ao_wakeup(&ao_lco_suspended);
	}
}

void
ao_lco_show_pad(uint8_t pad)
{
	(void) pad;
}

void
ao_lco_show_box(uint16_t box)
{
	(void) box;
}

void
ao_lco_show(void)
{
}

static void
ao_lco_input(void)
{
	static struct ao_event	event;
	uint8_t	timeout;

	for (;;) {
		if (ao_config.pad_idle && !ao_lco_suspended) {
			timeout = ao_event_get_for(&event, AO_SEC_TO_TICKS(ao_config.pad_idle));
			if (timeout) {
				ao_lco_suspend();
				continue;
			}
		} else {
			ao_event_get(&event);
		}
		ao_lco_wakeup();
		PRINTD("event type %d unit %d value %ld\n",
		       event.type, event.unit, (long) event.value);
		switch (event.type) {
		case AO_EVENT_BUTTON:
			switch (event.unit) {
			case AO_BUTTON_BOX:
				ao_lco_set_box((uint16_t) event.value);
				ao_lco_set_armed(0);
				break;
			case AO_BUTTON_ARM:
				ao_lco_set_armed((uint8_t) event.value);
				break;
			case AO_BUTTON_FIRE:
				if (ao_lco_armed)
					ao_lco_set_firing((uint8_t) event.value);
				break;
			}
			break;
		}
	}
}

static struct ao_task ao_lco_input_task;
static struct ao_task ao_lco_monitor_task;
static struct ao_task ao_lco_arm_warn_task;
static struct ao_task ao_lco_igniter_status_task;

static void
ao_lco_main(void)
{
	ao_config_get();
	ao_lco_set_box(ao_button_get(AO_BUTTON_BOX));
	ao_add_task(&ao_lco_input_task, ao_lco_input, "lco input");
	ao_add_task(&ao_lco_arm_warn_task, ao_lco_arm_warn, "lco arm warn");
	ao_add_task(&ao_lco_igniter_status_task, ao_lco_igniter_status, "lco igniter status");
	ao_led_on((uint16_t) ~0);
	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	ao_led_off((uint16_t) ~0);
	ao_lco_monitor();
}

#if DEBUG
static void
ao_lco_set_debug(void)
{
	uint32_t r = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success)
		ao_lco_debug = r != 0;
}

const struct ao_cmds ao_lco_cmds[] = {
	{ ao_lco_set_debug,	"D <0 off, 1 on>\0Debug" },
	{ 0, NULL }
};
#endif

void
ao_lco_init(void)
{
	ao_add_task(&ao_lco_monitor_task, ao_lco_main, "lco monitor");
#if DEBUG
	ao_cmd_register(&ao_lco_cmds[0]);
#endif
}
