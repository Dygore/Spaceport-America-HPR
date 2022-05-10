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
#include <ao_data.h>
#endif

#if HAS_GYRO
#include <ao_quaternion.h>
#endif

/*
 * Current sensor values
 */

#ifndef PRES_TYPE
#define PRES_TYPE int32_t
#define ALT_TYPE int32_t
#define ACCEL_TYPE int16_t
#endif

AO_TICK_TYPE	ao_sample_tick;		/* time of last data */
#if HAS_BARO
pres_t		ao_sample_pres;
alt_t		ao_sample_alt;
alt_t		ao_sample_height;
#endif
#if HAS_ACCEL
accel_t		ao_sample_accel;
#endif
#if HAS_IMU
accel_t		ao_sample_accel_along;
accel_t		ao_sample_accel_across;
accel_t		ao_sample_accel_through;
#endif
#if HAS_GYRO
gyro_t		ao_sample_roll;
gyro_t		ao_sample_pitch;
gyro_t		ao_sample_yaw;
angle_t		ao_sample_orient;
angle_t		ao_sample_orients[AO_NUM_ORIENT];
uint8_t		ao_sample_orient_pos;
#endif
#ifdef HAS_MOTOR_PRESSURE
motor_pressure_t	ao_sample_motor_pressure;
#endif

uint8_t		ao_sample_data;

/*
 * Sensor calibration values
 */

#if HAS_BARO
pres_t		ao_ground_pres;		/* startup pressure */
alt_t		ao_ground_height;	/* MSL of ao_ground_pres */
#endif

#if HAS_ACCEL
accel_t		ao_ground_accel;	/* startup acceleration */
accel_t		ao_accel_2g;		/* factory accel calibration */
int32_t		ao_accel_scale;		/* sensor to m/s² conversion */
#endif

#if HAS_IMU
accel_t		ao_ground_accel_along;
accel_t		ao_ground_accel_across;
accel_t		ao_ground_accel_through;
#endif

#if HAS_GYRO
int32_t		ao_ground_pitch;
int32_t		ao_ground_yaw;
int32_t		ao_ground_roll;
#endif

#if HAS_MOTOR_PRESSURE
motor_pressure_t	ao_ground_motor_pressure;
#endif

static uint8_t	ao_preflight;		/* in preflight mode */

static uint16_t	nsamples;
#if HAS_BARO
int32_t ao_sample_pres_sum;
#endif
#if HAS_ACCEL
int32_t ao_sample_accel_sum;
#endif
#if HAS_IMU
int32_t ao_sample_accel_along_sum;
int32_t ao_sample_accel_across_sum;
int32_t	ao_sample_accel_through_sum;
#endif
#if HAS_GYRO
int32_t ao_sample_pitch_sum;
int32_t ao_sample_yaw_sum;
int32_t	ao_sample_roll_sum;
static struct ao_quaternion ao_rotation;
#endif
#if HAS_MOTOR_PRESSURE
int32_t ao_sample_motor_pressure_sum;
#endif

#if HAS_FLIGHT_DEBUG
extern uint8_t ao_orient_test;
#endif

static void
ao_sample_preflight_add(void)
{
#if HAS_ACCEL
	ao_sample_accel_sum += ao_sample_accel;
#endif
#if HAS_BARO
	ao_sample_pres_sum += ao_sample_pres;
#endif
#if HAS_IMU
	ao_sample_accel_along_sum += ao_sample_accel_along;
	ao_sample_accel_across_sum += ao_sample_accel_across;
	ao_sample_accel_through_sum += ao_sample_accel_through;
#endif
#if HAS_GYRO
	ao_sample_pitch_sum += ao_sample_pitch;
	ao_sample_yaw_sum += ao_sample_yaw;
	ao_sample_roll_sum += ao_sample_roll;
#endif
#if HAS_MOTOR_PRESSURE
	ao_sample_motor_pressure_sum += ao_sample_motor_pressure;
#endif
	++nsamples;
}

#if HAS_GYRO
static void
ao_sample_set_all_orients(void)
{
	int i;
	for (i = 0; i < AO_NUM_ORIENT; i++)
		ao_sample_orients[i] = ao_sample_orient;
	ao_sample_orient_pos = 0;
}

static void
ao_sample_set_one_orient(void)
{
	ao_sample_orients[ao_sample_orient_pos] = ao_sample_orient;
	ao_sample_orient_pos = (uint8_t) ((ao_sample_orient_pos + 1) % AO_NUM_ORIENT);
}

static void
ao_sample_compute_orient(void)
{
	/* Compute pitch angle from vertical by taking the pad
	 * orientation vector and rotating it by the current total
	 * rotation value. That will be a unit vector pointing along
	 * the airframe axis. The Z value will be the cosine of the
	 * change in the angle from vertical since boost.
	 *
	 * rot = ao_rotation * vertical * ao_rotation°
	 * rot = ao_rotation * (0,0,0,1) * ao_rotation°
	 *     = ((a.z, a.y, -a.x, a.r) * (a.r, -a.x, -a.y, -a.z)) .z
	 *
	 *     = (-a.z * -a.z) + (a.y * -a.y) - (-a.x * -a.x) + (a.r * a.r)
	 *     = a.z² - a.y² - a.x² + a.r²
	 *
	 * rot = ao_rotation * (0, 0, 0, -1) * ao_rotation°
	 *     = ((-a.z, -a.y, a.x, -a.r) * (a.r, -a.x, -a.y, -a.z)) .z
	 *
	 *     = (a.z * -a.z) + (-a.y * -a.y) - (a.x * -a.x) + (-a.r * a.r)
	 *     = -a.z² + a.y² + a.x² - a.r²
	 */

	float rotz;
	rotz = ao_rotation.z * ao_rotation.z - ao_rotation.y * ao_rotation.y - ao_rotation.x * ao_rotation.x + ao_rotation.r * ao_rotation.r;

	ao_sample_orient = (angle_t) (acosf(rotz) * (float) (180.0/M_PI));
}
#endif /* HAS_GYRO */

static void
ao_sample_preflight_set(void)
{
#if HAS_ACCEL
	ao_ground_accel = (accel_t) (ao_sample_accel_sum >> 9);
	ao_sample_accel_sum = 0;
#endif
#if HAS_BARO
	ao_ground_pres = ao_sample_pres_sum >> 9;
	ao_ground_height = pres_to_altitude(ao_ground_pres);
	ao_sample_pres_sum = 0;
#endif
#if HAS_IMU
	ao_ground_accel_along = (accel_t) (ao_sample_accel_along_sum >> 9);
	ao_ground_accel_across = (accel_t) (ao_sample_accel_across_sum >> 9);
	ao_ground_accel_through = (accel_t) (ao_sample_accel_through_sum >> 9);
	ao_sample_accel_along_sum = 0;
	ao_sample_accel_across_sum = 0;
	ao_sample_accel_through_sum = 0;
#endif
#if HAS_MOTOR_PRESSURE
	ao_ground_motor_pressure = (motor_pressure_t) (ao_sample_motor_pressure_sum >> 9);
	ao_sample_motor_pressure_sum = 0;
#endif
#if HAS_GYRO
	ao_ground_pitch = ao_sample_pitch_sum;
	ao_ground_yaw = ao_sample_yaw_sum;
	ao_ground_roll = ao_sample_roll_sum;
	ao_sample_pitch_sum = 0;
	ao_sample_yaw_sum = 0;
	ao_sample_roll_sum = 0;
	ao_sample_set_all_orients();

	struct ao_quaternion	orient;

	/* Take the pad IMU acceleration values and compute our current direction
	 */

	ao_quaternion_init_vector(&orient,
				  (ao_ground_accel_across - ao_config.accel_zero_across),
				  (ao_ground_accel_through - ao_config.accel_zero_through),
				  (ao_ground_accel_along - ao_config.accel_zero_along));

	ao_quaternion_normalize(&orient,
				&orient);

	/* Here's up */

	struct ao_quaternion	up = { .r = 0, .x = 0, .y = 0, .z = 1 };

	if (ao_config.pad_orientation != AO_PAD_ORIENTATION_ANTENNA_UP)
		up.z = -1;

	/* Compute rotation to get from up to our current orientation, set
	 * that as the current rotation vector
	 */
	ao_quaternion_vectors_to_rotation(&ao_rotation, &up, &orient);
#if HAS_FLIGHT_DEBUG
	if (ao_orient_test) {
		printf("\n\treset across %d through %d along %d\n",
		       (ao_ground_accel_across - ao_config.accel_zero_across),
		       (ao_ground_accel_through - ao_config.accel_zero_through),
		       (ao_ground_accel_along - ao_config.accel_zero_along));
		fflush(stdout);
	}
#endif

	ao_sample_compute_orient();
	ao_sample_set_all_orients();
#endif
	nsamples = 0;
}

#if HAS_GYRO
#define TIME_DIV	200.0f

static void
ao_sample_rotate(void)
{
#ifdef AO_FLIGHT_TEST
	float	dt = (AO_TICK_SIGNED) (ao_sample_tick - ao_sample_prev_tick) / TIME_DIV;
#else
	static const float dt = 1/TIME_DIV;
#endif
	float	x = ao_convert_gyro((float) ((ao_sample_pitch << 9) - ao_ground_pitch) / 512.0f) * dt;
	float	y = ao_convert_gyro((float) ((ao_sample_yaw << 9) - ao_ground_yaw) / 512.0f) * dt;
	float	z = ao_convert_gyro((float) ((ao_sample_roll << 9) - ao_ground_roll) / 512.0f) * dt;
	struct ao_quaternion	rot;

	ao_quaternion_init_half_euler(&rot, x, y, z);
	ao_quaternion_multiply(&ao_rotation, &rot, &ao_rotation);

	/* And normalize to make sure it remains a unit vector */
	ao_quaternion_normalize(&ao_rotation, &ao_rotation);

#if HAS_FLIGHT_DEBUG
	if (ao_orient_test) {
		printf ("rot %d %d %d orient %d     \r",
			(int) (x * 1000),
			(int) (y * 1000),
			(int) (z * 1000),
			ao_sample_orient);
		fflush(stdout);
	}
#endif
	ao_sample_compute_orient();
	ao_sample_set_one_orient();
}
#endif

static void
ao_sample_preflight(void)
{
	/* startup state:
	 *
	 * Collect 512 samples of acceleration and pressure
	 * data and average them to find the resting values
	 */
	if (nsamples < 512) {
		ao_sample_preflight_add();
	} else {
#if HAS_ACCEL
		ao_accel_2g = ao_config.accel_minus_g - ao_config.accel_plus_g;
		ao_accel_scale = to_fix_32(GRAVITY * 2 * 16) / ao_accel_2g;
#endif
		ao_sample_preflight_set();
		ao_preflight = false;
	}
}

/*
 * While in pad mode, constantly update the ground state by
 * re-averaging the data.  This tracks changes in orientation, which
 * might be caused by adjustments to the rocket on the pad and
 * pressure, which might be caused by changes in the weather.
 */

static void
ao_sample_preflight_update(void)
{
	if (nsamples < 512)
		ao_sample_preflight_add();
	else if (nsamples < 1024)
		++nsamples;
	else
		ao_sample_preflight_set();
#if !HAS_BARO
	if ((nsamples & 0x3f) == 0)
		ao_kalman_reset_accumulate();
#endif
}

#if 0
#if HAS_GYRO
static int32_t	p_filt;
static int32_t	y_filt;

static gyro_t inline ao_gyro(void) {
	gyro_t	p = ao_sample_pitch - ao_ground_pitch;
	gyro_t	y = ao_sample_yaw - ao_ground_yaw;

	p_filt = p_filt - (p_filt >> 6) + p;
	y_filt = y_filt - (y_filt >> 6) + y;

	p = p_filt >> 6;
	y = y_filt >> 6;
	return ao_sqrt(p*p + y*y);
}
#endif
#endif

uint8_t
ao_sample(void)
{
	ao_wakeup(&ao_sample_data);
	ao_sleep((void *) &ao_data_head);
	while (ao_sample_data != ao_data_head) {
		struct ao_data *ao_data;

		/* Capture a sample */
		ao_data = (struct ao_data *) &ao_data_ring[ao_sample_data];
		ao_sample_tick = ao_data->tick;

#if HAS_BARO
		ao_data_pres_cook(ao_data);
		ao_sample_pres = ao_data_pres(ao_data);
		ao_sample_alt = pres_to_altitude(ao_sample_pres);
		ao_sample_height = ao_sample_alt - ao_ground_height;
#endif

#if HAS_ACCEL
		ao_sample_accel = ao_data_accel(ao_data);
#endif
#if HAS_IMU
		ao_sample_accel_along = ao_data_along(ao_data);
		ao_sample_accel_across = ao_data_across(ao_data);
		ao_sample_accel_through = ao_data_through(ao_data);
#endif
#if HAS_GYRO
		ao_sample_pitch = ao_data_pitch(ao_data);
		ao_sample_yaw = ao_data_yaw(ao_data);
		ao_sample_roll = ao_data_roll(ao_data);
#endif
#if HAS_MOTOR_PRESSURE
		ao_sample_motor_pressure = ao_data_motor_pressure(ao_data);
#endif

		if (ao_preflight)
			ao_sample_preflight();
		else {
			if (ao_flight_state < ao_flight_boost)
				ao_sample_preflight_update();
			ao_kalman();
#if HAS_GYRO
			ao_sample_rotate();
#endif
		}
#ifdef AO_FLIGHT_TEST
		ao_sample_prev_tick = ao_sample_tick;
#endif
		ao_sample_data = ao_data_ring_next(ao_sample_data);
	}
	return !ao_preflight;
}

void
ao_sample_init(void)
{
	ao_config_get();
	nsamples = 0;
#if HAS_BARO
	ao_sample_pres_sum = 0;
	ao_sample_pres = 0;
#endif
#if HAS_ACCEL
	ao_sample_accel_sum = 0;
	ao_sample_accel = 0;
#endif
#if HAS_IMU
	ao_sample_accel_along_sum = 0;
	ao_sample_accel_across_sum = 0;
	ao_sample_accel_through_sum = 0;
	ao_sample_accel_along = 0;
	ao_sample_accel_across = 0;
	ao_sample_accel_through = 0;
#endif
#if HAS_GYRO
	ao_sample_pitch_sum = 0;
	ao_sample_yaw_sum = 0;
	ao_sample_roll_sum = 0;
	ao_sample_pitch = 0;
	ao_sample_yaw = 0;
	ao_sample_roll = 0;
	ao_sample_orient = 0;
	ao_sample_set_all_orients();
#endif
	ao_sample_data = ao_data_head;
	ao_preflight = true;
}
