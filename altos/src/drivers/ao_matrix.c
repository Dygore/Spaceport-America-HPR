/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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

#include <ao.h>
#include <ao_matrix.h>
#include <ao_event.h>
#include <ao_exti.h>

#define row_port(q)	AO_MATRIX_ROW_ ## q ## _PORT
#define row_bit(q) 	AO_MATRIX_ROW_ ## q ## _PIN
#define row_pin(q) 	AO_MATRIX_ROW_ ## q ## _PIN

#define col_port(q)	AO_MATRIX_COL_ ## q ## _PORT
#define col_bit(q) 	AO_MATRIX_COL_ ## q ## _PIN
#define col_pin(q) 	AO_MATRIX_COL_ ## q ## _PIN

static void
_ao_matrix_drive_row(uint8_t row, uint8_t val)
{
	switch (row) {
#define drive(n) case n: ao_gpio_set(row_port(n), row_bit(n), row_pin(n), val); break
		drive(0);
#if AO_MATRIX_ROWS > 1
		drive(1);
#endif
#if AO_MATRIX_ROWS > 2
		drive(2);
#endif
#if AO_MATRIX_ROWS > 3
		drive(3);
#endif
#if AO_MATRIX_ROWS > 4
		drive(4);
#endif
#if AO_MATRIX_ROWS > 5
		drive(5);
#endif
#if AO_MATRIX_ROWS > 6
		drive(6);
#endif
#if AO_MATRIX_ROWS > 7
		drive(7);
#endif
	}
}

static uint8_t
_ao_matrix_read_cols(void)
{
	uint8_t	v = 0;
#define read(n)	(v |= ao_gpio_get(col_port(n), col_bit(n), col_pin(n)) << n)

	read(0);
#if AO_MATRIX_ROWS > 1
	read(1);
#endif
#if AO_MATRIX_ROWS > 2
	read(2);
#endif
#if AO_MATRIX_ROWS > 3
	read(3);
#endif
#if AO_MATRIX_ROWS > 4
	read(4);
#endif
#if AO_MATRIX_ROWS > 5
	read(5);
#endif
#if AO_MATRIX_ROWS > 6
	read(6);
#endif
#if AO_MATRIX_ROWS > 7
	read(7);
#endif
	return v;
}

static uint8_t
_ao_matrix_read(uint8_t row) {
	uint8_t	state;
	_ao_matrix_drive_row(row, 0);
	state = _ao_matrix_read_cols();
	_ao_matrix_drive_row(row, 1);
	return state;
}

#define AO_MATRIX_DEBOUNCE_INTERVAL	AO_MS_TO_TICKS(50)

static uint8_t ao_matrix_keymap[AO_MATRIX_ROWS][AO_MATRIX_COLS] = AO_MATRIX_KEYCODES;

static uint8_t		ao_matrix_state[AO_MATRIX_ROWS];
static AO_TICK_TYPE	ao_matrix_tick[AO_MATRIX_ROWS];

static void
_ao_matrix_poll_one(uint8_t row) {
	uint8_t	state = _ao_matrix_read(row);

	if (state != ao_matrix_state[row]) {
		AO_TICK_TYPE	now = ao_time();

		if ((now - ao_matrix_tick[row]) >= AO_MATRIX_DEBOUNCE_INTERVAL) {
			uint8_t col;
			uint8_t changes = state ^ ao_matrix_state[row];

			for (col = 0; col < AO_MATRIX_COLS; col++) {
				if (changes & (1 << col)) {
					ao_event_put_isr(AO_EVENT_KEY,
							 ao_matrix_keymap[row][col],
							 ((state >> col) & 1) == 0);
				}
			}
			ao_matrix_state[row] = state;
		}
		ao_matrix_tick[row] = now;
	}
}

void
ao_matrix_poll(void)
{
	uint8_t	row;

	for (row = 0; row < AO_MATRIX_ROWS; row++)
		_ao_matrix_poll_one(row);
}

#define init_row(b) do {						\
		ao_enable_output(row_port(b), row_bit(b), row_pin(v), 1); \
		ao_gpio_set_output_mode(row_port(b), row_bit(b), row_pin(b), AO_OUTPUT_OPEN_DRAIN); \
	} while (0)

#define init_col(b) do { \
		ao_enable_input(col_port(b), col_bit(b), AO_EXTI_MODE_PULL_UP); \
	} while(0)

void
ao_matrix_init(void)
{
	uint8_t	row;

	init_row(0);
#if AO_MATRIX_ROWS > 1
	init_row(1);
#endif
#if AO_MATRIX_ROWS > 2
	init_row(2);
#endif
#if AO_MATRIX_ROWS > 3
	init_row(3);
#endif
#if AO_MATRIX_ROWS > 4
	init_row(4);
#endif
#if AO_MATRIX_ROWS > 5
	init_row(5);
#endif
#if AO_MATRIX_ROWS > 6
	init_row(6);
#endif
#if AO_MATRIX_ROWS > 7
	init_row(7);
#endif

	init_col(0);
#if AO_MATRIX_COLS > 1
	init_col(1);
#endif
#if AO_MATRIX_COLS > 2
	init_col(2);
#endif
#if AO_MATRIX_COLS > 3
	init_col(3);
#endif
#if AO_MATRIX_COLS > 4
	init_col(4);
#endif
#if AO_MATRIX_COLS > 5
	init_col(5);
#endif
#if AO_MATRIX_COLS > 6
	init_col(6);
#endif
#if AO_MATRIX_COLS > 7
	init_col(7);
#endif
	for (row = 0; row < AO_MATRIX_ROWS; row++) {
		ao_matrix_state[row] = _ao_matrix_read(row);
		ao_matrix_tick[row] = ao_time();
	}
}
