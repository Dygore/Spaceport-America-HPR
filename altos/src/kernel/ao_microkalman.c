/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#ifndef AO_FLIGHT_TEST
#include <ao.h>
#endif
#include <ao_micropeak.h>

#define FIX_BITS	16

#define to_fix_v(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix_k(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix8(x)	((x) >> 8)
#define from_fix(x)	((x) >> 16)
#define fix8_to_fix_v(x)	((x) << 8)
#define fix16_to_fix8(x)	((x) >> 8)

#include <ao_kalman.h>

#ifndef AO_MK_STEP_96MS
#define AO_MK_STEP_96MS	1

#endif

#if AO_MK_STEP_100MS
#define AO_MK_TIME_STEP	0.1

#define AO_K0_10	AO_MK2_BARO_K0_10
#define AO_K1_10	AO_MK2_BARO_K1_10
#define AO_K2_10	AO_MK2_BARO_K2_10
#endif

#if AO_MK_STEP_96MS
#define AO_MK_TIME_STEP	0.096

#define AO_K0_10	AO_MK_BARO_K0_10
#define AO_K1_10	AO_MK_BARO_K1_10
#define AO_K2_10	AO_MK_BARO_K2_10
#endif

/* Basic time step (96ms) */
#define AO_MK_STEP	to_fix_v(AO_MK_TIME_STEP)

/* step ** 2 / 2 */
#define AO_MK_STEP_2_2	to_fix_v(AO_MK_TIME_STEP * AO_MK_TIME_STEP / 2.0)

uint32_t	ao_k_pa;		/* 24.8 fixed point */
int32_t		ao_k_pa_speed;		/* 16.16 fixed point */
int32_t		ao_k_pa_accel;		/* 16.16 fixed point */

uint32_t	ao_pa;			/* integer portion */
int16_t		ao_pa_speed;		/* integer portion */
int16_t		ao_pa_accel;		/* integer portion */

void
ao_microkalman_init(void)
{
	ao_pa = (uint32_t) pa;
	ao_k_pa = (uint32_t) (pa << 8);
}

void
ao_microkalman_predict(void)
{
	ao_k_pa       += (uint32_t) (int32_t) fix16_to_fix8((int32_t) ao_pa_speed * AO_MK_STEP + (int32_t) ao_pa_accel * AO_MK_STEP_2_2);
	ao_k_pa_speed += (int32_t) ao_pa_accel * AO_MK_STEP;
}

void
ao_microkalman_correct(void)
{
	int16_t	e;	/* Height error in Pa */

	e = (int16_t) (pa - from_fix8(ao_k_pa));

	ao_k_pa       += (uint32_t)(int32_t)fix16_to_fix8(e * AO_K0_10);
	ao_k_pa_speed += (int32_t) e * AO_K1_10;
	ao_k_pa_accel += (int32_t) e * AO_K2_10;
	ao_pa = from_fix8(ao_k_pa);
	ao_pa_speed = (int16_t) from_fix(ao_k_pa_speed);
	ao_pa_accel = (int16_t) from_fix(ao_k_pa_accel);
}
