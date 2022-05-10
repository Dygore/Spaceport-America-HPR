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
#include <ao_exti.h>
#include <ao_radio_cmac_cmd.h>

int
main(void)
{
	ao_clock_init();

	ao_led_init();
	ao_led_on(LEDS_AVAILABLE);

	ao_task_init();

	ao_timer_init();
	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_cmd_init();

	ao_adc_init();

	ao_eeprom_init();

	ao_radio_init();

	ao_usb_init();

	ao_config_init();

	ao_pad_init();

//	ao_radio_cmac_cmd_init();

	ao_led_off(LEDS_AVAILABLE);

	ao_start_scheduler();
}
