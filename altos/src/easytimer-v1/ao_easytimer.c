/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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
#include <ao_ms5607.h>
#include <ao_bmx160.h>
#include <ao_log.h>
#include <ao_exti.h>
#include <ao_packet.h>
#include <ao_companion.h>
#include <ao_eeprom.h>
#include <ao_profile.h>
#if HAS_SAMPLE_PROFILE
#include <ao_sample_profile.h>
#endif
#if HAS_STACK_GUARD
#include <ao_mpu.h>
#endif

int
main(void)
{
	ao_clock_init();

#if HAS_STACK_GUARD
	ao_mpu_init();
#endif

	ao_task_init();
	ao_timer_init();

	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_adc_init();
	ao_beep_init();
	ao_cmd_init();

	ao_eeprom_init();

//	ao_storage_init();

	ao_bmx160_init();

	ao_flight_init();
	// ao_log_init();
	ao_report_init();

	ao_usb_init();
	ao_pyro_init();
	ao_igniter_init();

	ao_config_init();
#if AO_PROFILE
	ao_profile_init();
#endif
#if HAS_SAMPLE_PROFILE
	ao_sample_profile_init();
#endif

	ao_start_scheduler();
	return 0;
}
