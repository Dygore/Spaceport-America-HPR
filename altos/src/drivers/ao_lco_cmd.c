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
#include <ao_pad.h>
#include <ao_lco_cmd.h>
#include <ao_lco_func.h>
#include <ao_radio_cmac.h>

#ifndef HAS_STATIC_TEST
#define HAS_STATIC_TEST	1
#endif

static uint16_t	lco_box;
static uint8_t	lco_channels;

static void
lco_args(void) 
{
	lco_box = (uint16_t) ao_cmd_decimal();
	lco_channels = (uint8_t) ao_cmd_hex();
}

static struct ao_pad_query	ao_pad_query;
static uint16_t			tick_offset;

static int8_t
lco_query(void)
{
	uint8_t	i;
	int8_t	r = AO_RADIO_CMAC_OK;

	for (i = 0; i < 10; i++) {
		printf ("."); flush();
		r = ao_lco_query(lco_box, &ao_pad_query, &tick_offset);
		if (r == AO_RADIO_CMAC_OK)
			break;
	}
	printf("\n"); flush();
	return r;
}

static void
lco_arm(void)
{
	ao_lco_arm(lco_box, lco_channels, tick_offset);
}

static void
lco_ignite(uint8_t cmd)
{
	ao_lco_ignite(cmd);
}

static void
lco_report_cmd(void) 
{
	int8_t		r;
	uint8_t		c;

	lco_args();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = lco_query();
	switch (r) {
	case AO_RADIO_CMAC_OK:
		switch (ao_pad_query.arm_status) {
		case AO_PAD_ARM_STATUS_ARMED:
			printf ("Armed: ");
			break;
		case AO_PAD_ARM_STATUS_DISARMED:
			printf("Disarmed: ");
			break;
		case AO_PAD_ARM_STATUS_UNKNOWN:
		default:
			printf("Unknown: ");
			break;
		}
		for (c = 0; c < AO_PAD_MAX_CHANNELS; c++) {
			if (ao_pad_query.channels & (1 << c)) {
				printf (" pad %d ", c);
				switch (ao_pad_query.igniter_status[c]) {
				default:
					printf("unknown, ");
					break;
				case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN:
					printf("bad-open, ");
					break;
				case AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN:
					printf("good-igniter, ");
					break;
				case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_CLOSED:
					printf("bad-closed, ");
					break;
				}
			}
		}
		printf("Rssi: %d\n", ao_radio_cmac_rssi);
		break;
	default:
		printf("Error %d\n", r);
		break;
	}
}

static void
lco_fire_cmd(void) 
{
	uint8_t		secs;
	uint8_t		i;
	int8_t		r;

	lco_args();
	secs = (uint8_t) ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = lco_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}

	for (i = 0; i < 4; i++) {
		printf("arm %d\n", i); flush();
		lco_arm();
	}

	secs = (uint8_t) (secs * 10 - 5);
	if (secs > 100)
		secs = 100;
	for (i = 0; i < secs; i++) {
		printf("fire %d\n", i); flush();
		lco_ignite(AO_PAD_FIRE);
		ao_delay(AO_MS_TO_TICKS(100));
	}
}

#if HAS_STATIC_TEST
static void
lco_static_cmd(void) 
{
	uint8_t		secs;
	uint8_t		i;
	int8_t		r;

	lco_args();
	secs = (uint8_t) ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = lco_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}

	for (i = 0; i < 4; i++) {
		printf("arm %d\n", i); flush();
		lco_arm();
	}

	secs = (uint8_t) (secs * 10 - 5);
	if (secs > 100)
		secs = 100;
	for (i = 0; i < secs; i++) {
		printf("fire %d\n", i); flush();
		lco_ignite(AO_PAD_STATIC);
		ao_delay(AO_MS_TO_TICKS(100));
	}
}
#endif

static void
lco_arm_cmd(void) 
{
	uint8_t	i;
	int8_t  r;
	lco_args();
	r = lco_query();
	if (r != AO_RADIO_CMAC_OK) {
		printf("query failed %d\n", r);
		return;
	}
	for (i = 0; i < 4; i++)
		lco_arm();
}

static void
lco_ignite_cmd(void) 
{
	uint8_t i;
	lco_args();
	for (i = 0; i < 4; i++)
		lco_ignite(AO_PAD_FIRE);
}


#if HAS_STATIC_TEST
static void
lco_endstatic_cmd(void) 
{
	lco_ignite(AO_PAD_ENDSTATIC);
}
#endif

static const struct ao_cmds ao_lco_cmds[] = {
	{ lco_report_cmd,	"l <box> <channel>\0Get remote status" },
	{ lco_fire_cmd,		"F <box> <channel> <secs>\0Fire remote igniters" },
#if HAS_STATIC_TEST
	{ lco_static_cmd,	"S <box> <channel> <secs>\0Initiate static test" },
	{ lco_endstatic_cmd,	"D\0End static test (and download someday)" },
#endif
	{ lco_arm_cmd,		"a <box> <channel>\0Arm remote igniter" },
	{ lco_ignite_cmd,	"i <box> <channel>\0Pulse remote igniter" },
	{ 0, NULL },
};

void
ao_lco_cmd_init(void)
{
	ao_cmd_register(&ao_lco_cmds[0]);
}
