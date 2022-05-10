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
#include <ao_radio_cmac_cmd.h>
#include <ao_radio_cmac.h>

static uint8_t cmac_data[AO_CMAC_MAX_LEN];

static uint8_t
getnibble(void)
{
	int8_t	b;

	b = ao_cmd_hexchar(getchar());
	if (b < 0) {
		ao_cmd_status = ao_cmd_lex_error;
		return 0;
	}
	return (uint8_t) b;
}

static uint8_t
getbyte(void)
{
	uint8_t	b;
	b = getnibble() << 4;
	b |= getnibble();
	return b;
}
	
static void
radio_cmac_send_cmd(void) 
{
	uint32_t i;
	uint32_t len;

	len = ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (len > AO_CMAC_MAX_LEN) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	flush();
	for (i = 0; i < len; i++) {
		cmac_data[i] = getbyte();
		if (ao_cmd_status != ao_cmd_success)
			return;
	}
	ao_radio_cmac_send(cmac_data, (uint8_t) len);
}

static void
radio_cmac_recv_cmd(void) 
{
	uint32_t len, l;
	int8_t i;
	AO_TICK_TYPE	timeout;

	len = ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (len > AO_CMAC_MAX_LEN) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	timeout = AO_MS_TO_TICKS(ao_cmd_decimal());
	if (ao_cmd_status != ao_cmd_success)
		return;
	i = ao_radio_cmac_recv(cmac_data, (uint8_t) len, timeout);
	if (i == AO_RADIO_CMAC_OK) {
		printf ("PACKET ");
		for (l = 0; l < len; l++)
			printf("%02x", cmac_data[l]);
		printf (" %d\n", ao_radio_cmac_rssi);
	} else
		printf ("ERROR %d %d\n", i, ao_radio_cmac_rssi);
}

static const struct ao_cmds ao_radio_cmac_cmds[] = {
	{ radio_cmac_send_cmd,	"s <length>\0Send AES-CMAC packet. Bytes to send follow on next line" },
	{ radio_cmac_recv_cmd,	"S <length> <timeout>\0Receive AES-CMAC packet. Timeout in ms" },
	{ 0, NULL },
};

void
ao_radio_cmac_cmd_init(void)
{
	ao_cmd_register(&ao_radio_cmac_cmds[0]);
}
