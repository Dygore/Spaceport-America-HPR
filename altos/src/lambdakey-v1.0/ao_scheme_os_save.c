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

#include <ao.h>
#include <ao_scheme.h>
#include <ao_flash.h>

extern uint8_t	__flash__[];

/* saved variables to rebuild the heap

   ao_scheme_atoms
   ao_scheme_frame_global
 */

int
ao_scheme_os_save(void)
{
	int i;

	for (i = 0; i < AO_SCHEME_POOL_TOTAL; i += 256) {
		void	*dst = &__flash__[i];
		void	*src = &ao_scheme_pool[i];

		ao_flash_page(dst, src);
	}
	return 1;
}

int
ao_scheme_os_restore_save(struct ao_scheme_os_save *save, int offset)
{
	memcpy(save, &__flash__[offset], sizeof (struct ao_scheme_os_save));
	return 1;
}

int
ao_scheme_os_restore(void)
{
	memcpy(ao_scheme_pool, __flash__, AO_SCHEME_POOL_TOTAL);
	return 1;
}
