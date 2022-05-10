/*
 * Copyright © 2017 Bdale Garbee <bdale@gag.com>
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
#include <ao_data.h>
#include <ao_flight.h>

static struct ao_log_firetwo ao_fireone_data;

const uint8_t ao_log_format = AO_LOG_FORMAT_TELEFIRETWO;

static uint8_t
ao_log_csum(uint8_t *b) 
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_firetwo); i++)
		sum += *b++;
	return -sum;
}

static uint8_t
ao_log_firetwo(void)
{
	uint8_t wrote = 0;
	/* set checksum */
	ao_fireone_data.csum = 0;
	ao_fireone_data.csum = ao_log_csum((uint8_t *) &ao_fireone_data);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 &ao_fireone_data,
					 sizeof (struct ao_log_firetwo));
			ao_log_current_pos += sizeof (struct ao_log_firetwo);
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

#if HAS_ADC
static uint8_t	ao_fireone_data_pos;

/* a hack to make sure that ao_log_metrums fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_firetwo))] ;
#endif

void
ao_log(void)
{
	ao_storage_setup();

	do {
		ao_log_scan();
	
		while (!ao_log_running)
			ao_sleep(&ao_log_running);
	
		ao_fireone_data.type = AO_LOG_FLIGHT;
		ao_fireone_data.tick = (uint16_t) ao_time();
		ao_fireone_data.u.flight.flight = ao_flight_number;
		ao_log_firetwo();

		/* Write the whole contents of the ring to the log
	 	* when starting up.
	 	*/
		ao_fireone_data_pos = ao_data_ring_next(ao_data_head);
		for (;;) {
			/* Write samples to EEPROM */
			while (ao_fireone_data_pos != ao_data_head) {
				ao_fireone_data.tick = (uint16_t) ao_data_ring[ao_fireone_data_pos].tick;
				ao_fireone_data.type = AO_LOG_SENSOR;
				ao_fireone_data.u.sensor.pressure = (uint16_t) ao_data_ring[ao_fireone_data_pos].adc.pressure;
				ao_fireone_data.u.sensor.thrust = (uint16_t) ao_data_ring[ao_fireone_data_pos].adc.thrust;
	//			for (i = 0; i < 4; i++) {
	//				ao_fireone_data.u.sensor.thermistor[i] = ao_data_ring[ao_fireone_data_pos].sensor.thermistor[i];
	//			}
				ao_log_firetwo();
				ao_fireone_data_pos = ao_data_ring_next(ao_fireone_data_pos);
			}

			ao_log_flush();

			if (!ao_log_running) break;

			/* Wait for a while */
			ao_delay(AO_MS_TO_TICKS(100));
		}
	} while (ao_log_running);
}

