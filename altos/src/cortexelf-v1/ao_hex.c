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
#include "ao_hex.h"
#include "ao_as1107.h"
#include "ao_1802.h"

static struct ao_task	ao_hex_task;

static void
ao_hex(void)
{
	for (;;) {
		ao_as1107_write_16(0, ADDRESS);
		ao_as1107_write_8(6, DATA);
		ao_sleep(&ADDRESS);
	}
}

void
ao_hex_init(void)
{
	ao_add_task(&ao_hex_task, ao_hex, "hex");
}
