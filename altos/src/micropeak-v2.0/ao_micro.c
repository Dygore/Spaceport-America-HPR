/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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
#include <ao_micropeak.h>
#include <ao_adc_stm32l0.h>
#include <ao_report_micro.h>
#include <ao_log_micro.h>

alt_t		ground_alt, max_alt;
alt_t		ao_max_height;

static void
ao_msi_init(void)
{
	uint32_t icscr = stm_rcc.icscr;

	/* Set MSI clock to desired range */
	icscr &= ~(STM_RCC_ICSCR_MSIRANGE_MASK << STM_RCC_ICSCR_MSIRANGE);
	icscr |= (AO_MSI_RANGE << STM_RCC_ICSCR_MSIRANGE);
	stm_rcc.icscr = icscr;

	/* Set vcore to 1.2V */
	uint32_t cr = stm_pwr.cr;
	cr &= ~(STM_PWR_CR_VOS_MASK << STM_PWR_CR_VOS);
	cr |= (STM_PWR_CR_VOS_1_2 << STM_PWR_CR_VOS);
	stm_pwr.cr = cr;
}

void
ao_pa_get(void)
{
	static struct ao_ms5607_value	value;

	ao_ms5607_sample(&ao_ms5607_current);
	ao_ms5607_convert(&ao_ms5607_current, &value);
	pa = (uint32_t) value.pres;
}

static void
ao_compute_height(void)
{
	ground_alt = ao_pa_to_altitude((pres_t) pa_ground);
	max_alt = ao_pa_to_altitude((pres_t) pa_min);
	ao_max_height = max_alt - ground_alt;
}

static void
ao_pips(void)
{
	uint8_t	i;
	for (i = 0; i < 5; i++) {
		ao_led_on(AO_LED_REPORT);
		ao_delay(AO_MS_TO_TICKS(80));
		ao_led_off(AO_LED_REPORT);
		ao_delay(AO_MS_TO_TICKS(80));
	}
	ao_delay(AO_MS_TO_TICKS(200));
}

static void
power_down(void)
{
	ao_timer_stop();
	for(;;) {
		/*
		 * Table 40, entering standby mode
		 *
		 * SLEEPDEEP = 1 in M0 SCR
		 * PDDS = 1
		 * WUF = 0
		 */
		stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SYSCFGEN);
		stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_PWREN);
		stm_scb.scr |= ((1 << STM_SCB_SCR_SLEEPDEEP) |
				(1 << STM_SCB_SCR_SLEEPONEXIT));
		stm_pwr.cr |= (1 << STM_PWR_CR_PDDS);
		stm_pwr.csr &= ~(1UL << STM_PWR_CSR_WUF);
		ao_arch_wait_interrupt();
	}
}

static bool log_stdout;

static void
log_micro_dump(void)
{
	int i;
	if (!log_stdout) {
		ao_led_off(AO_LED_REPORT);
		ao_lpuart1_enable();
	}
	ao_log_micro_dump();
	for (i = 0; i < 4; i++)
		ao_async_byte(stm_device_id.lot_num_0_3[i]);
	for (i = 0; i < 3; i++)
		ao_async_byte(stm_device_id.lot_num_4_6[i]);
	ao_async_byte('-');
	ao_log_hex(stm_device_id.waf_num);
	ao_async_byte('-');
	for (i = 0; i < 4; i++)
		ao_log_hex(stm_device_id.unique_id[i]);
	ao_log_newline();
	if (!log_stdout)
		ao_lpuart1_disable();
}

static void
log_erase(void)
{
	uint32_t	pos;

	for (pos = 0; pos < ao_storage_total; pos += STM_FLASH_PAGE_SIZE)
	{
		if (!ao_storage_device_is_erased(pos))
			ao_storage_device_erase(pos);
	}
}

static void
flight_mode(void)
{
	/* Give the person a second to get their finger out of the way */
	ao_delay(AO_MS_TO_TICKS(1000));

	ao_log_micro_restore();
	ao_compute_height();
	ao_report_altitude();
	ao_pips();
	log_micro_dump();
#ifdef BOOST_DELAY
	ao_delay(BOOST_DELAY);
#endif
	log_erase();
	ao_microflight();
	ao_log_micro_save();
	ao_compute_height();
	ao_report_altitude();
	power_down();
}

void ao_async_byte(char c)
{
	if (log_stdout)
		putchar(c);
	else
		ao_lpuart1_putchar(c);
}

static void
log_micro_dump_uart(void)
{
	log_stdout = true;
	log_micro_dump();
	log_stdout = false;
}

static void
log_erase_cmd(void)
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	log_erase();
}

const struct ao_cmds ao_micro_cmds[] = {
	{ log_micro_dump_uart, "l\0Dump log" },
	{ flight_mode, "F\0Flight mode" },
	{ power_down, "S\0Standby" },
	{ log_erase_cmd, "z <key>\0Erase. <key> is doit with D&I" },
	{}
};

static void
cmd_mode(void)
{
	ao_serial_init();
	ao_cmd_init();
	ao_cmd_register(ao_micro_cmds);
	ao_cmd();
}

int
main(void)
{
	ao_msi_init();
	ao_led_init();
	ao_timer_init();
	ao_spi_init();
	ao_ms5607_init();
	ao_ms5607_setup();

	/* Check the power supply voltage; it'll be 3.3V if
	 * the I/O board is connected
	 */
	uint16_t vref = ao_adc_read_vref();

	uint32_t vdda = 3UL * stm_vrefint_cal.vrefint_cal * 1000 / vref;

	/* Power supply > 3.25V means we're on USB power */
	if (vdda > 3250) {
		cmd_mode();
	} else {
		flight_mode();
	}
}
