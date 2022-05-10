/*
 * Copyright © 2014 Keith Packard <keithp@keithp.com>
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
#include <ao_log_gps.h>
#include <ao_data.h>
#include <ao_flight.h>
#include <ao_distance.h>
#include <ao_tracker.h>

void
ao_log_gps_flight(void)
{
	ao_log_data.type = AO_LOG_FLIGHT;
	ao_log_data.tick = (uint16_t) ao_time();
	ao_log_data.u.flight.flight = ao_flight_number;
	ao_log_write(&ao_log_data);
}

void
ao_log_gps_data(AO_TICK_TYPE tick, struct ao_telemetry_location *gps_data)
{
	ao_log_data.tick = (uint16_t) tick;
	ao_log_data.type = AO_LOG_GPS_TIME;
	ao_log_data.u.gps.latitude = gps_data->latitude;
	ao_log_data.u.gps.longitude = gps_data->longitude;
	ao_log_data.u.gps.altitude_low = gps_data->altitude_low;
	ao_log_data.u.gps.altitude_high = gps_data->altitude_high;

	ao_log_data.u.gps.hour = gps_data->hour;
	ao_log_data.u.gps.minute = gps_data->minute;
	ao_log_data.u.gps.second = gps_data->second;
	ao_log_data.u.gps.flags = gps_data->flags;
	ao_log_data.u.gps.year = gps_data->year;
	ao_log_data.u.gps.month = gps_data->month;
	ao_log_data.u.gps.day = gps_data->day;
	ao_log_data.u.gps.course = gps_data->course;
	ao_log_data.u.gps.ground_speed = gps_data->ground_speed;
	ao_log_data.u.gps.climb_rate = gps_data->climb_rate;
	ao_log_data.u.gps.pdop = gps_data->pdop;
	ao_log_data.u.gps.hdop = gps_data->hdop;
	ao_log_data.u.gps.vdop = gps_data->vdop;
	ao_log_data.u.gps.mode = gps_data->mode;
	ao_log_write(&ao_log_data);
}

void
ao_log_gps_tracking(AO_TICK_TYPE tick, struct ao_telemetry_satellite *gps_tracking_data)
{
	uint8_t c, n, i;

	ao_log_data.tick = (uint16_t) tick;
	ao_log_data.type = AO_LOG_GPS_SAT;
	i = 0;
	n = gps_tracking_data->channels;
	for (c = 0; c < n; c++)
		if ((ao_log_data.u.gps_sat.sats[i].svid = gps_tracking_data->sats[c].svid))
		{
			ao_log_data.u.gps_sat.sats[i].c_n = gps_tracking_data->sats[c].c_n_1;
			i++;
			if (i >= 12)
				break;
		}
	ao_log_data.u.gps_sat.channels = i;
	ao_log_write(&ao_log_data);
}

static uint8_t
ao_log_check_empty(void)
{
	uint8_t *b = (void *) &ao_log_data;
	unsigned i;

	for (i = 0; i < sizeof (ao_log_type); i++)
		if (*b++ != AO_STORAGE_ERASED_BYTE)
			return 0;
	return 1;
}

int8_t
ao_log_check(uint32_t pos)
{
	if (!ao_storage_read(pos,
			     &ao_log_data,
			     sizeof (struct ao_log_gps)))
		return AO_LOG_INVALID;

	if (ao_log_check_empty())
		return AO_LOG_EMPTY;

	if (!ao_log_check_data())
		return AO_LOG_INVALID;

	return AO_LOG_VALID;
}
