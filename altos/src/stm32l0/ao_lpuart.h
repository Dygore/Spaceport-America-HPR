/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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

char
ao_lpuart1_getchar(void);

void
ao_lpuart1_putchar(char c);

int
_ao_lpuart1_pollchar(void);

void
ao_lpuart1_drain(void);

void
ao_lpuart1_set_speed(uint8_t speed);

void
ao_lpuart1_enable(void);

void
ao_lpuart1_disable(void);
