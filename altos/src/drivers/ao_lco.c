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
#include <ao_seven_segment.h>
#include <ao_quadrature.h>
#include <ao_lco_func.h>
#include <ao_radio_cmac.h>
#if HAS_ADC_SINGLE
#include <ao_adc_single.h>
#endif

#define AO_LCO_PAD_DIGIT	0
#define AO_LCO_BOX_DIGIT_1	1
#define AO_LCO_BOX_DIGIT_10	2

#define AO_LCO_DRAG_RACE_START_TIME	AO_SEC_TO_TICKS(5)
#define AO_LCO_DRAG_RACE_STOP_TIME	AO_SEC_TO_TICKS(2)

#define AO_LCO_BOX_DRAG		0x1000

/* UI values */
static AO_TICK_TYPE	ao_lco_fire_tick;
static uint8_t	ao_lco_fire_down;

static uint8_t	ao_lco_display_mutex;

void
ao_lco_show_pad(uint8_t pad)
{
	ao_mutex_get(&ao_lco_display_mutex);
	ao_seven_segment_set(AO_LCO_PAD_DIGIT, (uint8_t) (pad | (ao_lco_drag_race << 4)));
	ao_mutex_put(&ao_lco_display_mutex);
}

#define SEVEN_SEGMENT_d		((0 << 0) |	\
				 (0 << 1) |	\
				 (1 << 2) |	\
				 (1 << 3) |	\
				 (1 << 4) |	\
				 (1 << 5) |	\
				 (1 << 6))


#define SEVEN_SEGMENT_r		((0 << 0) |	\
				 (0 << 1) |	\
				 (0 << 2) |	\
				 (1 << 3) |	\
				 (1 << 4) |	\
				 (0 << 5) |	\
				 (0 << 6))

void
ao_lco_show_box(uint16_t box)
{
	ao_mutex_get(&ao_lco_display_mutex);
	if (box == AO_LCO_BOX_DRAG) {
		ao_seven_segment_direct(AO_LCO_BOX_DIGIT_10, (uint8_t) (SEVEN_SEGMENT_d | (ao_lco_drag_race << 7)));
		ao_seven_segment_direct(AO_LCO_BOX_DIGIT_1, (uint8_t) (SEVEN_SEGMENT_r | (ao_lco_drag_race << 7)));
	} else {
		ao_seven_segment_set(AO_LCO_BOX_DIGIT_1, (uint8_t) (box % 10 | (ao_lco_drag_race << 4)));
		ao_seven_segment_set(AO_LCO_BOX_DIGIT_10, (uint8_t) (box / 10 | (ao_lco_drag_race << 4)));
	}
	ao_mutex_put(&ao_lco_display_mutex);
}

static void
ao_lco_show_voltage(uint16_t decivolts)
{
	uint8_t	tens, ones, tenths;

	tenths = (uint8_t) (decivolts % 10);
	ones = (uint8_t) ((decivolts / 10) % 10);
	tens = (uint8_t) ((decivolts / 100) % 10);
	ao_mutex_get(&ao_lco_display_mutex);
	ao_seven_segment_set(AO_LCO_PAD_DIGIT, tenths);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_1, ones | 0x10);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_10, tens);
	ao_mutex_put(&ao_lco_display_mutex);
}

void
ao_lco_show(void)
{
	if (ao_lco_pad == AO_LCO_PAD_VOLTAGE && ao_lco_box != AO_LCO_BOX_DRAG) {
		ao_lco_show_voltage(ao_pad_query.battery);
	} else {
		if (ao_lco_box == AO_LCO_BOX_DRAG)
			ao_lco_show_pad(ao_lco_drag_race);
		else
			ao_lco_show_pad(ao_lco_pad);
		ao_lco_show_box(ao_lco_box);
	}
}

uint8_t
ao_lco_box_present(uint16_t box)
{
	if (box == AO_LCO_BOX_DRAG)
		return 1;

	if (box >= AO_PAD_MAX_BOXES)
		return 0;
	return (ao_lco_box_mask[AO_LCO_MASK_ID(box)] >> AO_LCO_MASK_SHIFT(box)) & 1;
}

static struct ao_task	ao_lco_drag_task;
static uint8_t		ao_lco_drag_active;

static AO_TICK_TYPE
ao_lco_drag_button_check(AO_TICK_TYPE now, AO_TICK_TYPE delay)
{
	AO_TICK_TYPE	button_delay = ~0UL;

	/*
	 * Check to see if the button has been held down long enough
	 * to switch in/out of drag race mode
	 */
	if (ao_lco_fire_down) {
		if (ao_lco_drag_race) {
			if ((AO_TICK_SIGNED) (now - ao_lco_fire_tick) >= (AO_TICK_SIGNED) AO_LCO_DRAG_RACE_STOP_TIME) {
				ao_lco_drag_disable();
				ao_lco_fire_down = 0;
			}
			else
				button_delay = ao_lco_fire_tick + AO_LCO_DRAG_RACE_STOP_TIME - now;
		} else {
			if ((AO_TICK_SIGNED) (now - ao_lco_fire_tick) >= (AO_TICK_SIGNED) AO_LCO_DRAG_RACE_START_TIME) {
				ao_lco_drag_enable();
				ao_lco_fire_down = 0;
			}
			else
				button_delay = ao_lco_fire_tick + AO_LCO_DRAG_RACE_START_TIME - now;
		}
		if (delay > button_delay)
			delay = button_delay;
	}
	return delay;
}

static void
ao_lco_drag_monitor(void)
{
	AO_TICK_TYPE	delay = ~0UL;
	AO_TICK_TYPE	now;

	ao_beep_for(AO_BEEP_MID, AO_MS_TO_TICKS(200));
	for (;;) {
		PRINTD("Drag monitor count %d active %d delay %lu\n",
		       ao_lco_drag_beep_count, ao_lco_drag_active, (unsigned long) delay);
		if (delay == (AO_TICK_TYPE) ~0)
			ao_sleep(&ao_lco_drag_beep_count);
		else
			ao_sleep_for(&ao_lco_drag_beep_count, delay);

		delay = ~0UL;
		if (!ao_lco_drag_active)
			continue;

		now = ao_time();
		delay = ao_lco_drag_button_check(now, delay);
		delay = ao_lco_drag_warn_check(now, delay);
		delay = ao_lco_drag_beep_check(now, delay);

		/* check to see if there's anything left to do here */
		if (!ao_lco_fire_down && !ao_lco_drag_race && !ao_lco_drag_beep_count) {
			delay = ~0UL;
			ao_lco_drag_active = 0;
		}
	}
}

static void
ao_lco_step_box(int8_t dir)
{
	int32_t new_box = (int32_t) ao_lco_box;
	do {
		if (new_box == AO_LCO_BOX_DRAG) {
			if (dir < 0)
				new_box = ao_lco_max_box;
			else
				new_box = ao_lco_min_box;
		} else {
			new_box += dir;
			if (new_box > ao_lco_max_box)
				new_box = AO_LCO_BOX_DRAG;
			else if (new_box < ao_lco_min_box)
				new_box = AO_LCO_BOX_DRAG;
		}
		if (new_box == (int32_t) ao_lco_box)
			break;
	} while (!ao_lco_box_present((uint16_t) new_box));
	ao_lco_set_box((uint16_t) new_box);
}

static void
ao_lco_input(void)
{
	static struct ao_event	event;

	for (;;) {
		ao_event_get(&event);
		PRINTD("event type %d unit %d value %ld\n",
		       event.type, event.unit, (long) event.value);
		switch (event.type) {
		case AO_EVENT_QUADRATURE:
			switch (event.unit) {
			case AO_QUADRATURE_PAD:
				if (!ao_lco_armed)
					ao_lco_step_pad((int8_t) event.value);
				break;
			case AO_QUADRATURE_BOX:
				if (!ao_lco_armed)
					ao_lco_step_box((int8_t) event.value);
				break;
			}
			break;
		case AO_EVENT_BUTTON:
			switch (event.unit) {
			case AO_BUTTON_ARM:
				ao_lco_set_armed((uint8_t) event.value);
				break;
			case AO_BUTTON_FIRE:
				if (ao_lco_armed) {
					ao_lco_fire_down = 0;
					ao_lco_set_firing((uint8_t) event.value);
				} else {
					if (event.value) {
						if (ao_lco_box == AO_LCO_BOX_DRAG) {
							ao_lco_fire_down = 1;
							ao_lco_fire_tick = ao_time();
							ao_lco_drag_active = 1;
							ao_wakeup(&ao_lco_drag_beep_count);
						} else {
							ao_lco_toggle_drag();
						}
					} else {
						ao_lco_fire_down = 0;
						if (ao_lco_drag_active)
							ao_wakeup(&ao_lco_drag_beep_count);
					}
				}
				break;
			}
			break;
		}
	}
}

/*
 * Light up everything for a second at power on to let the user
 * visually inspect the system for correct operation
 */
static void
ao_lco_display_test(void)
{
	ao_mutex_get(&ao_lco_display_mutex);
	ao_seven_segment_set(AO_LCO_PAD_DIGIT, 8 | 0x10);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_1, 8 | 0x10);
	ao_seven_segment_set(AO_LCO_BOX_DIGIT_10, 8 | 0x10);
	ao_mutex_put(&ao_lco_display_mutex);
	ao_led_on(LEDS_AVAILABLE);
	ao_delay(AO_MS_TO_TICKS(1000));
	ao_led_off(LEDS_AVAILABLE);
}

#if HAS_ADC_SINGLE
static void
ao_lco_batt_voltage(void)
{
	struct ao_adc	packet;
	int16_t		decivolt;

	ao_adc_single_get(&packet);
	decivolt = ao_battery_decivolt(packet.v_batt);
	ao_lco_show_voltage((uint16_t) decivolt);
	ao_delay(AO_MS_TO_TICKS(1000));
}
#endif

static struct ao_task ao_lco_input_task;
static struct ao_task ao_lco_monitor_task;
static struct ao_task ao_lco_arm_warn_task;
static struct ao_task ao_lco_igniter_status_task;

static void
ao_lco_main(void)
{
	ao_lco_display_test();
#if HAS_ADC_SINGLE
	ao_lco_batt_voltage();
#endif
	ao_lco_search();
	ao_add_task(&ao_lco_input_task, ao_lco_input, "lco input");
	ao_add_task(&ao_lco_arm_warn_task, ao_lco_arm_warn, "lco arm warn");
	ao_add_task(&ao_lco_igniter_status_task, ao_lco_igniter_status, "lco igniter status");
	ao_add_task(&ao_lco_drag_task, ao_lco_drag_monitor, "drag race");
	ao_lco_monitor();
}

#if DEBUG
static void
ao_lco_set_debug(void)
{
	uint32_t r  = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success)
		ao_lco_debug = r != 0;
}

const struct ao_cmds ao_lco_cmds[] = {
	{ ao_lco_set_debug,	"D <0 off, 1 on>\0Debug" },
	{ ao_lco_search,	"s\0Search for pad boxes" },
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
