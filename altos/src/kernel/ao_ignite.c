/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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
#include <ao_data.h>
#if AO_PYRO_NUM
#include <ao_pyro.h>
#endif

#if HAS_IGNITE
struct ao_ignition ao_ignition[2];

#ifndef AO_SENSE_DROGUE
#define AO_SENSE_DROGUE(p)	((p)->adc.sense_d)
#define AO_SENSE_MAIN(p)	((p)->adc.sense_m)
#endif

enum ao_igniter_status
ao_igniter_status(enum ao_igniter igniter)
{
	struct ao_data packet;
	int16_t value;
	uint8_t request, firing, fired;

	ao_arch_critical(
		ao_data_get(&packet);
		request = ao_ignition[igniter].request;
		fired = ao_ignition[igniter].fired;
		firing = ao_ignition[igniter].firing;
		);
	if (firing || (request && !fired))
		return ao_igniter_active;

	value = (AO_IGNITER_CLOSED>>1);
	switch (igniter) {
	case ao_igniter_drogue:
		value = AO_SENSE_DROGUE(&packet);
		break;
	case ao_igniter_main:
		value = AO_SENSE_MAIN(&packet);
		break;
	}
	if (value < AO_IGNITER_OPEN)
		return ao_igniter_open;
	else if (value > AO_IGNITER_CLOSED)
		return ao_igniter_ready;
	else
		return ao_igniter_unknown;
}

#define AO_IGNITER_SET_DROGUE(v)	ao_gpio_set(AO_IGNITER_DROGUE_PORT, AO_IGNITER_DROGUE_PIN, v)
#define AO_IGNITER_SET_MAIN(v)		ao_gpio_set(AO_IGNITER_MAIN_PORT, AO_IGNITER_MAIN_PIN, v)

#ifndef AO_IGNITER_FIRE_TIME
#define AO_IGNITER_FIRE_TIME		AO_MS_TO_TICKS(50)
#endif

#ifndef AO_IGNITER_CHARGE_TIME
#define AO_IGNITER_CHARGE_TIME		AO_MS_TO_TICKS(2000)
#endif

static void
ao_igniter_fire(enum ao_igniter igniter, bool wait)
{
	if (!ao_ignition[igniter].fired) {
		ao_ignition[igniter].firing = 1;
		ao_ignition[igniter].fired = 1;
		switch (igniter) {
		case ao_igniter_drogue:
			if (wait && ao_config.apogee_delay &&
			    ao_config.ignite_mode != AO_IGNITE_MODE_MAIN)
				ao_delay(AO_SEC_TO_TICKS(ao_config.apogee_delay));
			AO_IGNITER_SET_DROGUE(1);
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_SET_DROGUE(0);
			break;
		case ao_igniter_main:
			AO_IGNITER_SET_MAIN(1);
			ao_delay(AO_IGNITER_FIRE_TIME);
			AO_IGNITER_SET_MAIN(0);
			break;
		}
		ao_ignition[igniter].firing = 0;
		if (wait)
			ao_delay(AO_IGNITER_CHARGE_TIME);
	}
}

static void
ao_igniter(void)
{
	ao_config_get();
	for (;;) {
		/* Wait for flight state change */
		ao_sleep(&ao_flight_state);

		/* Fire any igniters that are supposed to be triggered
		 * in this new state
		 */
		switch(ao_config.ignite_mode) {
		case AO_IGNITE_MODE_DUAL:
			if (ao_flight_drogue <= ao_flight_state && ao_flight_state < ao_flight_landed)
				ao_igniter_fire(ao_igniter_drogue, true);
			if (ao_flight_main <= ao_flight_state && ao_flight_state < ao_flight_landed)
				ao_igniter_fire(ao_igniter_main, true);
			break;
		case AO_IGNITE_MODE_APOGEE:
			if (ao_flight_drogue <= ao_flight_state && ao_flight_state < ao_flight_landed) {
				ao_igniter_fire(ao_igniter_drogue, true);
				ao_igniter_fire(ao_igniter_main, true);
			}
			break;
		case AO_IGNITE_MODE_MAIN:
			if (ao_flight_main <= ao_flight_state && ao_flight_state < ao_flight_landed) {
				ao_igniter_fire(ao_igniter_drogue, true);
				ao_igniter_fire(ao_igniter_main, true);
			}
			break;
		case AO_IGNITE_MODE_BOOSTER:
			if (ao_flight_fast <= ao_flight_state && ao_flight_state < ao_flight_landed)
				ao_igniter_fire(ao_igniter_main, true);
			if (ao_flight_drogue <= ao_flight_state && ao_flight_state < ao_flight_landed)
				ao_igniter_fire(ao_igniter_drogue, true);
			break;
		}
	}
}

#endif

static void
ao_ignite_manual(void)
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ao_cmd_white();
#if HAS_IGNITE
	if (ao_cmd_lex_c == 'm' && ao_match_word("main")) {
		ao_ignition[ao_igniter_main].fired = 0;
		ao_igniter_fire(ao_igniter_main, false);
		return;
	}
	if (ao_cmd_lex_c == 'd' && ao_match_word("drogue")) {
		ao_ignition[ao_igniter_drogue].fired = 0;
		ao_igniter_fire(ao_igniter_drogue, false);
		return;
	}
#endif
#if AO_PYRO_NUM
	if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9') {
		ao_pyro_manual(ao_cmd_lex_c - '0');
		return;
	}
#endif
	ao_cmd_status = ao_cmd_syntax_error;
}

const char * const ao_igniter_status_names[] = {
	"unknown", "ready", "active", "open"
};

#if HAS_IGNITE
static void
ao_ignite_print_status(enum ao_igniter igniter, const char *name) 
{
	enum ao_igniter_status status = ao_igniter_status(igniter);
	printf("Igniter: %6s Status: %s\n",
	       name,
	       ao_igniter_status_names[status]);
}
#endif

static void
ao_ignite_test(void)
{
#if HAS_IGNITE
	ao_ignite_print_status(ao_igniter_drogue, "drogue");
	ao_ignite_print_status(ao_igniter_main, "main");
#endif
#if AO_PYRO_NUM
	ao_pyro_print_status();
#endif
}

const struct ao_cmds ao_ignite_cmds[] = {
	{ ao_ignite_manual,	"i <key> {main|drogue}\0Fire igniter. <key> is doit with D&I" },
	{ ao_ignite_test,	"t\0Test igniter" },
	{ 0,	NULL },
};

#if HAS_IGNITE
struct ao_task ao_igniter_task;

void
ao_ignite_set_pins(void)
{
	ao_enable_output(AO_IGNITER_DROGUE_PORT, AO_IGNITER_DROGUE_PIN, 0);
	ao_enable_output(AO_IGNITER_MAIN_PORT, AO_IGNITER_MAIN_PIN, 0);
}
#endif

void
ao_igniter_init(void)
{
#if HAS_IGNITE
	ao_ignite_set_pins();
	ao_add_task(&ao_igniter_task, ao_igniter, "igniter");
#endif
#if HAS_IGNITE || AO_PYRO_NUM
	ao_cmd_register(&ao_ignite_cmds[0]);
#endif
}
