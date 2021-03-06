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

static int
ao_packet_getchar(void)
{
	int c;

	/* No need to block interrupts in this function as
	 * all packet variables are only modified from task
	 * context, not an interrupt handler
	 */
	while ((c = _ao_packet_pollchar()) == AO_READ_AGAIN) {
		if (!ao_packet_enable)
			break;
		if (ao_packet_master_sleeping)
			ao_wakeup(&ao_packet_master_sleeping);
		flush();
		ao_sleep(&ao_stdin_ready);
	}
	return c;
}

static void
ao_packet_echo(void) 
{
	int	c;
	while (ao_packet_enable) {
		c = ao_packet_getchar();
		if (c != AO_READ_AGAIN)
			putchar((char) c);
	}
	ao_exit();
}

static struct ao_task	ao_packet_echo_task;
static AO_TICK_TYPE	ao_packet_master_delay;
static AO_TICK_TYPE	ao_packet_master_time;

#define AO_PACKET_MASTER_DELAY_SHORT	AO_MS_TO_TICKS(100)
#define AO_PACKET_MASTER_DELAY_LONG	AO_MS_TO_TICKS(1000)
#define AO_PACKET_MASTER_DELAY_TIMEOUT	AO_MS_TO_TICKS(2000)

#if HAS_RADIO_RATE
#define AO_PACKET_MASTER_RECV_DELAY	AO_MS_TO_TICKS(100) << (ao_config.radio_rate << 1)
#else
#define AO_PACKET_MASTER_RECV_DELAY	AO_MS_TO_TICKS(100)
#endif

static void
ao_packet_master_busy(void)
{
	ao_packet_master_delay = AO_PACKET_MASTER_DELAY_SHORT;
	ao_packet_master_time = ao_time();
}

static void
ao_packet_master_check_busy(void)
{
	AO_TICK_SIGNED	idle;
	if (ao_packet_master_delay != AO_PACKET_MASTER_DELAY_SHORT)
		return;
	idle = (AO_TICK_SIGNED) (ao_time() - ao_packet_master_time);

	if (idle > AO_PACKET_MASTER_DELAY_TIMEOUT)
		ao_packet_master_delay = AO_PACKET_MASTER_DELAY_LONG;
}

static void
ao_packet_master(void)
{
	ao_config_get();
	ao_tx_packet.addr = (uint8_t) ao_serial_number;
	ao_tx_packet.len = AO_PACKET_SYN;
	ao_packet_master_time = ao_time();
	ao_packet_master_delay = AO_PACKET_MASTER_DELAY_SHORT;
	while (ao_packet_enable) {
		uint8_t	r;
		memcpy(ao_tx_packet.callsign, ao_config.callsign, AO_MAX_CALLSIGN);
		ao_packet_send();
		if (ao_tx_packet.len)
			ao_packet_master_busy();
		ao_packet_master_check_busy();
		r = ao_packet_recv(AO_PACKET_MASTER_RECV_DELAY);
		if (r) {
			/* if we can transmit data, do so */
			if (ao_packet_tx_used && ao_tx_packet.len == 0)
				continue;
			if (ao_rx_packet.packet.len)
				ao_packet_master_busy();
			ao_packet_master_sleeping = 1;
			ao_sleep_for(&ao_packet_master_sleeping, ao_packet_master_delay);
			ao_packet_master_sleeping = 0;
		}
	}
	ao_exit();
}

static void
ao_packet_forward(void) 
{
	char c;
	ao_packet_enable = 1;
	ao_packet_tx_used = 0;
	ao_cmd_white();

	flush();
#if HAS_MONITOR
	ao_monitor_disable();
#endif
	ao_add_task(&ao_packet_task, ao_packet_master, "master");
	ao_add_task(&ao_packet_echo_task, ao_packet_echo, "echo");
	while ((c = getchar()) != '~') {
		if (c == '\r') c = '\n';
		ao_packet_putchar(c);
	}

	/* Wait for a second if there is any pending data */
	for (c = 0; (ao_packet_tx_used || ao_tx_packet.len) && c < 10; c++)
		ao_delay(AO_MS_TO_TICKS(100));
	ao_packet_enable = 0;
	while (ao_packet_echo_task.wchan || ao_packet_task.wchan) {
		ao_radio_recv_abort();
		ao_wakeup(&ao_stdin_ready);
		ao_delay(AO_MS_TO_TICKS(10));
	}
#if HAS_MONITOR
	ao_monitor_enable();
#endif
}

static void
ao_packet_signal(void)
{
	printf ("RSSI: %d\n", ao_radio_rssi);
}

const struct ao_cmds ao_packet_master_cmds[] = {
	{ ao_packet_forward,	"p\0Remote packet link." },
	{ ao_packet_signal,	"s\0Report signal strength." },
	{ 0,	NULL },
};

void
ao_packet_master_init(void)
{
	ao_cmd_register(&ao_packet_master_cmds[0]);
}
