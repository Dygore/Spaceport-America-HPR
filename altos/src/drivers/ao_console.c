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
#include "ao_console.h"
#include "ao_ps2.h"
#include "ao_vga.h"

static uint8_t	console_row, console_col;

#define ao_console_bitmap	ao_vga_bitmap

static uint8_t	console_rows, console_cols;

static void
ao_console_scroll(void)
{
	ao_copy(&ao_console_bitmap,
		0, 0,
		ao_console_bitmap.width,
		ao_console_bitmap.height - ao_font.height,
		&ao_console_bitmap,
		0, ao_font.height,
		AO_COPY);
	ao_rect(&ao_console_bitmap,
		0,
		(console_rows - 1) * ao_font.height,
		ao_console_bitmap.width,
		ao_font.height,
		1,
		AO_COPY);
}

static void
ao_console_cursor(void)
{
	ao_rect(&ao_console_bitmap,
		console_col * ao_font.width,
		console_row * ao_font.height,
		ao_font.width,
		ao_font.height,
		1,
		AO_XOR);
}

static void
ao_console_clear(void)
{
	ao_rect(&ao_console_bitmap,
		0, 0,
		ao_console_bitmap.width,
		ao_console_bitmap.height,
		1,
		AO_COPY);
}

static void
ao_console_space(void)
{
	ao_rect(&ao_console_bitmap,
		console_col * ao_font.width,
		console_row * ao_font.height,
		ao_font.width,
		ao_font.height,
		1,
		AO_COPY);
}

static void
ao_console_newline(void)
{
	if (++console_row == console_rows) {
		ao_console_scroll();
		console_row--;
	}
}

void
ao_console_putchar(char c)
{
	if (' ' <= c && c < 0x7f) {
		char	text[2];
		ao_console_space();
		text[0] = c;
		text[1] = '\0';
		ao_text(&ao_console_bitmap,
			console_col * ao_font.width,
			console_row * ao_font.height + ao_font.ascent,
			text,
			0,
			AO_COPY);
		if (++console_col == console_cols) {
			console_col = 0;
			ao_console_newline();
		}
	} else {
		ao_console_cursor();
		switch (c) {
		case '\r':
			console_col = 0;
			break;
		case '\t':
			console_col += 8 - (console_col & 7);
			if (console_col >= console_cols) {
				console_col = 0;
				ao_console_newline();
			}
			break;
		case '\n':
			ao_console_newline();
			break;
		case '\f':
			console_col = console_row = 0;
			ao_console_clear();
			break;
		case '\177':
		case '\010':
			if (console_col)
				console_col--;
			break;
		}
	}
	ao_console_cursor();
}

void
ao_console_init(void)
{
	console_cols = ao_console_bitmap.width / ao_font.width;
	console_rows = ao_console_bitmap.height / ao_font.height;
#if CONSOLE_STDIN
	ao_ps2_stdin = 1;
	ao_add_stdio(_ao_ps2_pollchar,
		     ao_console_putchar,
		     NULL);
#endif
	ao_console_clear();
	ao_console_cursor();
	ao_vga_enable(1);
}
