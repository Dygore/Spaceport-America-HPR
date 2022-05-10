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

#ifndef _AO_PACKET_H_
#define _AO_PACKET_H_

/*
 * ao_packet.c
 *
 * Packet-based command interface
 */

#define AO_PACKET_MAX		64
#define AO_PACKET_SYN		(uint8_t) 0xff

struct ao_packet {
	uint8_t		addr;
	uint8_t		len;
	uint8_t		seq;
	uint8_t		ack;
	uint8_t		d[AO_PACKET_MAX];
	uint8_t		callsign[AO_MAX_CALLSIGN];
};

struct ao_packet_recv {
	struct ao_packet	packet;
	int8_t			rssi;
	uint8_t			status;
};

extern struct ao_packet_recv ao_rx_packet;
extern struct ao_packet ao_tx_packet;
extern struct ao_task	ao_packet_task;
extern uint8_t ao_packet_enable;
extern uint8_t ao_packet_master_sleeping;
extern uint8_t ao_packet_rx_len, ao_packet_rx_used, ao_packet_tx_used;
extern uint8_t ao_packet_restart;

void
ao_packet_send(void);

uint8_t
ao_packet_recv(uint16_t timeout);

void
ao_packet_flush(void);

void
ao_packet_putchar(char c);

int
_ao_packet_pollchar(void);

#if PACKET_HAS_MASTER
/* ao_packet_master.c */

extern int8_t ao_packet_last_rssi;

void
ao_packet_master_init(void);
#endif

#if PACKET_HAS_SLAVE
/* ao_packet_slave.c */

void
ao_packet_slave_start(void);

void
ao_packet_slave_stop(void);

void
ao_packet_slave_init(uint8_t enable);

#endif

#endif /* _AO_PACKET_H_ */
