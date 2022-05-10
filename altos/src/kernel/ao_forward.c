/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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
#include "ao_telem.h"

static void
ao_monitor_forward(void)
{
	uint32_t					recv_radio_setting;
	static struct ao_telemetry_all_recv	packet;

	for (;;) {
		while (ao_monitoring)
			ao_sleep(&ao_monitoring);

		if (!ao_radio_recv(&packet, sizeof(packet), 0))
			continue;
		if (!(packet.status & PKT_APPEND_STATUS_1_CRC_OK))
			continue;
		recv_radio_setting = ao_config.radio_setting;
		ao_config.radio_setting = ao_send_radio_setting;
		ao_radio_send(&packet.telemetry, sizeof (packet.telemetry));
		ao_config.radio_setting = recv_radio_setting;
	}
}

static struct ao_task ao_monitor_forward_task;

void
ao_monitor_forward_init(void) 
{
	ao_add_task(&ao_monitor_forward_task, ao_monitor_forward, "monitor_forward");
}
