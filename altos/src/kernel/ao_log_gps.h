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
#include "ao_telemetry.h"

#ifndef _AO_LOG_GPS_H_
#define _AO_LOG_GPS_H_

void
ao_log_gps_flight(void);

void
ao_log_gps_data(AO_TICK_TYPE tick, struct ao_telemetry_location *gps_data);

void
ao_log_gps_tracking(AO_TICK_TYPE tick, struct ao_telemetry_satellite *gps_tracking_data);

#endif /* _AO_LOG_GPS_H_ */
