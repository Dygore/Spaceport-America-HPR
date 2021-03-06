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

#include <ao-ms5607.h>

void
ao_ms5607_convert(struct ao_ms5607_sample *sample, struct ao_ms5607_value *value,
		  struct ao_ms5607_prom *prom, bool is_ms5611)
{
	int32_t	dT;
	int32_t TEMP;
	int64_t OFF;
	int64_t SENS;

	dT = sample->temp - ((int32_t) prom->tref << 8);

	TEMP = 2000 + (((int64_t) dT * prom->tempsens) >> 23);

	if (is_ms5611) {
		OFF = ((int64_t) prom->off << 16) + (((int64_t) prom->tco * dT) >> 7);
		SENS = ((int64_t) prom->sens << 15) + (((int64_t) prom->tcs * dT) >> 8);
	} else {
		OFF = ((int64_t) prom->off << 17) + (((int64_t) prom->tco * dT) >> 6);
		SENS = ((int64_t) prom->sens << 16) + (((int64_t) prom->tcs * dT) >> 7);
	}

	if (TEMP < 2000) {
		int32_t	T2 = ((int64_t) dT * (int64_t) dT) >> 31;
		int32_t TEMPM = TEMP - 2000;
		int64_t OFF2 = (61 * (int64_t) TEMPM * (int64_t) TEMPM) >> 4;
		int64_t SENS2 = 2 * (int64_t) TEMPM * (int64_t) TEMPM;
		if (TEMP < -1500) {
			int32_t TEMPP = TEMP + 1500;
			/* You'd think this would need a 64-bit int, but
			 * that would imply a temperature below -327.67°C...
			 */
			int32_t TEMPP2 = TEMPP * TEMPP;
			OFF2 = OFF2 + (int64_t) 15 * TEMPP2;
			SENS2 = SENS2 + (int64_t) 8 * TEMPP2;
		}
		TEMP -= T2;
		OFF -= OFF2;
		SENS -= SENS2;
	}

	value->pres = ((((int64_t) sample->pres * SENS) >> 21) - OFF) >> 15;
	value->temp = TEMP;
}
