/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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
#include <ao_exti.h>
#include <ao_watchdog.h>
#include <ao_storage.h>
#include <ao_fat.h>

int
main(void)
{
	ao_clock_init();
	
#if HAS_STACK_GUARD
	ao_mpu_init();
#endif

	ao_task_init();
	ao_led_init();
	ao_led_on(AO_LED_RED|AO_LED_GREEN|AO_LED_RED_2|AO_LED_GREEN_2);
	ao_timer_init();

	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_adc_init();

	ao_cmd_init();

	ao_usb_init();

	ao_storage_init();

	ao_watchdog_init();

	ao_fat_init();

	ao_start_scheduler();
	return 0;
}
