/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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
#include <ao_exti.h>

#if HAS_FORCE_FREQ
static void
ao_check_recovery(void)
{
	int	i;
	ao_enable_input(AO_RECOVERY_PORT, AO_RECOVERY_PIN, AO_RECOVERY_MODE);
	for (i = 0; i < 100; i++)
		ao_arch_nop();
	if (ao_gpio_get(AO_RECOVERY_PORT, AO_RECOVERY_PIN) == AO_RECOVERY_VALUE) {
		ao_flight_force_idle = 1;
		ao_force_freq = 1;
	}
	ao_gpio_set_mode(AO_RECOVERY_PORT, AO_RECOVERY_PIN, 0);
	ao_disable_port(AO_RECOVERY_PORT);
}
#endif

int
main(void)
{
#if HAS_FORCE_FREQ
	ao_check_recovery();
#endif

	ao_clock_init();
	ao_task_init();
	ao_timer_init();

	ao_dma_init();
	ao_spi_init();
	ao_exti_init();

	ao_adc_init();

#if HAS_BEEP
	ao_beep_init();
#endif
#if HAS_SERIAL_1
	ao_serial_init();
#endif
#if HAS_USB
	ao_usb_init();
#endif
	ao_cmd_init();

	ao_ms5607_init();

	ao_storage_init();
	ao_flight_init();
	ao_log_init();
	ao_report_init();
	ao_telemetry_init();
	ao_radio_init();
	ao_packet_slave_init(true);
	ao_igniter_init();
	ao_config_init();

	ao_start_scheduler();
}
