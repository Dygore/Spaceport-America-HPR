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
#include <ao_log.h>
#include <ao_config.h>
#if HAS_TRACKER
#include <ao_tracker.h>
#endif

uint8_t	ao_log_mutex;
uint32_t ao_log_current_pos;
uint32_t ao_log_end_pos;
uint32_t ao_log_start_pos;
uint8_t	ao_log_running;
enum ao_flight_state ao_log_state;
uint16_t ao_flight_number;

void
ao_log_flush(void)
{
	ao_storage_flush();
}

/*
 * When erasing a flight log, make sure the config block
 * has an up-to-date version of the current flight number
 */

struct ao_log_erase {
	uint8_t	mark;
	uint16_t flight;
};

static struct ao_log_erase erase;

#ifndef LOG_MAX_ERASE
#define LOG_MAX_ERASE	16
#endif

#ifndef LOG_ERASE_MARK
#if USE_EEPROM_CONFIG
#error "Must define LOG_ERASE_MARK with USE_EEPROM_CONFIG"
#endif
#define LOG_ERASE_MARK	0x00
#endif

static uint32_t
ao_log_erase_pos(uint8_t i)
{
	return i * sizeof (struct ao_log_erase) + AO_CONFIG_MAX_SIZE;
}

void
ao_log_write_erase(uint8_t pos)
{
	erase.mark = LOG_ERASE_MARK;
	erase.flight = ao_flight_number;
	ao_config_write(ao_log_erase_pos(pos),  &erase, sizeof (erase));

#if USE_EEPROM_CONFIG
	if (pos == 0) {
		uint8_t	i;
		for (i = 1; i < LOG_MAX_ERASE; i++) {
			erase.mark = (uint8_t) ~LOG_ERASE_MARK;
			erase.flight = 0;
			ao_config_write(ao_log_erase_pos(i), &erase, sizeof (erase));
		}
	}
#endif

	ao_config_flush();
}

static void
ao_log_read_erase(uint8_t pos)
{
	ao_config_read(ao_log_erase_pos(pos), &erase, sizeof (erase));
}


static void
ao_log_erase_mark(void)
{
	uint8_t				i;

	for (i = 0; i < LOG_MAX_ERASE; i++) {
		ao_log_read_erase(i);
		if (erase.mark == LOG_ERASE_MARK && erase.flight == ao_flight_number)
			return;
		if (erase.mark != LOG_ERASE_MARK) {
			ao_log_write_erase(i);
			return;
		}
	}
	ao_config_put();
}

/* Position of first flight record in slot */
static uint32_t
ao_log_pos(uint8_t slot)
{
	return ((slot) * ao_config.flight_log_max);
}

/* Start of erase block containing first flight record */
static uint32_t
ao_log_pos_block_start(uint8_t slot)
{
	return ao_log_pos(slot) & ~(ao_storage_block - 1);
}

/* End of erase block containing last flight record */
static uint32_t
ao_log_pos_block_end(uint8_t slot)
{
	return ao_log_pos_block_start(slot + 1);
}

#ifndef AO_LOG_UNCOMMON
/*
 * Common logging functions which depend on the type of the log data
 * structure.
 */

ao_log_type ao_log_data;

static uint8_t
ao_log_csum(uint8_t *b) 
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (ao_log_type); i++)
		sum += *b++;
	return -sum;
}

uint8_t
ao_log_write(ao_log_type *log) 
{
	uint8_t wrote = 0;
	/* set checksum */
	log->csum = 0;
	log->csum = ao_log_csum((uint8_t *) log);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 log,
					 sizeof (ao_log_type));
			ao_log_current_pos += sizeof (ao_log_type);
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

uint8_t
ao_log_check_data(void)
{
	if (ao_log_csum((uint8_t *) &ao_log_data) != 0)
		return 0;
	return 1;
}

int32_t
ao_log_flight(uint8_t slot)
{
	if (ao_storage_is_erased(ao_log_pos_block_start(slot)))
		return 0;

	if (!ao_storage_read(ao_log_pos(slot),
			     &ao_log_data,
			     sizeof (ao_log_type)))
		return -(int32_t) (slot + 1);

	if (!ao_log_check_data() || ao_log_data.type != AO_LOG_FLIGHT)
		return -(int32_t) (slot + 1);

	return (int32_t) ao_log_data.u.flight.flight;
}
#endif

static uint8_t
ao_log_slots(void)
{
	return (uint8_t) (ao_storage_log_max / ao_config.flight_log_max);
}

static uint16_t
ao_log_max_flight(void)
{
	uint8_t		log_slot;
	uint8_t		log_slots;
	int32_t		log_flight;
	uint16_t	max_flight = 0;

	/* Scan the log space looking for the biggest flight number */
	log_slots = ao_log_slots();
	for (log_slot = 0; log_slot < log_slots; log_slot++) {
		log_flight = ao_log_flight(log_slot);
		if (log_flight <= 0)
			continue;
		if (max_flight == 0 || log_flight > max_flight)
			max_flight = (uint16_t) log_flight;
	}
	return max_flight;
}

static uint8_t
ao_log_erase(uint8_t slot) 
{
	uint32_t start_pos;
	uint32_t end_pos;

	ao_log_erase_mark();
	start_pos = ao_log_pos_block_start(slot);
	end_pos = ao_log_pos_block_end(slot);
	return ao_storage_erase(start_pos, end_pos - start_pos);
}

static void
ao_log_find_max_erase_flight(void) 
{
	uint8_t	log_slot;

	/* Now look through the log of flight numbers from erase operations and
	 * see if the last one is bigger than what we found above
	 */
	for (log_slot = LOG_MAX_ERASE; log_slot-- > 0;) {
		ao_log_read_erase(log_slot);
		if (erase.mark == LOG_ERASE_MARK) {
			if (ao_flight_number == 0 ||
			    (int16_t) (erase.flight - ao_flight_number) > 0)
				ao_flight_number = erase.flight;
			break;
		}
	}
	if (ao_flight_number == 0)
		ao_flight_number = 1;
}

uint8_t
ao_log_scan(void) 
{
	uint8_t		log_slot;
	uint8_t		log_slots;
#if FLIGHT_LOG_APPEND
	uint8_t		ret;
#else
	uint8_t		log_want;
#endif

	ao_config_get();

	/* Get any existing flight number */
	ao_flight_number = ao_log_max_flight();

#if FLIGHT_LOG_APPEND

	/* Deal with older OS versions which stored multiple
	 * flights in rom by erasing everything after the first
	 * slot
	 */
	if (ao_config.flight_log_max != ao_storage_log_max) {
		log_slots = ao_log_slots();
		for (log_slot = 1; log_slot < log_slots; log_slot++) {
			if (ao_log_flight(log_slot) != 0)
				if (!ao_log_erase(log_slot))
					printf("erase %d failed\n", log_slot);
		}
		ao_config_log_fix_append();
	}
	ao_log_current_pos = ao_log_pos(0);
	ao_log_end_pos = ao_log_pos_block_end(0);

	if (ao_flight_number) {
		uint32_t	full = (ao_log_current_pos) / AO_LOG_SIZE;
		uint32_t	empty = (ao_log_end_pos - AO_LOG_SIZE) / AO_LOG_SIZE;

		/* If there's already a flight started, then find the
		 * end of it
		 */
		for (;;) {
			uint32_t current = (full + empty) >> 1;
			ao_log_current_pos = current * AO_LOG_SIZE;

			if (current == full) {
				if (ao_log_check(ao_log_current_pos) != AO_LOG_EMPTY)
					ao_log_current_pos += AO_LOG_SIZE;
				break;
			}
			if (current == empty)
				break;

			if (ao_log_check(ao_log_current_pos) != AO_LOG_EMPTY) {
				full = current;
			} else {
				empty = current;
			}
		}
		ret = 1;
	} else {
		ao_log_find_max_erase_flight();
		ret = 0;
	}
	ao_wakeup(&ao_flight_number);
	return ret;
#else
	if (ao_flight_number) {
		++ao_flight_number;
		if (ao_flight_number <= 0)
			ao_flight_number = 1;
	}

	ao_log_find_max_erase_flight();

	/* With a flight number in hand, find a place to write a new log,
	 * use the target flight number to index the available log slots so
	 * that we write logs to each spot about the same number of times.
	 */

	/* Find a log slot for the next flight, if available */
	ao_log_current_pos = ao_log_end_pos = 0;
	log_slots = ao_log_slots();
	log_want = (uint8_t) ((ao_flight_number - 1) % log_slots);
	log_slot = log_want;
	do {
		if (ao_log_flight(log_slot) == 0) {
			ao_log_current_pos = ao_log_pos(log_slot);
			ao_log_end_pos = ao_log_pos_block_end(log_slot);
			break;
		}
		if (++log_slot >= log_slots)
			log_slot = 0;
	} while (log_slot != log_want);
	ao_wakeup(&ao_flight_number);
	return 0;
#endif
}

void
ao_log_start(void)
{
	/* start logging */
	ao_log_running = 1;
	ao_wakeup(&ao_log_running);
}

void
ao_log_stop(void)
{
	ao_log_running = 0;
	ao_log_flush();
}

uint8_t
ao_log_present(void)
{
	return ao_log_max_flight() != 0;
}

uint8_t
ao_log_full(void)
{
	return ao_log_current_pos == ao_log_end_pos;
}

#ifndef LOG_ADC
#define LOG_ADC	HAS_ADC
#endif

#if LOG_ADC
static struct ao_task ao_log_task;
#endif

static void
ao_log_list(void) 
{
	uint8_t	slot;
	uint8_t slots;
	int32_t flight;

	slots = ao_log_slots();
	for (slot = 0; slot < slots; slot++)
	{
		flight = ao_log_flight(slot);
		if (flight)
			printf ("flight %ld start %x end %x\n",
				flight,
				(uint16_t) (ao_log_pos(slot) >> 8),
				(uint16_t) (ao_log_pos_block_end(slot) >> 8));
		else
			printf ("slot %d  start %x end %x\n",
				slot,
				(uint16_t) (ao_log_pos(slot) >> 8),
				(uint16_t) (ao_log_pos_block_end(slot) >> 8));
	}
	printf ("done\n");
}

static void
ao_log_delete(void) 
{
	uint8_t slot;
	uint8_t slots;
	int32_t cmd_flight = 1;

	ao_cmd_white();
	if (ao_cmd_lex_c == '-') {
		cmd_flight = -1;
		ao_cmd_lex();
	}
	cmd_flight *= (int32_t) ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;

	slots = ao_log_slots();
	/* Look for the flight log matching the requested flight */
	if (cmd_flight) {
		for (slot = 0; slot < slots; slot++) {
			if (ao_log_flight(slot) == cmd_flight) {
#if HAS_TRACKER
				ao_tracker_erase_start(cmd_flight);
#endif
				if (ao_log_erase(slot))
					puts("Erased");
				else
					puts("Failed to erase");
#if HAS_TRACKER
				ao_tracker_erase_end();
#endif
				return;
			}
		}
	}
	printf("No such flight: %ld\n", cmd_flight);
}

const struct ao_cmds ao_log_cmds[] = {
	{ ao_log_list,	"l\0List logs" },
	{ ao_log_delete,	"d <flight-number>\0Delete flight" },
	{ 0,	NULL },
};

void
ao_log_init(void)
{
	ao_log_running = 0;

	/* For now, just log the flight starting at the begining of eeprom */
	ao_log_state = ao_flight_invalid;

	ao_cmd_register(&ao_log_cmds[0]);

#ifndef HAS_ADC
#error Define HAS_ADC for ao_log.c
#endif
#if LOG_ADC
	/* Create a task to log events to eeprom */
	ao_add_task(&ao_log_task, ao_log, "log");
#endif
}
