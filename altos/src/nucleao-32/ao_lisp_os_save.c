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
#include <ao_lisp.h>
#include <ao_flash.h>

extern uint8_t	__flash__[] __attribute__((aligned(4)));

/* saved variables to rebuild the heap

   ao_lisp_atoms
   ao_lisp_frame_global
 */

int
ao_lisp_os_save(void)
{
	int i;

	for (i = 0; i < AO_LISP_POOL_TOTAL; i += 256) {
		uint32_t	*dst = (uint32_t *) (void *) &__flash__[i];
		uint32_t	*src = (uint32_t *) (void *) &ao_lisp_pool[i];

		ao_flash_page(dst, src);
	}
	return 1;
}

int
ao_lisp_os_restore_save(struct ao_lisp_os_save *save, int offset)
{
	memcpy(save, &__flash__[offset], sizeof (struct ao_lisp_os_save));
	return 1;
}

int
ao_lisp_os_restore(void)
{
	memcpy(ao_lisp_pool, __flash__, AO_LISP_POOL_TOTAL);
	return 1;
}
