/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "ao.h"
#include "ao_ps2.h"
#include "ao_exti.h"

static struct ao_fifo	ao_ps2_rx_fifo;

static uint16_t		ao_ps2_tx;
static uint8_t		ao_ps2_tx_count;

static AO_TICK_TYPE	ao_ps2_tick;
static uint16_t		ao_ps2_value;
static uint8_t		ao_ps2_count;

uint8_t			ao_ps2_stdin;

uint8_t			ao_ps2_scancode_set;

#define AO_PS2_CLOCK_MODE(pull) ((pull) | AO_EXTI_MODE_FALLING | AO_EXTI_PRIORITY_MED)

static void
ao_ps2_isr(void);

static uint8_t
_ao_ps2_parity(uint8_t value)
{
	uint8_t	parity = 1;
	uint8_t	b;

	for (b = 0; b < 8; b++) {
		parity ^= (value & 1);
		value >>= 1;
	}
	return parity;
}

static int
_ao_ps2_poll(void)
{
	uint8_t	u;
	if (ao_fifo_empty(ao_ps2_rx_fifo)) {
		return AO_READ_AGAIN;
	}
	ao_fifo_remove(ao_ps2_rx_fifo, u);

	return (int) u;
}

uint8_t
ao_ps2_get(void)
{
	int c;
	ao_arch_block_interrupts();
	while ((c = _ao_ps2_poll()) == AO_READ_AGAIN)
		ao_sleep(&ao_ps2_rx_fifo);
	ao_arch_release_interrupts();
	return (uint8_t) c;
}


int
ao_ps2_poll(void)
{
	int	c;
	ao_arch_block_interrupts();
	c = _ao_ps2_poll();
	ao_arch_release_interrupts();
	return (uint8_t) c;
}

void
ao_ps2_put(uint8_t c)
{
	ao_arch_block_interrupts();
	ao_ps2_tx = ((uint16_t) c) | (_ao_ps2_parity(c) << 8) | (3 << 9);
	ao_ps2_tx_count = 11;
	ao_exti_disable(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT);
	ao_arch_release_interrupts();

	/* pull the clock pin down */
	ao_enable_output(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT, 0);
	ao_delay(0);

	/* pull the data pin down for the start bit */
	ao_enable_output(AO_PS2_DATA_PORT, AO_PS2_DATA_BIT, 0);
	ao_delay(0);

	/* switch back to input mode for the interrupt to work */
	ao_exti_setup(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT,
		      AO_PS2_CLOCK_MODE(AO_EXTI_MODE_PULL_UP),
		      ao_ps2_isr);
	ao_exti_enable(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT);

	/* wait for the bits to drain */
	while (ao_ps2_tx_count)
		ao_sleep(&ao_ps2_tx_count);

}

static uint8_t	ao_ps2_down[128 / 8];

static void
ao_ps2_set_down(uint8_t code, uint8_t value)
{
	uint8_t shift = (code & 0x07);
	uint8_t	byte = code >> 3;

	ao_ps2_down[byte] = (ao_ps2_down[byte] & ~(1 << shift)) | (value << shift);
}

uint8_t
ao_ps2_is_down(uint8_t code)
{
	uint8_t shift = (code & 0x07);
	uint8_t	byte = code >> 3;

	return (ao_ps2_down[byte] >> shift) & 1;
}

static void
_ao_ps2_set_leds(void)
{
	uint8_t	led = 0;
	if (ao_ps2_is_down(AO_PS2_CAPS_LOCK))
		led |= AO_PS2_SET_LEDS_CAPS;
	if (ao_ps2_is_down(AO_PS2_NUM_LOCK))
		led |= AO_PS2_SET_LEDS_NUM;
	if (ao_ps2_is_down(AO_PS2_SCROLL_LOCK))
		led |= AO_PS2_SET_LEDS_SCROLL;
	ao_arch_release_interrupts();
	ao_ps2_put(AO_PS2_SET_LEDS);
	while (ao_ps2_get() != 0xfa);
	ao_ps2_put(led);
	ao_arch_block_interrupts();
}

static uint8_t
ao_ps2_is_lock(uint8_t code) {
	switch (code) {
	case AO_PS2_CAPS_LOCK:
	case AO_PS2_NUM_LOCK:
	case AO_PS2_SCROLL_LOCK:
		return 1;
	}
	return 0;
}

static void
_ao_ps2_set_scancode_set(uint8_t set)
{
	ao_ps2_scancode_set = set;
	ao_arch_release_interrupts();
	ao_ps2_put(AO_PS2_SET_SCAN_CODE_SET);
	while (ao_ps2_get() != 0xfa);
	ao_ps2_put(set);
	ao_ps2_put(AO_PS2_SET_KEY_TYPEMATIC_MAKE_BREAK);
	while (ao_ps2_get() != 0xfa);
	ao_arch_block_interrupts();
}

static int
_ao_ps2_poll_key(void)
{
	int	c;
	uint8_t	set_led = 0;
	static uint8_t	saw_break;

	c = _ao_ps2_poll();
	if (c < 0) {
		if (ao_ps2_scancode_set != 3) {
			_ao_ps2_set_scancode_set(3);
		}
		return c;
	}

	if (c == AO_PS2_BREAK) {
		saw_break = 1;
		return AO_READ_AGAIN;
	}
	if (c & 0x80)
		return AO_READ_AGAIN;

	if (ao_ps2_is_lock(c)) {
		if (saw_break) {
			saw_break = 0;
			return AO_READ_AGAIN;
		}
		if (ao_ps2_is_down(c))
			saw_break = 1;
		set_led = 1;
	}
	if (saw_break) {
		saw_break = 0;
		ao_ps2_set_down(c, 0);
		c |= 0x80;
	} else
		ao_ps2_set_down(c, 1);
	if (set_led)
		_ao_ps2_set_leds();

	if (ao_ps2_scancode_set != 3)
		_ao_ps2_set_scancode_set(3);

	return c;
}

int
ao_ps2_poll_key(void)
{
	int	c;
	ao_arch_block_interrupts();
	c = _ao_ps2_poll_key();
	ao_arch_release_interrupts();
	return c;
}

uint8_t
ao_ps2_get_key(void)
{
	int	c;
	ao_arch_block_interrupts();
	while ((c = _ao_ps2_poll_key()) == AO_READ_AGAIN)
		ao_sleep(&ao_ps2_rx_fifo);
	ao_arch_release_interrupts();
	return (uint8_t) c;
}

static const uint8_t	ao_ps2_asciimap[128][2] = {
	[AO_PS2_A] = { 'a', 'A' },
	[AO_PS2_B] = { 'b', 'B' },
	[AO_PS2_C] = { 'c', 'C' },
	[AO_PS2_D] = { 'd', 'D' },
	[AO_PS2_E] = { 'e', 'E' },
	[AO_PS2_F] = { 'f', 'F' },
	[AO_PS2_G] = { 'g', 'G' },
	[AO_PS2_H] = { 'h', 'H' },
	[AO_PS2_I] = { 'i', 'I' },
	[AO_PS2_J] = { 'j', 'J' },
	[AO_PS2_K] = { 'k', 'K' },
	[AO_PS2_L] = { 'l', 'L' },
	[AO_PS2_M] = { 'm', 'M' },
	[AO_PS2_N] = { 'n', 'N' },
	[AO_PS2_O] = { 'o', 'O' },
	[AO_PS2_P] = { 'p', 'P' },
	[AO_PS2_Q] = { 'q', 'Q' },
	[AO_PS2_R] = { 'r', 'R' },
	[AO_PS2_S] = { 's', 'S' },
	[AO_PS2_T] = { 't', 'T' },
	[AO_PS2_U] = { 'u', 'U' },
	[AO_PS2_V] = { 'v', 'V' },
	[AO_PS2_W] = { 'w', 'W' },
	[AO_PS2_X] = { 'x', 'X' },
	[AO_PS2_Y] = { 'y', 'Y' },
	[AO_PS2_Z] = { 'z', 'Z' },

	[AO_PS2_0] = { '0', ')' },
	[AO_PS2_1] = { '1', '!' },
	[AO_PS2_2] = { '2', '@' },
	[AO_PS2_3] = { '3', '#' },
	[AO_PS2_4] = { '4', '$' },
	[AO_PS2_5] = { '5', '%' },
	[AO_PS2_6] = { '6', '^' },
	[AO_PS2_7] = { '7', '&' },
	[AO_PS2_8] = { '8', '*' },
	[AO_PS2_9] = { '9', '(' },

	[AO_PS2_GRAVE] = { '`', '~' },
	[AO_PS2_HYPHEN] = { '-', '_' },
	[AO_PS2_EQUAL] = { '=', '+' },
	[AO_PS2_BACKSLASH] = { '\\', '|' },
	[AO_PS2_BACKSPACE] = { '\010', '\010' },
	[AO_PS2_SPACE] = { ' ', ' ' },
	[AO_PS2_TAB] = { '\t', '\t' },

	[AO_PS2_ENTER] = { '\r', '\r' },
	[AO_PS2_ESC] = { '\033', '\033' },

	[AO_PS2_OPEN_SQ] = { '[', '{' },
	[AO_PS2_DELETE] = { '\177', '\177' },

	[AO_PS2_KP_TIMES] = { '*', '*' },
	[AO_PS2_KP_PLUS] = { '+', '+' },
	[AO_PS2_KP_ENTER] = { '\r', '\r' },
	[AO_PS2_KP_DECIMAL] = { '.', '.' },
	[AO_PS2_KP_0] = { '0', '0' },
	[AO_PS2_KP_1] = { '1', '1' },
	[AO_PS2_KP_2] = { '2', '2' },
	[AO_PS2_KP_3] = { '3', '3' },
	[AO_PS2_KP_4] = { '4', '4' },
	[AO_PS2_KP_5] = { '5', '5' },
	[AO_PS2_KP_6] = { '6', '6' },
	[AO_PS2_KP_7] = { '7', '7' },
	[AO_PS2_KP_8] = { '8', '8' },
	[AO_PS2_KP_9] = { '9', '9' },
	[AO_PS2_CLOSE_SQ] = { ']', '}' },
	[AO_PS2_SEMICOLON] = { ';', ':' },
	[AO_PS2_ACUTE] = { '\'', '"' },
	[AO_PS2_COMMA] = { ',', '<' },
	[AO_PS2_PERIOD] = { '.', '>' },
	[AO_PS2_SLASH] = { '/', '?' },
};

int
ao_ps2_ascii(uint8_t key)
{
	uint8_t	col;
	char a;

	/* Skip key releases */
	if (key & 0x80)
		return AO_READ_AGAIN;

	col = 0;
	if (ao_ps2_is_down(AO_PS2_L_SHIFT) || ao_ps2_is_down(AO_PS2_R_SHIFT))
		col = 1;

	/* caps lock */
	a = ao_ps2_asciimap[key][0];
	if (!a)
		return AO_READ_AGAIN;

	if ('a' <= a && a <= 'z')
		if (ao_ps2_is_down(AO_PS2_CAPS_LOCK))
			col ^= 1;
	a = ao_ps2_asciimap[key][col];
	if ('@' <= a && a <= 0x7f && (ao_ps2_is_down(AO_PS2_L_CTRL) || ao_ps2_is_down(AO_PS2_R_CTRL)))
		a &= 0x1f;
	return a;
}

int
_ao_ps2_pollchar(void)
{
	int	key;

	key = _ao_ps2_poll_key();
	if (key < 0)
		return key;
	return ao_ps2_ascii(key);
}

char
ao_ps2_getchar(void)
{
	int	c;
	ao_arch_block_interrupts();
	while ((c = _ao_ps2_pollchar()) == AO_READ_AGAIN)
		ao_sleep(&ao_ps2_rx_fifo);
	ao_arch_release_interrupts();
	return (char) c;
}

static void
ao_ps2_isr(void)
{
	uint8_t	bit;

	if (ao_ps2_tx_count) {
		ao_gpio_set(AO_PS2_DATA_PORT, AO_PS2_DATA_BIT, ao_ps2_tx&1);
		ao_ps2_tx >>= 1;
		ao_ps2_tx_count--;
		if (!ao_ps2_tx_count) {
			ao_enable_input(AO_PS2_DATA_PORT, AO_PS2_DATA_BIT, AO_EXTI_MODE_PULL_UP);
			ao_wakeup(&ao_ps2_tx_count);
		}
		return;
	}
	/* reset if its been a while */
	if ((ao_tick_count - ao_ps2_tick) > AO_MS_TO_TICKS(100))
		ao_ps2_count = 0;
	ao_ps2_tick = ao_tick_count;

	bit = ao_gpio_get(AO_PS2_DATA_PORT, AO_PS2_DATA_BIT);
	if (ao_ps2_count == 0) {
		/* check for start bit, ignore if not zero */
		if (bit)
			return;
		ao_ps2_value = 0;
	} else if (ao_ps2_count < 9) {
		ao_ps2_value |= (bit << (ao_ps2_count - 1));
	} else if (ao_ps2_count == 10) {
		ao_fifo_insert(ao_ps2_rx_fifo, ao_ps2_value);
		ao_wakeup(&ao_ps2_rx_fifo);
		if (ao_ps2_stdin)
			ao_wakeup(&ao_stdin_ready);
		ao_ps2_count = 0;
		return;
	}
	ao_ps2_count++;
}

void
ao_ps2_init(void)
{
	ao_enable_input(AO_PS2_DATA_PORT, AO_PS2_DATA_BIT,
			AO_EXTI_MODE_PULL_UP);

	ao_enable_port(AO_PS2_CLOCK_PORT);

	ao_exti_setup(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT,
		      AO_PS2_CLOCK_MODE(AO_EXTI_MODE_PULL_UP),
		      ao_ps2_isr);
	ao_exti_enable(AO_PS2_CLOCK_PORT, AO_PS2_CLOCK_BIT);

	ao_ps2_scancode_set = 2;
}
