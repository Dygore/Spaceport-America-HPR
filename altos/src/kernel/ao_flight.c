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

#ifndef AO_FLIGHT_TEST
#include "ao.h"
#include <ao_log.h>
#endif

#include <ao_flight.h>

#if HAS_MPU6000 || HAS_MPU9250
#include <ao_quaternion.h>
#endif

#ifndef HAS_ACCEL
#error Please define HAS_ACCEL
#endif

#ifndef HAS_GPS
#error Please define HAS_GPS
#endif

#ifndef HAS_USB
#error Please define HAS_USB
#endif

#if HAS_FAKE_FLIGHT
#include <ao_fake_flight.h>
#endif

#ifndef HAS_TELEMETRY
#define HAS_TELEMETRY	HAS_RADIO
#endif

/* Main flight thread. */

enum ao_flight_state	ao_flight_state;	/* current flight state */
AO_TICK_TYPE		ao_boost_tick;		/* time of most recent boost detect */
AO_TICK_TYPE		ao_launch_tick;		/* time of first boost detect */
uint16_t		ao_motor_number;	/* number of motors burned so far */

#if HAS_SENSOR_ERRORS
/* Any sensor can set this to mark the flight computer as 'broken' */
uint8_t			ao_sensor_errors;
#endif

/*
 * track min/max data over a long interval to detect
 * resting
 */
static AO_TICK_TYPE	ao_interval_end;
#ifdef HAS_BARO
static ao_v_t		ao_interval_min_height;
static ao_v_t		ao_interval_max_height;
#else
static accel_t		ao_interval_min_accel_along, ao_interval_max_accel_along;
static accel_t		ao_interval_min_accel_across, ao_interval_max_accel_across;
static accel_t		ao_interval_min_accel_through, ao_interval_max_accel_through;
#endif
#if HAS_ACCEL
static ao_v_t		ao_coast_avg_accel;
#endif

#define init_bounds(_cur, _min, _max) do {				\
		_min = _max = _cur;					\
	} while (0)

#define check_bounds(_cur, _min, _max) do {	\
		if (_cur < _min)		\
			_min = _cur;		\
		if (_cur > _max)		\
			_max = _cur;		\
	} while(0)

uint8_t			ao_flight_force_idle;

/* We also have a clock, which can be used to sanity check things in
 * case of other failures
 */

#define BOOST_TICKS_MAX	AO_SEC_TO_TICKS(15)

/* Landing is detected by getting constant readings from both pressure and accelerometer
 * for a fairly long time (AO_INTERVAL_TICKS)
 */
#define AO_INTERVAL_TICKS	AO_SEC_TO_TICKS(10)

#define abs(a)	((a) < 0 ? -(a) : (a))

#if !HAS_BARO
// #define DEBUG_ACCEL_ONLY	1
#endif

void
ao_flight(void)
{
	ao_sample_init();
	ao_flight_state = ao_flight_startup;
	for (;;) {

		/*
		 * Process ADC samples, just looping
		 * until the sensors are calibrated.
		 */
		if (!ao_sample())
			continue;

		switch (ao_flight_state) {
		case ao_flight_startup:

			/* Check to see what mode we should go to.
			 *  - Invalid mode if accel cal appears to be out
			 *  - pad mode if we're upright,
			 *  - idle mode otherwise
			 */
#if HAS_ACCEL
			if (ao_config.accel_plus_g == 0 ||
			    ao_config.accel_minus_g == 0 ||
			    ao_ground_accel < (accel_t) ao_config.accel_plus_g - ACCEL_NOSE_UP ||
			    ao_ground_accel > (accel_t) ao_config.accel_minus_g + ACCEL_NOSE_UP
#if HAS_BARO
			    || ao_ground_height < -1000 ||
			    ao_ground_height > 7000
#endif
				)
			{
				/* Detected an accel value outside -1.5g to 1.5g
				 * (or uncalibrated values), so we go into invalid mode
				 */
				ao_flight_state = ao_flight_invalid;

#if HAS_RADIO && PACKET_HAS_SLAVE
				/* Turn on packet system in invalid mode on TeleMetrum */
				ao_packet_slave_start();
#endif
			} else
#endif
				if (!ao_flight_force_idle
#if HAS_ACCEL
				    && ao_ground_accel < ao_config.accel_plus_g + ACCEL_NOSE_UP
#endif
					)
 			{
				/* Set pad mode - we can fly! */
				ao_flight_state = ao_flight_pad;
#if HAS_USB && !HAS_FLIGHT_DEBUG && !HAS_SAMPLE_PROFILE && !DEBUG
				/* Disable the USB controller in flight mode
				 * to save power
				 */
#if HAS_FAKE_FLIGHT
				if (!ao_fake_flight_active)
#endif
					ao_usb_disable();
#endif

#if !HAS_ACCEL && PACKET_HAS_SLAVE
				/* Disable packet mode in pad state on TeleMini */
				ao_packet_slave_stop();
#endif

#if HAS_TELEMETRY
				/* Turn on telemetry system */
				ao_rdf_set(1);
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_PAD);
#endif
#if AO_LED_RED
				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
#endif
			} else {
				/* Set idle mode */
				ao_flight_state = ao_flight_idle;
#if HAS_SENSOR_ERRORS
				if (ao_sensor_errors)
					ao_flight_state = ao_flight_invalid;
#endif
 
#if HAS_ACCEL && HAS_RADIO && PACKET_HAS_SLAVE
				/* Turn on packet system in idle mode on TeleMetrum */
				ao_packet_slave_start();
#endif

#if AO_LED_RED
				/* signal successful initialization by turning off the LED */
				ao_led_off(AO_LED_RED);
#endif
			}
			/* wakeup threads due to state change */
			ao_wakeup(&ao_flight_state);

			break;

#if DEBUG_ACCEL_ONLY
		case ao_flight_invalid:
		case ao_flight_idle:
			printf("+g %d ga %d sa %d accel %ld speed %ld\n",
			       ao_config.accel_plus_g, ao_ground_accel, ao_sample_accel, ao_accel, ao_speed);
			break;
#endif

		case ao_flight_pad:
			/* pad to boost:
			 *
			 * barometer: > 20m vertical motion
			 *             OR
			 * accelerometer: > 2g AND velocity > 5m/s
			 *
			 * The accelerometer should always detect motion before
			 * the barometer, but we use both to make sure this
			 * transition is detected. If the device
			 * doesn't have an accelerometer, then ignore the
			 * speed and acceleration as they are quite noisy
			 * on the pad.
			 */
			if (ao_height > AO_M_TO_HEIGHT(20)
#if HAS_ACCEL
			    || (ao_accel > AO_MSS_TO_ACCEL(20)
				&& ao_speed > AO_MS_TO_SPEED(5))
#endif
				)
			{
				ao_flight_state = ao_flight_boost;
				ao_wakeup(&ao_flight_state);

				ao_launch_tick = ao_boost_tick = ao_sample_tick;

				/* start logging data */
#if HAS_LOG
				ao_log_start();
#endif

#if HAS_TELEMETRY
				/* Increase telemetry rate */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_FLIGHT);

				/* disable RDF beacon */
				ao_rdf_set(0);
#endif

#if HAS_GPS
				/* Record current GPS position by waking up GPS log tasks */
				ao_gps_new = AO_GPS_NEW_DATA | AO_GPS_NEW_TRACKING;
				ao_wakeup(&ao_gps_new);
#endif
			}
			break;
		case ao_flight_boost:

			/* boost to fast:
			 *
			 * accelerometer: start to fall at > 1/4 G
			 *              OR
			 * time: boost for more than 15 seconds
			 *
			 * Detects motor burn out by the switch from acceleration to
			 * deceleration, or by waiting until the maximum burn duration
			 * (15 seconds) has past.
			 */
			if ((ao_accel < AO_MSS_TO_ACCEL(-2.5)) ||
			    (AO_TICK_SIGNED) (ao_sample_tick - ao_boost_tick) > (AO_TICK_SIGNED) BOOST_TICKS_MAX)
			{
#if HAS_ACCEL
#if HAS_BARO
				ao_flight_state = ao_flight_fast;
#else
				ao_flight_state = ao_flight_coast;

				/* Initialize landing detection interval values */
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;

				init_bounds(ao_sample_accel_along, ao_interval_min_accel_along, ao_interval_max_accel_along);
				init_bounds(ao_sample_accel_across, ao_interval_min_accel_across, ao_interval_max_accel_across);
				init_bounds(ao_sample_accel_through, ao_interval_min_accel_through, ao_interval_max_accel_through);
#endif
				ao_coast_avg_accel = ao_accel;
#else
				ao_flight_state = ao_flight_coast;
#endif
				ao_wakeup(&ao_flight_state);
				++ao_motor_number;
			}
			break;
#if HAS_ACCEL && HAS_BARO
		case ao_flight_fast:
			/*
			 * This is essentially the same as coast,
			 * but the barometer is being ignored as
			 * it may be unreliable.
			 */
			if (ao_speed < AO_MS_TO_SPEED(AO_MAX_BARO_SPEED))
			{
				ao_flight_state = ao_flight_coast;
				ao_wakeup(&ao_flight_state);
			} else
				goto check_re_boost;
			break;
#endif
		case ao_flight_coast:

#if HAS_BARO
			/*
			 * By customer request - allow the user
			 * to lock out apogee detection for a specified
			 * number of seconds.
			 */
			if (ao_config.apogee_lockout) {
				if ((AO_TICK_SIGNED) (ao_sample_tick - ao_launch_tick) <
				    (AO_TICK_SIGNED) AO_SEC_TO_TICKS(ao_config.apogee_lockout))
					break;
			}

			/* apogee detect: coast to drogue deploy:
			 *
			 * speed: < 0
			 *
			 * Also make sure the model altitude is tracking
			 * the measured altitude reasonably closely; otherwise
			 * we're probably transsonic.
			 */
#define AO_ERROR_BOUND	100

			if (ao_speed < 0
#if !HAS_ACCEL
			    && (ao_sample_alt >= AO_MAX_BARO_HEIGHT || ao_error_h_sq_avg < AO_ERROR_BOUND)
#endif
				)
			{
				/* enter drogue state */
				ao_flight_state = ao_flight_drogue;
				ao_wakeup(&ao_flight_state);
#if HAS_TELEMETRY
				/* slow down the telemetry system */
				ao_telemetry_set_interval(AO_TELEMETRY_INTERVAL_RECOVER);

				/* Turn the RDF beacon back on */
				ao_rdf_set(1);
#endif
			}
			else
#else /* not HAS_BARO */
			/* coast to land:
			 *
			 * accel: values stable
			 */
			check_bounds(ao_sample_accel_along, ao_interval_min_accel_along, ao_interval_max_accel_along);
			check_bounds(ao_sample_accel_across, ao_interval_min_accel_across, ao_interval_max_accel_across);
			check_bounds(ao_sample_accel_through, ao_interval_min_accel_through, ao_interval_max_accel_through);

#define MAX_QUIET_ACCEL	2

			if ((AO_TICK_SIGNED) (ao_sample_tick - ao_interval_end) >= 0) {
				if (ao_interval_max_accel_along - ao_interval_min_accel_along <= ao_data_accel_to_sample(MAX_QUIET_ACCEL) &&
				    ao_interval_max_accel_across - ao_interval_min_accel_across <= ao_data_accel_to_sample(MAX_QUIET_ACCEL) &&
				    ao_interval_max_accel_through - ao_interval_min_accel_through <= ao_data_accel_to_sample(MAX_QUIET_ACCEL))
				{
					ao_flight_state = ao_flight_landed;
					ao_wakeup(&ao_flight_state);
#if HAS_ADC
					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);
#endif
				}

				/* Reset interval values */
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;

				init_bounds(ao_sample_accel_along, ao_interval_min_accel_along, ao_interval_max_accel_along);
				init_bounds(ao_sample_accel_across, ao_interval_min_accel_across, ao_interval_max_accel_across);
				init_bounds(ao_sample_accel_through, ao_interval_min_accel_through, ao_interval_max_accel_through);
			}
#endif
#if HAS_ACCEL
			{
#if HAS_BARO
			check_re_boost:
#endif
				ao_coast_avg_accel = ao_coast_avg_accel + ((ao_accel - ao_coast_avg_accel) >> 5);
				if (ao_coast_avg_accel > AO_MSS_TO_ACCEL(20)) {
					ao_boost_tick = ao_sample_tick;
					ao_flight_state = ao_flight_boost;
					ao_wakeup(&ao_flight_state);
				}
			}
#endif

			break;
#if HAS_BARO
		case ao_flight_drogue:

			/* drogue to main deploy:
			 *
			 * barometer: reach main deploy altitude
			 *
			 * Would like to use the accelerometer for this test, but
			 * the orientation of the flight computer is unknown after
			 * drogue deploy, so we ignore it. Could also detect
			 * high descent rate using the pressure sensor to
			 * recognize drogue deploy failure and eject the main
			 * at that point. Perhaps also use the drogue sense lines
			 * to notice continutity?
			 */
			if (ao_height <= ao_config.main_deploy)
			{
				ao_flight_state = ao_flight_main;
				ao_wakeup(&ao_flight_state);

				/*
				 * Start recording min/max height
				 * to figure out when the rocket has landed
				 */

				/* initialize interval values */
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;

				ao_interval_min_height = ao_interval_max_height = ao_avg_height;
			}
			break;

			/* fall through... */
		case ao_flight_main:

			/* main to land:
			 *
			 * barometer: altitude stable
			 */
			if (ao_avg_height < ao_interval_min_height)
				ao_interval_min_height = ao_avg_height;
			if (ao_avg_height > ao_interval_max_height)
				ao_interval_max_height = ao_avg_height;

			if ((AO_TICK_SIGNED) (ao_sample_tick - ao_interval_end) >= 0) {
				if (ao_interval_max_height - ao_interval_min_height <= AO_M_TO_HEIGHT(4))
				{
					ao_flight_state = ao_flight_landed;
					ao_wakeup(&ao_flight_state);
#if HAS_ADC
					/* turn off the ADC capture */
					ao_timer_set_adc_interval(0);
#endif
				}
				ao_interval_min_height = ao_interval_max_height = ao_avg_height;
				ao_interval_end = ao_sample_tick + AO_INTERVAL_TICKS;
			}
			break;
#endif /* HAS_BARO */
#if HAS_FLIGHT_DEBUG
		case ao_flight_test:
#if HAS_GYRO
			printf ("angle %4d pitch %7ld yaw %7ld roll %7ld\n",
				ao_sample_orient,
				((ao_sample_pitch << 9) - ao_ground_pitch) >> 9,
				((ao_sample_yaw << 9) - ao_ground_yaw) >> 9,
				((ao_sample_roll << 9) - ao_ground_roll) >> 9);
#endif
			flush();
			break;
#endif /* HAS_FLIGHT_DEBUG */
		default:
			break;
		}
	}
}

#if HAS_FLIGHT_DEBUG
static inline int int_part(ao_v_t i)	{ return i >> 4; }
static inline int frac_part(ao_v_t i)	{ return ((i & 0xf) * 100 + 8) / 16; }

static void
ao_flight_dump(void)
{
#if HAS_ACCEL
	ao_v_t	accel;

	accel = ((ao_config.accel_plus_g - ao_sample_accel) * ao_accel_scale) >> 16;
#endif

	printf ("sample:\n");
	printf ("  tick        %d\n", ao_sample_tick);
#if HAS_BARO
	printf ("  raw pres    %ld\n", ao_sample_pres);
#endif
#if HAS_ACCEL
	printf ("  raw accel   %d\n", ao_sample_accel);
#endif
#if HAS_BARO
	printf ("  ground pres %ld\n", ao_ground_pres);
	printf ("  ground alt  %ld\n", ao_ground_height);
#endif
#if HAS_ACCEL
	printf ("  raw accel   %d\n", ao_sample_accel);
	printf ("  groundaccel %d\n", ao_ground_accel);
	printf ("  accel_2g    %d\n", ao_accel_2g);
#endif

#if HAS_BARO
	printf ("  alt         %ld\n", ao_sample_alt);
	printf ("  height      %ld\n", ao_sample_height);
#endif

#if HAS_ACCEL
	printf ("  accel       %d.%02d\n", int_part(accel), frac_part(accel));
#endif


	printf ("kalman:\n");
	printf ("  height      %ld\n", ao_height);
	printf ("  speed       %d.%02d\n", int_part(ao_speed), frac_part(ao_speed));
	printf ("  accel       %d.%02d\n", int_part(ao_accel), frac_part(ao_accel));
	printf ("  max_height  %ld\n", ao_max_height);
	printf ("  avg_height  %ld\n", ao_avg_height);
	printf ("  error_h     %ld\n", ao_error_h);
#if !HAS_ACCEL
	printf ("  error_avg   %d\n", ao_error_h_sq_avg);
#endif
}

static void
ao_gyro_test(void)
{
	ao_flight_state = ao_flight_test;
	ao_getchar();
	ao_flight_state = ao_flight_idle;
}

uint8_t ao_orient_test;

static void
ao_orient_test_select(void)
{
	ao_orient_test = !ao_orient_test;
	printf("orient test %d\n", ao_orient_test);
}

const struct ao_cmds ao_flight_cmds[] = {
	{ ao_flight_dump, 	"F\0Dump flight status" },
	{ ao_gyro_test,		"G\0Test gyro code" },
	{ ao_orient_test_select,"O\0Test orientation code" },
	{ 0, NULL },
};
#endif

static struct ao_task	flight_task;

void
ao_flight_init(void)
{
	ao_flight_state = ao_flight_startup;
#if HAS_FLIGHT_DEBUG
	ao_cmd_register(&ao_flight_cmds[0]);
#endif
	ao_add_task(&flight_task, ao_flight, "flight");
}
