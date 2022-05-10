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
#include <ao_log.h>
#include <ao_pad.h>
#include <ao_exti.h>
#include <ao_radio_cmac_cmd.h>
#include <ao_eeprom.h>
#include <ao_ads124s0x.h>
#include <ao_max6691.h>

static void
set_logging(void)
{
	ao_log_running = ao_cmd_hex();
	ao_wakeup(&ao_log_running);
}

const struct ao_cmds ao_firetwo_cmds[] = {
        { set_logging,  "L <0 off, 1 on>\0Log sensors to flash" },
        { 0,    NULL },
};

void
main(void)
{
	ao_clock_init();

	ao_led_init();

	ao_task_init();

	ao_timer_init();
	ao_spi_init();
	ao_dma_init();
	ao_exti_init();

	ao_cmd_register(&ao_firetwo_cmds[0]);
	ao_cmd_init();

	ao_adc_init();

	ao_max6691_init();

	ao_eeprom_init();
	ao_storage_init();
	ao_log_init();

	ao_radio_init();

	ao_usb_init();

	ao_config_init();

	ao_pad_init();

//	ao_radio_cmac_cmd_init();

	ao_ads124s0x_init();

	ao_start_scheduler();
}
