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
#include <ao_exti.h>

static void	(*ao_exti_callback[16])(void);

uint32_t	ao_last_exti;

static void ao_exti_range_isr(uint8_t first, uint8_t last, uint16_t mask) {
	uint16_t	pending = (uint16_t) ((ao_last_exti = stm_exti.pr) & mask);
	uint8_t		pin;
	static uint16_t	last_mask;
	static uint8_t	last_pin;

	if (pending == last_mask) {
		stm_exti.pr = last_mask;
		(*ao_exti_callback[last_pin])();
		return;
	}
	stm_exti.pr = pending;
	for (pin = first; pin <= last; pin++)
		if ((pending & ((uint32_t) 1 << pin)) && ao_exti_callback[pin]) {
			last_mask = (1 << pin);
			last_pin = pin;
			(*ao_exti_callback[pin])();
		}
}

void stm_exti0_1_isr(void) { ao_exti_range_isr(0, 1, 0x0003); }
void stm_exti2_3_isr(void) { ao_exti_range_isr(2, 3, 0x000c); }
void stm_exti4_15_isr(void) { ao_exti_range_isr(4, 15, 0xfff0); }

void
ao_exti_setup (struct stm_gpio *gpio, uint8_t pin, uint8_t mode, void (*callback)(void)) {
	uint32_t	mask = 1 << pin;
	uint32_t	pupdr;
	uint8_t		irq;
	uint8_t		prio;

	ao_exti_callback[pin] = callback;

	/* configure gpio to interrupt routing */
	stm_exticr_set(gpio, pin);

	if (!(mode & AO_EXTI_PIN_NOCONFIGURE)) {
		/* configure pin as input, setting selected pull-up/down mode */
		stm_moder_set(gpio, pin, STM_MODER_INPUT);
		switch (mode & (AO_EXTI_MODE_PULL_UP|AO_EXTI_MODE_PULL_DOWN)) {
		case 0:
		default:
			pupdr  = STM_PUPDR_NONE;
			break;
		case AO_EXTI_MODE_PULL_UP:
			pupdr = STM_PUPDR_PULL_UP;
			break;
		case AO_EXTI_MODE_PULL_DOWN:
			pupdr = STM_PUPDR_PULL_DOWN;
			break;
		}
		stm_pupdr_set(gpio, pin, pupdr);
	}

	/* Set interrupt mask and rising/falling mode */
	stm_exti.imr &= ~mask;
	if (mode & AO_EXTI_MODE_RISING)
		stm_exti.rtsr |= mask;
	else
		stm_exti.rtsr &= ~mask;
	if (mode & AO_EXTI_MODE_FALLING)
		stm_exti.ftsr |= mask;
	else
		stm_exti.ftsr &= ~mask;

	if (pin <= 1)
		irq = STM_ISR_EXTI0_1_POS;
	else if (pin <= 3)
		irq = STM_ISR_EXTI2_3_POS;
	else
		irq = STM_ISR_EXTI4_15_POS;

	/* Set priority */
	prio = AO_STM_NVIC_MED_PRIORITY;
	if (mode & AO_EXTI_PRIORITY_LOW)
		prio = AO_STM_NVIC_LOW_PRIORITY;
	else if (mode & AO_EXTI_PRIORITY_HIGH)
		prio = AO_STM_NVIC_HIGH_PRIORITY;

	stm_nvic_set_priority(irq, prio);
	stm_nvic_set_enable(irq);
}

void
ao_exti_set_mode(struct stm_gpio *gpio, uint8_t pin, uint8_t mode) {
	(void) gpio;

	uint32_t	mask = 1 << pin;

	if (mode & AO_EXTI_MODE_RISING)
		stm_exti.rtsr |= mask;
	else
		stm_exti.rtsr &= ~mask;
	if (mode & AO_EXTI_MODE_FALLING)
		stm_exti.ftsr |= mask;
	else
		stm_exti.ftsr &= ~mask;
}

void
ao_exti_set_callback(struct stm_gpio *gpio, uint8_t pin, void (*callback)(void)) {
	(void) gpio;
	ao_exti_callback[pin] = callback;
}

void
ao_exti_enable(struct stm_gpio *gpio, uint8_t pin) {
	uint32_t	mask = (1 << pin);
	(void) gpio;
	stm_exti.pr = mask;
	stm_exti.imr |= (1 << pin);
}

void
ao_exti_disable(struct stm_gpio *gpio, uint8_t pin) {
	uint32_t	mask = (1 << pin);
	(void) gpio;
	stm_exti.imr &= ~mask;
	stm_exti.pr = mask;
}

void
ao_exti_init(void)
{
}
