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

static void
ao_gpio_suspend(void *arg)
{
	struct stm_gpio *port = arg;
	if (port == &stm_gpioa)
		stm_rcc.ahbenr &= ~(1UL << STM_RCC_AHBENR_IOPAEN);
	else if ((port) == &stm_gpiob)
		stm_rcc.ahbenr &= ~(1UL << STM_RCC_AHBENR_IOPBEN);
	else if ((port) == &stm_gpioc)
		stm_rcc.ahbenr &= ~(1UL << STM_RCC_AHBENR_IOPCEN);
	else if ((port) == &stm_gpiof)
		stm_rcc.ahbenr &= ~(1UL << STM_RCC_AHBENR_IOPFEN);
}

static void
ao_gpio_resume(void *arg)
{
	struct stm_gpio *port = arg;
	if (port == &stm_gpioa)
		stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_IOPAEN);
	else if ((port) == &stm_gpiob)
		stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_IOPBEN);
	else if ((port) == &stm_gpioc)
		stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_IOPCEN);
	else if ((port) == &stm_gpiof)
		stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_IOPFEN);
}

struct ao_power	ao_power_gpioa = {
	.suspend = ao_gpio_suspend,
	.resume = ao_gpio_resume,
	.arg = &stm_gpioa
};

struct ao_power	ao_power_gpiob = {
	.suspend = ao_gpio_suspend,
	.resume = ao_gpio_resume,
	.arg = &stm_gpiob
};

struct ao_power	ao_power_gpioc = {
	.suspend = ao_gpio_suspend,
	.resume = ao_gpio_resume,
	.arg = &stm_gpioc
};

struct ao_power	ao_power_gpiof = {
	.suspend = ao_gpio_suspend,
	.resume = ao_gpio_resume,
	.arg = &stm_gpiof
};

