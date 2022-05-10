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

static struct ao_log_telestatic log_data;

const uint8_t ao_log_format = AO_LOG_FORMAT_TELESTATIC;

static uint8_t
ao_log_csum(uint8_t *b) 
{
	uint8_t	sum = 0x5a;
	uint8_t	i;

	for (i = 0; i < sizeof (struct ao_log_telestatic); i++)
		sum += *b++;
	return -sum;
}

static uint8_t
ao_log_telestatic(void)
{
	uint8_t wrote = 0;
	/* set checksum */
	log_data.csum = 0;
	log_data.csum = ao_log_csum((uint8_t *) &log_data);
	ao_mutex_get(&ao_log_mutex); {
		if (ao_log_current_pos >= ao_log_end_pos && ao_log_running)
			ao_log_stop();
		if (ao_log_running) {
			wrote = 1;
			ao_storage_write(ao_log_current_pos,
					 &log_data,
					 sizeof (struct ao_log_telestatic));
			ao_log_current_pos += sizeof (struct ao_log_telestatic);
		}
	} ao_mutex_put(&ao_log_mutex);
	return wrote;
}

#if HAS_ADC
static uint8_t	ao_log_data_pos;

/* a hack to make sure that ao_log_metrums fill the eeprom block in even units */
typedef uint8_t check_log_size[1-(256 % sizeof(struct ao_log_telestatic))] ;
#endif

void
ao_log(void)
{
	ao_storage_setup();

	do {
		ao_log_scan();
	
		while (!ao_log_running)
			ao_sleep(&ao_log_running);
	
		log_data.type = AO_LOG_FLIGHT;
		log_data.tick = ao_time();
		log_data.u.flight.flight = ao_flight_number;
		ao_log_telestatic();

		/* Write the whole contents of the ring to the log
	 	* when starting up.
	 	*/
		ao_log_data_pos = ao_data_ring_next(ao_data_head);
		for (;;) {
			/* Write samples to EEPROM */
			while (ao_log_data_pos != ao_data_head) {
				log_data.tick = ao_data_ring[ao_log_data_pos].tick;
				log_data.type = AO_LOG_SENSOR;
#if HAS_ADS131A0X
				log_data.u.sensor.pressure = ao_data_ring[ao_log_data_pos].ads131a0x.ain[0];
				log_data.u.sensor.pressure2 = ao_data_ring[ao_log_data_pos].ads131a0x.ain[1];
				log_data.u.sensor.thrust = ao_data_ring[ao_log_data_pos].ads131a0x.ain[2];
				log_data.u.sensor.mass = ao_data_ring[ao_log_data_pos].ads131a0x.ain[3];
#endif
				log_data.u.sensor.t_low = ao_data_ring[ao_log_data_pos].max6691.sensor[0].t_low;
				int i;
				for (i = 0; i < 4; i++)
					log_data.u.sensor.t_high[i] = ao_data_ring[ao_log_data_pos].max6691.sensor[i].t_high;
				ao_log_telestatic();
				ao_log_data_pos = ao_data_ring_next(ao_log_data_pos);
			}

			ao_log_flush();

			if (!ao_log_running) break;

			/* Wait for a while */
			ao_delay(AO_MS_TO_TICKS(100));
		}
	} while (ao_log_running);
}

