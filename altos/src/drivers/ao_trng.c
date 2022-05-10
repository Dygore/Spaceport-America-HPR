/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
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
#include <ao_adc_fast.h>
#include <ao_crc.h>
#include <ao_trng.h>

static void
ao_trng_fetch(void)
{
	static uint16_t	*buffer[2];
	uint32_t	kbytes = 1;
	uint32_t	count;
	int		usb_buf_id;
	uint16_t	i;
	uint16_t	*buf;
	uint16_t	t;
	uint32_t	*rnd = (uint32_t *) ao_adc_ring;

	if (!buffer[0]) {
		buffer[0] = ao_usb_alloc();
		buffer[1] = ao_usb_alloc();
		if (!buffer[0])
			return;
	}

	ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success)
		kbytes = ao_cmd_lex_u32;
	else
		ao_cmd_status = ao_cmd_success;
	usb_buf_id = 0;
	count = kbytes * (1024/AO_USB_IN_SIZE);

	ao_crc_reset();

	ao_led_on(AO_LED_TRNG_READ);
	while (count--) {
		t = ao_adc_get(AO_USB_IN_SIZE) >> 1;	/* one 16-bit value per output byte */
		buf = buffer[usb_buf_id];
		for (i = 0; i < AO_USB_IN_SIZE / sizeof (uint16_t); i++) {
			*buf++ = ao_crc_in_32_out_16(rnd[t]);
			t = (t + 1) & ((AO_ADC_RING_SIZE>>1) - 1);
		}
		ao_adc_ack(AO_USB_IN_SIZE);
		ao_led_toggle(AO_LED_TRNG_READ|AO_LED_TRNG_WRITE);
		ao_usb_write(buffer[usb_buf_id], AO_USB_IN_SIZE);
		ao_led_toggle(AO_LED_TRNG_READ|AO_LED_TRNG_WRITE);
		usb_buf_id = 1-usb_buf_id;
	}
	ao_led_off(AO_LED_TRNG_READ|AO_LED_TRNG_WRITE);
	flush();
}

static const struct ao_cmds ao_trng_cmds[] = {
	{ ao_trng_fetch,	"f <kbytes>\0Fetch a block of numbers" },
	{ 0, NULL },
};

void
ao_trng_init(void)
{
	ao_cmd_register(ao_trng_cmds);
}
