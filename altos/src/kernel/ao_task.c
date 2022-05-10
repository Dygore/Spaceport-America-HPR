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

#include <ao.h>
#include <ao_task.h>
#if HAS_SAMPLE_PROFILE
#include <ao_sample_profile.h>
#endif
#if HAS_STACK_GUARD
#include <ao_mpu.h>
#endif
#include <picotls.h>

#define DEBUG	0

struct ao_task * ao_tasks[AO_NUM_TASKS];
uint8_t ao_num_tasks;
struct ao_task *ao_cur_task;

#ifdef ao_arch_task_globals
ao_arch_task_globals
#endif

#define AO_CHECK_STACK	0

#if AO_CHECK_STACK
static uint8_t	in_yield;

static inline void ao_check_stack(void) {
	uint8_t	q;
	if (!in_yield && ao_cur_task && &q < &ao_cur_task->stack[0])
		ao_panic(AO_PANIC_STACK);
}
#else
#define ao_check_stack()
#endif

#if DEBUG
#define ao_task_irq_check()	ao_arch_irq_check()
#else
#define ao_task_irq_check()
#endif

#ifndef SLEEP_HASH_SIZE
#define SLEEP_HASH_SIZE	17
#endif

static struct ao_list	run_queue;
static struct ao_list	alarm_queue;
static struct ao_list	ao_sleep_queue[SLEEP_HASH_SIZE];

static void
_ao_task_to_run_queue(struct ao_task *task)
{
	ao_task_irq_check();
	ao_list_del(&task->queue);
	ao_list_append(&task->queue, &run_queue);
}

static struct ao_list *
ao_task_sleep_queue(void *wchan)
{
	return &ao_sleep_queue[(uintptr_t) wchan % SLEEP_HASH_SIZE];
}

static void
_ao_task_to_sleep_queue(struct ao_task *task, void *wchan)
{
	ao_task_irq_check();
	ao_list_del(&task->queue);
	ao_list_append(&task->queue, ao_task_sleep_queue(wchan));
}

#if DEBUG
static void
ao_task_validate_alarm_queue(void)
{
	struct ao_task	*alarm, *prev = NULL;
	int		i;

	if (ao_list_is_empty(&alarm_queue))
		return;
	ao_list_for_each_entry(alarm, &alarm_queue, struct ao_task, alarm_queue) {
		if (prev) {
			if ((int16_t) (alarm->alarm - prev->alarm) < 0) {
				ao_panic(1);
			}
		}
		prev = alarm;
	}
	for (i = 0; i < ao_num_tasks; i++) {
		alarm = ao_tasks[i];
		if (alarm->alarm) {
			if (ao_list_is_empty(&alarm->alarm_queue))
				ao_panic(2);
		} else {
			if (!ao_list_is_empty(&alarm->alarm_queue))
				ao_panic(3);
		}
	}
	if (ao_task_alarm_tick != ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm)
		ao_panic(4);
}
#else
#define ao_task_validate_alarm_queue()
#endif

AO_TICK_TYPE	ao_task_alarm_tick;

static void
_ao_task_to_alarm_queue(struct ao_task *task)
{
	struct ao_task	*alarm;
	ao_task_irq_check();
	ao_list_for_each_entry(alarm, &alarm_queue, struct ao_task, alarm_queue) {
		if ((int16_t) (alarm->alarm - task->alarm) >= 0) {
			ao_list_insert(&task->alarm_queue, alarm->alarm_queue.prev);
			ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
			ao_task_validate_alarm_queue();
			return;
		}
	}
	ao_list_append(&task->alarm_queue, &alarm_queue);
	ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
	ao_task_validate_alarm_queue();
}

static void
_ao_task_from_alarm_queue(struct ao_task *task)
{
	ao_task_irq_check();
	ao_list_del(&task->alarm_queue);
	if (ao_list_is_empty(&alarm_queue))
		ao_task_alarm_tick = 0;
	else
		ao_task_alarm_tick = ao_list_first_entry(&alarm_queue, struct ao_task, alarm_queue)->alarm;
	ao_task_validate_alarm_queue();
}

static void
ao_task_init_queue(struct ao_task *task)
{
	ao_list_init(&task->queue);
	ao_list_init(&task->alarm_queue);
}

static void
ao_task_exit_queue(struct ao_task *task)
{
	ao_task_irq_check();
	ao_list_del(&task->queue);
	ao_list_del(&task->alarm_queue);
}

void
ao_task_alarm(AO_TICK_TYPE tick)
{
	struct ao_task	*alarm, *next;

	ao_arch_critical(
		ao_list_for_each_entry_safe(alarm, next, &alarm_queue, struct ao_task, alarm_queue) {
			if ((AO_TICK_SIGNED) (tick - alarm->alarm) < 0)
				break;
			alarm->alarm = 0;
			_ao_task_from_alarm_queue(alarm);
			_ao_task_to_run_queue(alarm);
		});
}

void
ao_task_init(void)
{
	uint8_t	i;
	ao_list_init(&run_queue);
	ao_list_init(&alarm_queue);
	ao_task_alarm_tick = 0;
	for (i = 0; i < SLEEP_HASH_SIZE; i++)
		ao_list_init(&ao_sleep_queue[i]);
}

#if DEBUG
static uint8_t
ao_task_validate_queue(struct ao_task *task)
{
	uint32_t flags;
	struct ao_task *m;
	uint8_t ret = 0;
	struct ao_list *queue;

	flags = ao_arch_irqsave();
	if (task->wchan) {
		queue = ao_task_sleep_queue(task->wchan);
		ret |= 2;
	} else {
		queue = &run_queue;
		ret |= 4;
	}
	ao_list_for_each_entry(m, queue, struct ao_task, queue) {
		if (m == task) {
			ret |= 1;
			break;
		}
	}
	ao_arch_irqrestore(flags);
	return ret;
}

static uint8_t
ao_task_validate_alarm(struct ao_task *task)
{
	uint32_t	flags;
	struct ao_task	*m;
	uint8_t		ret = 0;

	flags = ao_arch_irqsave();
	if (task->alarm == 0)
		return 0xff;
	ao_list_for_each_entry(m, &alarm_queue, struct ao_task, alarm_queue) {
		if (m == task)
			ret |= 1;
		else {
			if (!(ret&1)) {
				if ((int16_t) (m->alarm - task->alarm) > 0)
					ret |= 2;
			} else {
				if ((int16_t) (task->alarm - m->alarm) > 0)
					ret |= 4;
			}
		}
	}
	ao_arch_irqrestore(flags);
	return ret;
}


static void
ao_task_validate(void)
{
	uint8_t		i;
	struct ao_task	*task;
	uint8_t		ret;

	for (i = 0; i < ao_num_tasks; i++) {
		task = ao_tasks[i];
		ret = ao_task_validate_queue(task);
		if (!(ret & 1)) {
			if (ret & 2)
				printf ("sleeping task not on sleep queue %s %08x\n",
					task->name, task->wchan);
			else
				printf ("running task not on run queue %s\n",
					task->name);
		}
		ret = ao_task_validate_alarm(task);
		if (ret != 0xff) {
			if (!(ret & 1))
				printf ("alarm task not on alarm queue %s %d\n",
					task->name, task->alarm);
			if (ret & 2)
				printf ("alarm queue has sooner entries after %s %d\n",
					task->name, task->alarm);
			if (ret & 4)
				printf ("alarm queue has later entries before %s %d\n",
					task->name, task->alarm);
		}
	}
}
#endif /* DEBUG */

static inline void *
ao_stack_top(struct ao_task *task)
{
	uint8_t *top = &task->stack8[AO_STACK_SIZE];

	/* Subtract off the TLS space, but keep the resulting
	 * stack 8-byte aligned
	 */
#if USE_TLS
	return top - ((_tls_size() + 7) & ~3);
#else
	return top;
#endif
}

void
ao_add_task(struct ao_task * task, void (*task_func)(void), const char *name) 
{
	uint8_t task_id;
	uint8_t t;
	if (ao_num_tasks == AO_NUM_TASKS)
		ao_panic(AO_PANIC_NO_TASK);
	for (task_id = 1; task_id != 0; task_id++) {
		for (t = 0; t < ao_num_tasks; t++)
			if (ao_tasks[t]->task_id == task_id)
				break;
		if (t == ao_num_tasks)
			break;
	}
	task->task_id = task_id;
	task->name = name;
	task->wchan = NULL;
	/*
	 * Construct a stack frame so that it will 'return'
	 * to the start of the task
	 */
	uint32_t *sp = ao_stack_top(task);
#if USE_TLS
	_init_tls(sp);
#endif
	ao_arch_init_stack(task, sp, task_func);
	ao_task_init_queue(task);
	ao_arch_critical(
		_ao_task_to_run_queue(task);
		ao_tasks[ao_num_tasks] = task;
		ao_num_tasks++;
		);
}

uint8_t	ao_task_minimize_latency;

/* Task switching function. */
void
ao_yield(void)
{
	if (ao_cur_task)
	{
#if HAS_SAMPLE_PROFILE
		AO_TICK_TYPE	tick = ao_sample_profile_timer_value();
		AO_TICK_TYPE	run = tick - ao_cur_task->start;
		if (run > ao_cur_task->max_run)
			ao_cur_task->max_run = run;
		++ao_cur_task->yields;
#endif
		ao_arch_save_regs();
		ao_arch_save_stack();
	}

	ao_arch_isr_stack();
	ao_arch_block_interrupts();

#if AO_CHECK_STACK
	in_yield = 1;
#endif
	/* Find a task to run. If there isn't any runnable task,
	 * this loop will run forever, which is just fine
	 */
	/* If the current task is running, move it to the
	 * end of the queue to allow other tasks a chance
	 */
	if (ao_cur_task && ao_cur_task->wchan == NULL)
		_ao_task_to_run_queue(ao_cur_task);
	for (;;) {
		ao_arch_memory_barrier();
		if (!ao_list_is_empty(&run_queue))
			break;
		/* Wait for interrupts when there's nothing ready */
		if (ao_task_minimize_latency) {
			ao_arch_release_interrupts();
			ao_arch_block_interrupts();
		} else
			ao_arch_wait_interrupt();
	}
	ao_cur_task = ao_list_first_entry(&run_queue, struct ao_task, queue);
#if HAS_SAMPLE_PROFILE
	ao_cur_task->start = ao_sample_profile_timer_value();
#endif
#if HAS_STACK_GUARD
	ao_mpu_stack_guard(ao_cur_task->stack);
#endif
#if AO_CHECK_STACK
	in_yield = 0;
#endif
#if USE_TLS
	_set_tls(ao_stack_top(ao_cur_task));
#endif
	ao_arch_restore_stack();
}

uint8_t
ao_sleep(void *wchan)
{
	ao_arch_critical(
		ao_cur_task->wchan = wchan;
		_ao_task_to_sleep_queue(ao_cur_task, wchan);
		);
	ao_yield();
	if (ao_cur_task->wchan) {
		ao_cur_task->wchan = NULL;
		ao_cur_task->alarm = 0;
		return 1;
	}
	return 0;
}

void
ao_wakeup(void *wchan) 
{
	ao_validate_cur_stack();
	struct ao_task	*sleep, *next;
	struct ao_list	*sleep_queue;
	uint32_t	flags;

	if (ao_num_tasks == 0)
		return;
	sleep_queue = ao_task_sleep_queue(wchan);
	flags = ao_arch_irqsave();
	ao_list_for_each_entry_safe(sleep, next, sleep_queue, struct ao_task, queue) {
		if (sleep->wchan == wchan) {
			sleep->wchan = NULL;
			_ao_task_to_run_queue(sleep);
		}
	}
	ao_arch_irqrestore(flags);
	ao_check_stack();
}

uint8_t
ao_sleep_for(void *wchan, AO_TICK_TYPE timeout)
{
	uint8_t	ret;
	if (timeout) {
		ao_arch_critical(
			/* Make sure we sleep *at least* delay ticks, which means adding
			 * one to account for the fact that we may be close to the next tick
			 */
			if (!(ao_cur_task->alarm = ao_time() + timeout + 1))
				ao_cur_task->alarm = 1;
			_ao_task_to_alarm_queue(ao_cur_task);
			);
	}
	ret = ao_sleep(wchan);
	if (timeout) {
		ao_arch_critical(
			ao_cur_task->alarm = 0;
			_ao_task_from_alarm_queue(ao_cur_task);
			);
	}
	return ret;
}

static uint8_t ao_forever;

void
ao_delay(AO_TICK_TYPE ticks)
{
	if (!ticks)
		ticks = 1;
	ao_sleep_for(&ao_forever, ticks);
}

void
ao_exit(void)
{
	uint8_t	i;
	ao_arch_block_interrupts();
	for (i = 0; i < ao_num_tasks; i++)
		if (ao_tasks[i] == ao_cur_task)
			break;
	ao_task_exit_queue(ao_cur_task);
	/* Remove task from list */
	ao_num_tasks--;
	for (; i < ao_num_tasks; i++)
		ao_tasks[i] = ao_tasks[i+1];
	ao_cur_task = NULL;
	ao_yield();
	__builtin_unreachable();
}

#if HAS_TASK_INFO
void
ao_task_info(void)
{
	uint8_t		i;
	struct ao_task *task;
	AO_TICK_TYPE	now = ao_time();

	for (i = 0; i < ao_num_tasks; i++) {
		task = ao_tasks[i];
		printf("%2d: wchan %08x alarm %5d %s\n",
		       task->task_id,
		       (int) task->wchan,
		       task->alarm ? (int16_t) (task->alarm - now) : 9999,
		       task->name);
	}
#if DEBUG
	ao_task_validate();
#endif
}
#endif

void
ao_start_scheduler(void)
{
	ao_cur_task = NULL;
#if HAS_ARCH_START_SCHEDULER
	ao_arch_start_scheduler();
#endif
	ao_yield();
	__builtin_unreachable();
}
