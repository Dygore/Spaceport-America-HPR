/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
#include "ao.h"
#include "ao_flight.h"
#endif

#include "ao_sample.h"
#include "ao_kalman.h"

static ao_k_t		ao_k_height;
static ao_k_t		ao_k_speed;
static ao_k_t		ao_k_accel;

#define AO_K_STEP_100		to_fix_v(0.01)
#define AO_K_STEP_2_2_100	to_fix_v(0.00005)

#define AO_K_STEP_10		to_fix_v(0.1)
#define AO_K_STEP_2_2_10	to_fix_v(0.005)

#define AO_K_STEP_1		to_fix_v(1)
#define AO_K_STEP_2_2_1		to_fix_v(0.5)

ao_v_t			ao_height;
ao_v_t			ao_speed;
ao_v_t			ao_accel;
ao_v_t			ao_max_height;
static ao_k_t		ao_avg_height_scaled;
ao_v_t			ao_avg_height;

ao_v_t			ao_error_h;
#if !HAS_ACCEL || AO_FLIGHT_TEST
#define AO_ERROR_H_SQ_AVG	1
#endif

#if AO_ERROR_H_SQ_AVG
ao_v_t			ao_error_h_sq_avg;
#endif

#if HAS_ACCEL
ao_v_t			ao_error_a;
#endif

static void
ao_kalman_predict(void)
{
#ifdef AO_FLIGHT_TEST
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 50) {
		ao_k_height += ((ao_k_t) ao_speed * AO_K_STEP_1 +
				(ao_k_t) ao_accel * AO_K_STEP_2_2_1) >> 4;
		ao_k_speed += (ao_k_t) ao_accel * AO_K_STEP_1;

		return;
	}
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 5) {
		ao_k_height += ((ao_k_t) ao_speed * AO_K_STEP_10 +
				(ao_k_t) ao_accel * AO_K_STEP_2_2_10) >> 4;
		ao_k_speed += (ao_k_t) ao_accel * AO_K_STEP_10;

		return;
	}
	if (ao_flight_debug) {
		printf ("predict speed %g + (%g * %g) = %g\n",
			ao_k_speed / (65536.0 * 16.0), ao_accel / 16.0, AO_K_STEP_100 / 65536.0,
			(ao_k_speed + (ao_k_t) ao_accel * AO_K_STEP_100) / (65536.0 * 16.0));
	}
#endif
	ao_k_height += ((ao_k_t) ao_speed * AO_K_STEP_100 +
			(ao_k_t) ao_accel * AO_K_STEP_2_2_100) >> 4;
	ao_k_speed += (ao_k_t) ao_accel * AO_K_STEP_100;
}

#if HAS_BARO
static void
ao_kalman_err_height(void)
{
#if AO_ERROR_H_SQ_AVG
	ao_v_t	e;
#endif
	ao_v_t height_distrust;
#if HAS_ACCEL
	ao_v_t	speed_distrust;
#endif

	ao_error_h = ao_sample_height - (ao_v_t) (ao_k_height >> 16);

#if AO_ERROR_H_SQ_AVG
	e = ao_error_h;
	if (e < 0)
		e = -e;
	if (e > 127)
		e = 127;
	ao_error_h_sq_avg -= ao_error_h_sq_avg >> 4;
	ao_error_h_sq_avg += (e * e) >> 4;
#endif

	if (ao_flight_state >= ao_flight_drogue)
		return;
	height_distrust = ao_sample_alt - AO_MAX_BARO_HEIGHT;
#if HAS_ACCEL
	/* speed is stored * 16, but we need to ramp between 248 and 328, so
	 * we want to multiply by 2. The result is a shift by 3.
	 */
	speed_distrust = (ao_speed - AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) >> (4 - 1);
	if (speed_distrust > AO_MAX_SPEED_DISTRUST)
		speed_distrust = AO_MAX_SPEED_DISTRUST;
	if (speed_distrust > height_distrust)
		height_distrust = speed_distrust;
#endif
	if (height_distrust > 0) {
#ifdef AO_FLIGHT_TEST
		int	old_ao_error_h = ao_error_h;
#endif
		if (height_distrust > 0x100)
			height_distrust = 0x100;
		ao_error_h = (ao_v_t) (((ao_k_t) ao_error_h * (0x100 - height_distrust)) >> 8);
#ifdef AO_FLIGHT_TEST
		if (ao_flight_debug) {
			printf("over height %g over speed %g distrust: %g height: error %d -> %d\n",
			       (double) (ao_sample_alt - AO_MAX_BARO_HEIGHT),
			       (ao_speed - AO_MS_TO_SPEED(AO_MAX_BARO_SPEED)) / 16.0,
			       height_distrust / 256.0,
			       old_ao_error_h, ao_error_h);
		}
#endif
	}
}
#endif

#if HAS_BARO
static void
ao_kalman_correct_baro(void)
{
	ao_kalman_err_height();
#ifdef AO_FLIGHT_TEST
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 50) {
		ao_k_height += (ao_k_t) AO_BARO_K0_1 * ao_error_h;
		ao_k_speed  += (ao_k_t) AO_BARO_K1_1 * ao_error_h;
		ao_k_accel  += (ao_k_t) AO_BARO_K2_1 * ao_error_h;
		return;
	}
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 5) {
		ao_k_height += (ao_k_t) AO_BARO_K0_10 * ao_error_h;
		ao_k_speed  += (ao_k_t) AO_BARO_K1_10 * ao_error_h;
		ao_k_accel  += (ao_k_t) AO_BARO_K2_10 * ao_error_h;
		return;
	}
#endif
	ao_k_height += (ao_k_t) AO_BARO_K0_100 * ao_error_h;
	ao_k_speed  += (ao_k_t) AO_BARO_K1_100 * ao_error_h;
	ao_k_accel  += (ao_k_t) AO_BARO_K2_100 * ao_error_h;
}
#endif

#if HAS_ACCEL

static void
ao_kalman_err_accel(void)
{
	ao_k_t	accel;

	accel = (ao_config.accel_plus_g - ao_sample_accel) * ao_accel_scale;

	/* Can't use ao_accel here as it is the pre-prediction value still */
	ao_error_a = (ao_v_t) ((accel - ao_k_accel) >> 16);
}

#if !defined(FORCE_ACCEL) && HAS_BARO
static void
ao_kalman_correct_both(void)
{
	ao_kalman_err_height();
	ao_kalman_err_accel();

#ifdef AO_FLIGHT_TEST
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 50) {
		if (ao_flight_debug) {
			printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
				ao_k_speed / (65536.0 * 16.0),
				(double) ao_error_h, AO_BOTH_K10_1 / 65536.0,
				(double) ao_error_a, AO_BOTH_K11_1 / 65536.0,
				(ao_k_speed +
				 (ao_k_t) AO_BOTH_K10_1 * ao_error_h +
				 (ao_k_t) AO_BOTH_K11_1 * ao_error_a) / (65536.0 * 16.0));
		}
		ao_k_height +=
			(ao_k_t) AO_BOTH_K00_1 * ao_error_h +
			(ao_k_t) AO_BOTH_K01_1 * ao_error_a;
		ao_k_speed +=
			(ao_k_t) AO_BOTH_K10_1 * ao_error_h +
			(ao_k_t) AO_BOTH_K11_1 * ao_error_a;
		ao_k_accel +=
			(ao_k_t) AO_BOTH_K20_1 * ao_error_h +
			(ao_k_t) AO_BOTH_K21_1 * ao_error_a;
		return;
	}
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 5) {
		if (ao_flight_debug) {
			printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
				ao_k_speed / (65536.0 * 16.0),
				(double) ao_error_h, AO_BOTH_K10_10 / 65536.0,
				(double) ao_error_a, AO_BOTH_K11_10 / 65536.0,
				(ao_k_speed +
				 (ao_k_t) AO_BOTH_K10_10 * ao_error_h +
				 (ao_k_t) AO_BOTH_K11_10 * ao_error_a) / (65536.0 * 16.0));
		}
		ao_k_height +=
			(ao_k_t) AO_BOTH_K00_10 * ao_error_h +
			(ao_k_t) AO_BOTH_K01_10 * ao_error_a;
		ao_k_speed +=
			(ao_k_t) AO_BOTH_K10_10 * ao_error_h +
			(ao_k_t) AO_BOTH_K11_10 * ao_error_a;
		ao_k_accel +=
			(ao_k_t) AO_BOTH_K20_10 * ao_error_h +
			(ao_k_t) AO_BOTH_K21_10 * ao_error_a;
		return;
	}
	if (ao_flight_debug) {
		printf ("correct speed %g + (%g * %g) + (%g * %g) = %g\n",
			ao_k_speed / (65536.0 * 16.0),
			(double) ao_error_h, AO_BOTH_K10_100 / 65536.0,
			(double) ao_error_a, AO_BOTH_K11_100 / 65536.0,
			(ao_k_speed +
			 (ao_k_t) AO_BOTH_K10_100 * ao_error_h +
			 (ao_k_t) AO_BOTH_K11_100 * ao_error_a) / (65536.0 * 16.0));
	}
#endif
	ao_k_height +=
		(ao_k_t) AO_BOTH_K00_100 * ao_error_h +
		(ao_k_t) AO_BOTH_K01_100 * ao_error_a;
	ao_k_speed +=
		(ao_k_t) AO_BOTH_K10_100 * ao_error_h +
		(ao_k_t) AO_BOTH_K11_100 * ao_error_a;
	ao_k_accel +=
		(ao_k_t) AO_BOTH_K20_100 * ao_error_h +
		(ao_k_t) AO_BOTH_K21_100 * ao_error_a;
}

#else

static void
ao_kalman_correct_accel(void)
{
	ao_kalman_err_accel();

#ifdef AO_FLIGHT_TEST
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 5) {
		ao_k_height +=(ao_k_t) AO_ACCEL_K0_10 * ao_error_a;
		ao_k_speed  += (ao_k_t) AO_ACCEL_K1_10 * ao_error_a;
		ao_k_accel  += (ao_k_t) AO_ACCEL_K2_10 * ao_error_a;
		return;
	}
#endif
	ao_k_height += (ao_k_t) AO_ACCEL_K0_100 * ao_error_a;
	ao_k_speed  += (ao_k_t) AO_ACCEL_K1_100 * ao_error_a;
	ao_k_accel  += (ao_k_t) AO_ACCEL_K2_100 * ao_error_a;
}

#endif /* else FORCE_ACCEL */
#endif /* HAS_ACCEL */

#if !HAS_BARO
static ao_k_t	ao_k_height_prev;
static ao_k_t	ao_k_speed_prev;

/*
 * While in pad mode without a barometric sensor, remove accumulated
 * speed and height values to reduce the effect of systematic sensor
 * error
 */
void
ao_kalman_reset_accumulate(void)
{
	ao_k_height -= ao_k_height_prev;
	ao_k_speed -= ao_k_speed_prev;
	ao_k_height_prev = ao_k_height;
	ao_k_speed_prev = ao_k_speed;
}
#endif

void
ao_kalman(void)
{
	ao_kalman_predict();
#if HAS_BARO
#if HAS_ACCEL
	if (ao_flight_state <= ao_flight_coast) {
#ifdef FORCE_ACCEL
		ao_kalman_correct_accel();
#else
		ao_kalman_correct_both();
#endif
	} else
#endif
		ao_kalman_correct_baro();
#else
#if HAS_ACCEL
	ao_kalman_correct_accel();
#endif
#endif
	ao_height = (ao_v_t) from_fix(ao_k_height);
	ao_speed = (ao_v_t) from_fix(ao_k_speed);
	ao_accel = (ao_v_t) from_fix(ao_k_accel);
	if (ao_height > ao_max_height)
		ao_max_height = ao_height;
#if HAS_BARO
	ao_avg_height_scaled = ao_avg_height_scaled - ao_avg_height + ao_sample_height;
#else
	ao_avg_height_scaled = ao_avg_height_scaled - ao_avg_height + ao_height;
#endif
#ifdef AO_FLIGHT_TEST
	if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 50)
		ao_avg_height = (ao_avg_height_scaled + 1) >> 1;
	else if ((AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) > 5)
		ao_avg_height = (ao_avg_height_scaled + 7) >> 4;
	else 
#endif
		ao_avg_height = (ao_v_t) ((ao_avg_height_scaled + 63) >> 7);
}
