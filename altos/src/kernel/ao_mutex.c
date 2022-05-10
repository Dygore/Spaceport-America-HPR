/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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

#include "ao.h"

#ifndef HAS_MUTEX_TRY
#define HAS_MUTEX_TRY 1
#endif

#if HAS_MUTEX_TRY

uint8_t
ao_mutex_try(uint8_t *mutex, uint8_t task_id) 
{
	uint8_t	ret;
	if (*mutex == task_id)
		ao_panic(AO_PANIC_MUTEX);
	ao_arch_critical(
		if (*mutex)
			ret = 0;
		else {
			*mutex = task_id;
			ret = 1;
		});
	return ret;
}
#endif

void
ao_mutex_get(uint8_t *mutex) 
{
	if (*mutex == ao_cur_task->task_id)
		ao_panic(AO_PANIC_MUTEX);
	ao_arch_critical(
		while (*mutex)
			ao_sleep(mutex);
		*mutex = ao_cur_task->task_id;
		);
}

void
ao_mutex_put(uint8_t *mutex) 
{
	if (*mutex != ao_cur_task->task_id)
		ao_panic(AO_PANIC_MUTEX);
	ao_arch_critical(
		*mutex = 0;
		ao_wakeup(mutex);
		);
}
