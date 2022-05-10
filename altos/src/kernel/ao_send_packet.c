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

#include "ao.h"
#include "ao_send_packet.h"

#define AO_MAX_SEND	128

static uint8_t ao_send[AO_MAX_SEND];

static void
ao_send_packet(void)
{
	uint32_t count;
	uint8_t b;
	uint8_t	i;

	count = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (count > AO_MAX_SEND - 2) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	for (i = 0; i < count; i++) {
		b = ao_getnibble() << 4;
		b |= ao_getnibble();
		if (ao_cmd_status != ao_cmd_success)
			return;
		ao_send[i] = b;
	}
	ao_radio_send(ao_send, (uint8_t) count);
}

static const struct ao_cmds ao_send_packet_cmds[] = {
	{ ao_send_packet, "S <len>\0Send packet. Data on next line" },
	{ 0, NULL }
};

void
ao_send_packet_init(void)
{
	ao_cmd_register(&ao_send_packet_cmds[0]);
}
