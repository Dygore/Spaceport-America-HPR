/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_TASK_H_
#define _AO_TASK_H_

#include <ao_list.h>

#ifndef HAS_TASK_INFO
#define HAS_TASK_INFO 1
#endif

/* arm stacks must be 64-bit aligned */
#ifndef AO_STACK_ALIGNMENT
#ifdef __arm__
#define AO_STACK_ALIGNMENT __attribute__ ((aligned(8)))
#else
#define AO_STACK_ALIGNMENT
#endif
#endif

/* An AltOS task */
struct ao_task {
	void *wchan;			/* current wait channel (NULL if running) */
	AO_TICK_TYPE alarm;		/* abort ao_sleep time */
	uint8_t task_id;		/* unique id */
	/* Saved stack pointer */
	union {
		uint32_t	*sp32;
		uint8_t		*sp8;
	};
	const char *name;		/* task name */
	struct ao_list	queue;
	struct ao_list	alarm_queue;
	/* Provide both 32-bit and 8-bit stacks */
	union {
		uint32_t stack32[AO_STACK_SIZE>>2];
		uint8_t stack8[AO_STACK_SIZE];
	} AO_STACK_ALIGNMENT;
#if HAS_SAMPLE_PROFILE
	uint32_t ticks;
	uint32_t yields;
	uint16_t start;
	uint16_t max_run;
#endif
};

#ifndef AO_NUM_TASKS
#define AO_NUM_TASKS		16	/* maximum number of tasks */
#endif

extern struct ao_task * ao_tasks[AO_NUM_TASKS];
extern uint8_t ao_num_tasks;
extern struct ao_task *ao_cur_task;
extern uint8_t ao_task_minimize_latency;	/* Reduce IRQ latency */

#ifndef HAS_ARCH_VALIDATE_CUR_STACK
#define ao_validate_cur_stack()
#endif

/*
 ao_task.c
 */

/* Suspend the current task until wchan is awoken.
 * returns:
 *  0 on normal wake
 *  1 on alarm
 */
uint8_t
ao_sleep(void *wchan);

/* Suspend the current task until wchan is awoken or the timeout
 * expires. returns:
 *  0 on normal wake
 *  1 on alarm
 */
uint8_t
ao_sleep_for(void *wchan, AO_TICK_TYPE timeout);

/* Wake all tasks sleeping on wchan */
void
ao_wakeup(void *wchan);

#if 0
/* set an alarm to go off in 'delay' ticks */
void
ao_alarm(AO_TICK_TYPE delay);

/* Clear any pending alarm */
void
ao_clear_alarm(void);
#endif

/* Yield the processor to another task */
void
ao_yield(void) ao_arch_naked_declare;

/* Add a task to the run queue */
void
ao_add_task(struct ao_task * task, void (*start)(void), const char *name);

/* Called on timer interrupt to check alarms */
extern AO_TICK_TYPE		ao_task_alarm_tick;
extern volatile AO_TICK_TYPE	ao_tick_count;

void
ao_task_alarm(AO_TICK_TYPE tick);

static inline void
ao_task_check_alarm(void) {
#if HAS_TASK
	if ((AO_TICK_SIGNED) (ao_tick_count - ao_task_alarm_tick) >= 0)
		ao_task_alarm(ao_tick_count);
#endif
}

/* Terminate the current task */
void
ao_exit(void) __attribute__ ((noreturn));

/* Dump task info to console */
void
ao_task_info(void);

/* Start the scheduler. This will not return */
void
ao_start_scheduler(void) __attribute__((noreturn));

void
ao_task_init(void);

#endif
