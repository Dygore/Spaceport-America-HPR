/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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
#include <ao_boot.h>
#include <ao_exti.h>

int
ao_boot_check_pin(void)
{
	uint16_t v;

	/* Enable the input pin */
	ao_enable_input(AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN,
			AO_BOOT_APPLICATION_MODE);

	for (v = 0; v < 100; v++)
		ao_arch_nop();

	/* Read the value */
	v = ao_gpio_get(AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN);

	/* Reset the chip to turn off the port and the power interface clock */
	ao_gpio_set_mode(AO_BOOT_APPLICATION_GPIO, AO_BOOT_APPLICATION_PIN, 0);
	ao_disable_port(AO_BOOT_APPLICATION_GPIO);
	return v == AO_BOOT_APPLICATION_VALUE;
}
