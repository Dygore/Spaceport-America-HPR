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

#include <ao.h>
#include <ao_adc_fast.h>
#include <ao_crc.h>
#include <ao_trng_send.h>

void main(void)
{
	ao_led_init(LEDS_AVAILABLE);
	ao_led_on(AO_LED_RED);
	ao_clock_init();
	ao_task_init();
	ao_timer_init();
	ao_dma_init();
	ao_adc_init();
	ao_crc_init();

	ao_usb_init();

	ao_trng_send_init();

	ao_led_off(AO_LED_RED);

	ao_start_scheduler();
}
