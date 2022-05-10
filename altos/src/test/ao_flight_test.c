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

#define _GNU_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#define log ao_log_data

#define GRAVITY 9.80665

#define AO_HERTZ	100

#define HAS_ADC 1
#define AO_DATA_RING	64
#define ao_data_ring_next(n)	(((n) + 1) & (AO_DATA_RING - 1))
#define ao_data_ring_prev(n)	(((n) - 1) & (AO_DATA_RING - 1))

#if 0
#define AO_M_TO_HEIGHT(m)	((int16_t) (m))
#define AO_MS_TO_SPEED(ms)	((int16_t) ((ms) * 16))
#define AO_MSS_TO_ACCEL(mss)	((int16_t) ((mss) * 16))
#endif

#define AO_GPS_NEW_DATA		1
#define AO_GPS_NEW_TRACKING	2

int ao_gps_new;

#if !defined(TELEMEGA) && !defined(TELEMETRUM_V2) && !defined(EASYMINI) && !defined(EASYMOTOR_V_2)
#define TELEMETRUM_V1 1
#endif

#if TELEMEGA
#define AO_ADC_NUM_SENSE	6
#define HAS_MS5607		1
#define HAS_MPU6000		1
#define HAS_MMA655X		1
#define HAS_HMC5883 		1
#define HAS_BEEP		1
#define HAS_BARO		1
#define AO_CONFIG_MAX_SIZE	1024
#define AO_MMA655X_INVERT	0

struct ao_adc {
	int16_t			sense[AO_ADC_NUM_SENSE];
	int16_t			v_batt;
	int16_t			v_pbatt;
	int16_t			temp;
};
#endif

#if TELEMETRUM_V2
#define AO_ADC_NUM_SENSE	2
#define HAS_MS5607		1
#define HAS_MMA655X		1
#define AO_MMA655X_INVERT	0
#define HAS_BEEP		1
#define HAS_BARO		1
#define AO_CONFIG_MAX_SIZE	1024

struct ao_adc {
	int16_t			sense_a;
	int16_t			sense_m;
	int16_t			v_batt;
	int16_t			temp;
};
#endif

#if EASYMINI
#define AO_ADC_NUM_SENSE	2
#define HAS_MS5607		1
#define HAS_BEEP		1
#define AO_CONFIG_MAX_SIZE	1024

struct ao_adc {
	int16_t			sense_a;
	int16_t			sense_m;
	int16_t			v_batt;
};
#endif

#if TELEMETRUM_V1
/*
 * One set of samples read from the A/D converter
 */
struct ao_adc {
	int16_t		accel;		/* accelerometer */
	int16_t		pres;		/* pressure sensor */
	int16_t		pres_real;	/* unclipped */
	int16_t		temp;		/* temperature sensor */
	int16_t		v_batt;		/* battery voltage */
	int16_t		sense_d;	/* drogue continuity sense */
	int16_t		sense_m;	/* main continuity sense */
};

#ifndef HAS_ACCEL
#define HAS_ACCEL 1
#define HAS_ACCEL_REF 0
#endif
#define HAS_BARO		1

#endif

#if EASYMOTOR_V_2
#define AO_ADC_NUM_SENSE	0
#define HAS_ADXL375		1
#define HAS_BEEP		1
#define AO_CONFIG_MAX_SIZE	1024
#define USE_ADXL375_IMU		1
#define AO_ADXL375_INVERT	0
#define HAS_IMU			1
#define AO_ADXL375_AXIS		x
#define AO_ADXL375_ACROSS_AXIS	y
#define AO_ADXL375_THROUGH_AXIS	z

struct ao_adc {
	int16_t			pressure;
	int16_t			v_batt;
};

#endif

#define const

#define HAS_FLIGHT 1
#define HAS_IGNITE 1
#define HAS_USB 1
#define HAS_GPS 1

int16_t
ao_time(void);

void
ao_dump_state(void);

#define ao_tick_count	(ao_time())
#define ao_wakeup(wchan) ao_dump_state()

#include <ao_data.h>
#include <ao_log.h>
#include <ao_telemetry.h>
#include <ao_sample.h>

#if TELEMEGA
int ao_gps_count;
struct ao_telemetry_location ao_gps_first;
struct ao_telemetry_location ao_gps_prev;
struct ao_telemetry_location ao_gps_static;

struct ao_telemetry_satellite ao_gps_tracking;

static inline double sqr(double a) { return a * a; }

void
cc_great_circle (double start_lat, double start_lon,
		 double end_lat, double end_lon,
		 double *dist, double *bearing)
{
	const double rad = M_PI / 180;
	const double earth_radius = 6371.2 * 1000;	/* in meters */
	double lat1 = rad * start_lat;
	double lon1 = rad * -start_lon;
	double lat2 = rad * end_lat;
	double lon2 = rad * -end_lon;

//	double d_lat = lat2 - lat1;
	double d_lon = lon2 - lon1;

	/* From http://en.wikipedia.org/wiki/Great-circle_distance */
	double vdn = sqrt(sqr(cos(lat2) * sin(d_lon)) +
			  sqr(cos(lat1) * sin(lat2) -
			      sin(lat1) * cos(lat2) * cos(d_lon)));
	double vdd = sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(d_lon);
	double d = atan2(vdn,vdd);
	double course;

	if (cos(lat1) < 1e-20) {
		if (lat1 > 0)
			course = M_PI;
		else
			course = -M_PI;
	} else {
		if (d < 1e-10)
			course = 0;
		else
			course = acos((sin(lat2)-sin(lat1)*cos(d)) /
				      (sin(d)*cos(lat1)));
		if (sin(lon2-lon1) > 0)
			course = 2 * M_PI-course;
	}
	*dist = d * earth_radius;
	*bearing = course * 180/M_PI;
}

double
ao_distance_from_pad(void)
{
	double	dist, bearing;
	if (!ao_gps_count)
		return 0;

	cc_great_circle(ao_gps_first.latitude / 1e7,
			ao_gps_first.longitude / 1e7,
			ao_gps_static.latitude / 1e7,
			ao_gps_static.longitude / 1e7,
			&dist, &bearing);
	return dist;
}

double
ao_gps_angle(void)
{
	double	dist, bearing;
	double	height;
	double	angle;

	if (ao_gps_count < 2)
		return 0;

	cc_great_circle(ao_gps_prev.latitude / 1e7,
			ao_gps_prev.longitude / 1e7,
			ao_gps_static.latitude / 1e7,
			ao_gps_static.longitude / 1e7,
			&dist, &bearing);
	height = AO_TELEMETRY_LOCATION_ALTITUDE(&ao_gps_static) - AO_TELEMETRY_LOCATION_ALTITUDE(&ao_gps_prev);

	angle = atan2(dist, height);
	return angle * 180/M_PI;
}
#endif

#define to_fix16(x) ((int16_t) ((x) * 65536.0 + 0.5))
#define to_fix32(x) ((int32_t) ((x) * 65536.0 + 0.5))
#define from_fix(x)	((x) >> 16)

#define ACCEL_NOSE_UP	(ao_accel_2g >> 2)

extern enum ao_flight_state ao_flight_state;

#define false 0
#define true 1

volatile struct ao_data ao_data_ring[AO_DATA_RING];
volatile uint8_t ao_data_head;
int	ao_summary = 0;

#define ao_led_on(l)
#define ao_led_off(l)
#define ao_timer_set_adc_interval(i)
#define ao_cmd_register(c)
#define ao_usb_disable()
#define ao_telemetry_set_interval(x)
#define ao_rdf_set(rdf)
#define ao_packet_slave_start()
#define ao_packet_slave_stop()
#define flush()

enum ao_igniter {
	ao_igniter_drogue = 0,
	ao_igniter_main = 1
};

struct ao_data ao_data_static;

int	drogue_height;
double	drogue_time;
int	main_height;
double	main_time;

int	tick_offset;

static ao_k_t	ao_k_height;
static double	simple_speed;

int16_t
ao_time(void)
{
	return ao_data_static.tick;
}

void
ao_delay(int16_t interval)
{
	return;
}

void
ao_ignite(enum ao_igniter igniter)
{
	double time = (double) (ao_data_static.tick + tick_offset) / 100;

	if (igniter == ao_igniter_drogue) {
		drogue_time = time;
		drogue_height = ao_k_height >> 16;
	} else {
		main_time = time;
		main_height = ao_k_height >> 16;
	}
}

struct ao_task {
	int dummy;
};

#define ao_add_task(t,f,n) ((void) (t))

#define ao_log_start()
#define ao_log_stop()

#define AO_MS_TO_TICKS(ms)	((ms) / 10)
#define AO_SEC_TO_TICKS(s)	((s) * 100)

#define AO_FLIGHT_TEST	1

int	ao_flight_debug;

struct ao_eeprom	*eeprom;
uint32_t		eeprom_offset;

FILE *emulator_in;
char *emulator_app;
char *emulator_name;
char *emulator_info;
double emulator_error_max = 4;
double emulator_height_error_max = 20;	/* noise in the baro sensor */

void
ao_sleep(void *wchan);

const char * const ao_state_names[] = {
	"startup", "idle", "pad", "boost", "fast",
	"coast", "drogue", "main", "landed", "invalid"
};

struct ao_cmds {
	void		(*func)(void);
	const char	*help;
};

#define AO_NEED_ALTITUDE_TO_PRES 1
#if TELEMEGA || TELEMETRUM_V2 || EASYMINI
#include "ao_convert_pa.c"
#include <ao_ms5607.h>
struct ao_ms5607_prom	ao_ms5607_prom;
#include "ao_ms5607_convert.c"
#if TELEMEGA
#define AO_PYRO_NUM	4
#include <ao_pyro.h>
#endif
#else
#include "ao_convert.c"
#endif

#include <ao_config.h>
#include <ao_fake_flight.h>
#include <ao_eeprom_read.h>
#include <ao_log.h>

#define ao_config_get()

struct ao_config ao_config;

extern int16_t ao_ground_accel, ao_flight_accel;
extern int16_t ao_accel_2g;

typedef int16_t	accel_t;

uint16_t	ao_serial_number;
int16_t		ao_flight_number;

extern AO_TICK_TYPE	ao_sample_tick;

#if HAS_BARO
extern alt_t	ao_sample_height;
#endif
extern accel_t	ao_sample_accel;
extern int32_t	ao_accel_scale;
#if HAS_BARO
extern alt_t	ao_ground_height;
extern alt_t	ao_sample_alt;
#endif

double ao_sample_qangle;

AO_TICK_TYPE	ao_sample_prev_tick;
AO_TICK_TYPE	prev_tick;


#include "ao_kalman.c"
#include "ao_sqrt.c"
#include "ao_sample.c"
#include "ao_flight.c"
#include "ao_data.c"
#if TELEMEGA
#define AO_PYRO_NUM	4

#define AO_PYRO_0	0
#define AO_PYRO_1	1
#define AO_PYRO_2	2
#define AO_PYRO_3	3

#define PYRO_DBG	1

static void
ao_pyro_pin_set(uint8_t pin, uint8_t value)
{
	printf ("set pyro %d %d\n", pin, value);
}

#include "ao_pyro.c"
#endif
#include "ao_eeprom_read.c"
#include "ao_eeprom_read_old.c"

#define to_double(f)	((f) / 65536.0)

static int	ao_records_read = 0;
static int	ao_eof_read = 0;
#if !EASYMINI
static int	ao_flight_ground_accel;
#endif
static int	ao_flight_started = 0;
static int	ao_test_max_height;
static double	ao_test_max_height_time;
static int	ao_test_main_height;
static double	ao_test_main_height_time;
static double	ao_test_landed_time;
static double	ao_test_landed_height;
static double	ao_test_landed_time;
static int	landed_set;
static double	landed_time;
static double	landed_height;
#if AO_PYRO_NUM
static uint16_t	pyros_fired;
#endif

#if HAS_MPU6000
static struct ao_mpu6000_sample	ao_ground_mpu6000;
#endif

void
ao_test_exit(void)
{
	double	drogue_error;
	double	main_error;
	double	landed_error;
	double	landed_time_error;

	if (!ao_test_main_height_time) {
		ao_test_main_height_time = ao_test_max_height_time;
		ao_test_main_height = ao_test_max_height;
	}
	drogue_error = fabs(ao_test_max_height_time - drogue_time);
	main_error = fabs(ao_test_main_height_time - main_time);
	landed_error = fabs(ao_test_landed_height - landed_height);
	landed_time_error = ao_test_landed_time - landed_time;
	if (drogue_error > emulator_error_max || main_error > emulator_error_max) {
		printf ("%s %s\n",
			emulator_app, emulator_name);
		if (emulator_info)
			printf ("\t%s\n", emulator_info);
		printf ("\tApogee error %g\n", drogue_error);
		printf ("\tMain error %g\n", main_error);
		printf ("\tLanded height error %g\n", landed_error);
		printf ("\tLanded time error %g\n", landed_time_error);
		printf ("\tActual: apogee: %d at %7.2f main: %d at %7.2f landed %7.2f at %7.2f\n",
			ao_test_max_height, ao_test_max_height_time,
			ao_test_main_height, ao_test_main_height_time,
			ao_test_landed_height, ao_test_landed_time);
		printf ("\tComputed: apogee: %d at %7.2f main: %d at %7.2f landed %7.2f at %7.2f\n",
			drogue_height, drogue_time, main_height, main_time,
			landed_height, landed_time);
		exit (1);
	}
	exit(0);
}

#ifdef TELEMEGA
struct ao_azel {
	int	az;
	int	el;
};

static void
azel (struct ao_azel *r, struct ao_quaternion *q)
{
	double	v;

	r->az = floor (atan2(q->y, q->x) * 180/M_PI + 0.5);
	v = sqrt (q->x*q->x + q->y*q->y);
	r->el = floor (atan2(q->z, v) * 180/M_PI + 0.5);
}
#endif

void
ao_insert(void)
{
	double	time;

	ao_data_ring[ao_data_head] = ao_data_static;
	if (ao_flight_state != ao_flight_startup) {
#if HAS_ACCEL
		double  accel = ((ao_flight_ground_accel - ao_data_accel(&ao_data_static)) * GRAVITY * 2.0) /
			(ao_config.accel_minus_g - ao_config.accel_plus_g);
#else
		double	accel = 0.0;
#endif

		(void) accel;
		if (!tick_offset)
			tick_offset = -ao_data_static.tick;
		if ((prev_tick - ao_data_static.tick) > 0x400)
			tick_offset += 65536;
		if (prev_tick) {
			int ticks = ao_data_static.tick - prev_tick;
			if (ticks < 0)
				ticks += 65536;
			simple_speed += accel * ticks / 100.0;
		}
		prev_tick = ao_data_static.tick;
		time = (double) (ao_data_static.tick + tick_offset) / 100;

		double height = 0;
#if HAS_BARO
#if TELEMEGA || TELEMETRUM_V2 || EASYMINI
		ao_ms5607_convert(&ao_data_static.ms5607_raw, &ao_data_static.ms5607_cooked);
		height = ao_pa_to_altitude(ao_data_static.ms5607_cooked.pres) - ao_ground_height;

		/* Hack to skip baro spike at accidental drogue charge
		 * firing in 2015-09-26-serial-2093-flight-0012.eeprom
		 * so we can test the kalman filter with this data. Just
		 * keep reporting the same baro value across the pressure spike
		 */
		{
			static struct ao_ms5607_sample save;
			if (ao_serial_number == 2093 && ao_flight_number == 12 && 32.5 < time && time < 33.7) {
				ao_data_ring[ao_data_head].ms5607_raw = save;
			} else {
				save = ao_data_static.ms5607_raw;
			}
		}
#else
		height = ao_pres_to_altitude(ao_data_static.adc.pres_real) - ao_ground_height;
#endif
#endif

		if (ao_test_max_height < height) {
			ao_test_max_height = height;
			ao_test_max_height_time = time;
			ao_test_landed_height = height;
			ao_test_landed_time = time;
		}
		if (height > ao_config.main_deploy) {
			ao_test_main_height_time = time;
			ao_test_main_height = height;
		}

		if (ao_test_landed_height > height) {
			ao_test_landed_height = height;
			ao_test_landed_time = time;
		}

		if (ao_flight_state == ao_flight_landed && !landed_set) {
			landed_set = 1;
			landed_time = time;
			landed_height = height;
		}

		if (!ao_summary) {
#if TELEMEGA
			static struct ao_quaternion	ao_ground_mag;
			static int			ao_ground_mag_set;

			if (!ao_ground_mag_set) {
				ao_quaternion_init_vector (&ao_ground_mag,
							   ao_data_mag_across(&ao_data_static),
							   ao_data_mag_through(&ao_data_static),
							   ao_data_mag_along(&ao_data_static));
				ao_quaternion_normalize(&ao_ground_mag, &ao_ground_mag);
				ao_quaternion_rotate(&ao_ground_mag, &ao_ground_mag, &ao_rotation);
				ao_ground_mag_set = 1;
			}

			struct ao_quaternion		ao_mag, ao_mag_rot;

			ao_quaternion_init_vector(&ao_mag,
						  ao_data_mag_across(&ao_data_static),
						  ao_data_mag_through(&ao_data_static),
						  ao_data_mag_along(&ao_data_static));

			ao_quaternion_normalize(&ao_mag, &ao_mag);
			ao_quaternion_rotate(&ao_mag_rot, &ao_mag, &ao_rotation);

			float				ao_dot;
			int				ao_mag_angle;

			ao_dot = ao_quaternion_dot(&ao_mag_rot, &ao_ground_mag);

			struct ao_azel			ground_azel, mag_azel, rot_azel;

			azel(&ground_azel, &ao_ground_mag);
			azel(&mag_azel, &ao_mag);
			azel(&rot_azel, &ao_mag_rot);

			ao_mag_angle = floor (acos(ao_dot) * 180 / M_PI + 0.5);

			(void) ao_mag_angle;

			static struct ao_quaternion	ao_x = { .r = 0, .x = 1, .y = 0, .z = 0 };
			struct ao_quaternion		ao_out;

			ao_quaternion_rotate(&ao_out, &ao_x, &ao_rotation);

#if 0
			int	out = floor (atan2(ao_out.y, ao_out.x) * 180 / M_PI);

			printf ("%7.2f state %-8.8s height %8.4f tilt %4d rot %4d mag_tilt %4d mag_rot %4d\n",
				time,
				ao_state_names[ao_flight_state],
				ao_k_height / 65536.0,
				ao_sample_orient, out,
				mag_azel.el,
				mag_azel.az);
#endif
#if 0
			printf ("%7.2f state %-8.8s height %8.4f tilt %4d rot %4d dist %12.2f gps_tilt %4d gps_sats %2d\n",
				time,
				ao_state_names[ao_flight_state],
				ao_k_height / 65536.0,
				ao_sample_orient, out,
				ao_distance_from_pad(),
				(int) floor (ao_gps_angle() + 0.5),
				(ao_gps_static.flags & 0xf) * 10);

#endif
#if 0
			printf ("\t\tstate %-8.8s ground az: %4d el %4d mag az %4d el %4d rot az %4d el %4d el_diff %4d az_diff %4d angle %4d tilt %4d ground %8.5f %8.5f %8.5f cur %8.5f %8.5f %8.5f rot %8.5f %8.5f %8.5f\n",
				ao_state_names[ao_flight_state],
				ground_azel.az, ground_azel.el,
				mag_azel.az, mag_azel.el,
				rot_azel.az, rot_azel.el,
				ground_azel.el - rot_azel.el,
				ground_azel.az - rot_azel.az,
				ao_mag_angle,
				ao_sample_orient,
				ao_ground_mag.x,
				ao_ground_mag.y,
				ao_ground_mag.z,
				ao_mag.x,
				ao_mag.y,
				ao_mag.z,
				ao_mag_rot.x,
				ao_mag_rot.y,
				ao_mag_rot.z);
#endif
#endif

#if 1
			printf("%7.2f height %8.2f accel %8.3f accel_speed %8.3f "
			       "state %d k_height %8.2f k_speed %8.3f k_accel %8.3f avg_height %5d drogue %4d main %4d error %5d"
#if TELEMEGA
			       " angle %5d "
			       "accel_x %8.3f accel_y %8.3f accel_z %8.3f gyro_x %8.3f gyro_y %8.3f gyro_z %8.3f mag_x %8d mag_y %8d, mag_z %8d mag_angle %4d "
			       "avg_accel %8.3f "
#endif
			       "\n",
			       time,
			       height,
			       accel,
			       simple_speed > -100.0 ? simple_speed : -100.0,
			       ao_flight_state * 10,
			       ao_k_height / 65536.0,
			       ao_k_speed / 65536.0 / 16.0,
			       ao_k_accel / 65536.0 / 16.0,
			       ao_avg_height,
			       drogue_height,
			       main_height,
			       ao_error_h_sq_avg
#if TELEMEGA
			       , ao_sample_orient,

			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_x),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_y),
			       ao_mpu6000_accel(ao_data_static.mpu6000.accel_z),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_x - ao_ground_mpu6000.gyro_x),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_y - ao_ground_mpu6000.gyro_y),
			       ao_mpu6000_gyro(ao_data_static.mpu6000.gyro_z - ao_ground_mpu6000.gyro_z),
			       ao_data_static.hmc5883.x,
			       ao_data_static.hmc5883.y,
			       ao_data_static.hmc5883.z,
			       ao_mag_angle,
			       ao_coast_avg_accel / 16.0
#endif
				);
#endif

//			if (ao_flight_state == ao_flight_landed)
//				ao_test_exit();
		}
	}
	ao_data_head = ao_data_ring_next(ao_data_head);
}


uint16_t
uint16(uint8_t *bytes, int off)
{
	return (uint16_t) bytes[off] | (((uint16_t) bytes[off+1]) << 8);
}

int16_t
int16(uint8_t *bytes, int off)
{
	return (int16_t) uint16(bytes, off);
}

uint32_t
uint32(uint8_t *bytes, int off)
{
	return (uint32_t) bytes[off] | (((uint32_t) bytes[off+1]) << 8) |
		(((uint32_t) bytes[off+2]) << 16) |
		(((uint32_t) bytes[off+3]) << 24);
}

int32_t
int32(uint8_t *bytes, int off)
{
	return (int32_t) uint32(bytes, off);
}

uint32_t
uint24(uint8_t *bytes, int off)
{
	return (uint32_t) bytes[off] | (((uint32_t) bytes[off+1]) << 8) |
		(((uint32_t) bytes[off+2]) << 16);
}

int32_t
int24(uint8_t *bytes, int off)
{
	return (int32_t) uint24(bytes, off);
}

static int log_format;

void
ao_sleep(void *wchan)
{
	if (wchan == &ao_data_head) {
#if TELEMEGA
		if (ao_flight_state >= ao_flight_boost && ao_flight_state < ao_flight_landed)
			ao_pyro_check();
#endif
		for (;;) {
			if (ao_records_read > 2 && ao_flight_state == ao_flight_startup)
			{

#if TELEMEGA
				ao_data_static.mpu6000 = ao_ground_mpu6000;
#endif
#if TELEMETRUM_V1
				ao_data_static.adc.accel = ao_flight_ground_accel;
#endif
#if EASYMOTOR_V_2
				ao_data_static.adxl375.AO_ADXL375_AXIS = ao_flight_ground_accel;
#endif

				ao_insert();
				return;
			}

			if (eeprom) {
#if TELEMEGA || EASYMOTOR_V_2
				struct ao_log_mega	*log_mega;
#endif
#if EASYMOTOR_V_2
				struct ao_log_motor	*log_motor;
#endif
#if TELEMETRUM_V2
				struct ao_log_metrum	*log_metrum;
#endif
#if EASYMINI
				struct ao_log_mini	*log_mini;
#endif
#if TELEMETRUM_V1
				struct ao_log_record	*log_record;
#endif

				if (eeprom_offset >= eeprom->len) {
					if (++ao_eof_read >= 1000)
						if (!ao_summary)
							printf ("no more data, exiting simulation\n");
					ao_test_exit();
					ao_data_static.tick += 10;
					ao_insert();
					return;
				}
				switch (eeprom->log_format) {
#if TELEMEGA
				case AO_LOG_FORMAT_TELEMEGA_OLD:
				case AO_LOG_FORMAT_TELEMEGA:
					log_mega = (struct ao_log_mega *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_mega);
					switch (log_mega->type) {
					case AO_LOG_FLIGHT:
						ao_flight_number = log_mega->u.flight.flight;
						ao_flight_ground_accel = log_mega->u.flight.ground_accel;
						ao_flight_started = 1;
						ao_ground_pres = log_mega->u.flight.ground_pres;
						ao_ground_height = ao_pa_to_altitude(ao_ground_pres);
						ao_ground_accel_along = log_mega->u.flight.ground_accel_along;
						ao_ground_accel_across = log_mega->u.flight.ground_accel_across;
						ao_ground_accel_through = log_mega->u.flight.ground_accel_through;
						ao_ground_roll = log_mega->u.flight.ground_roll;
						ao_ground_pitch = log_mega->u.flight.ground_pitch;
						ao_ground_yaw = log_mega->u.flight.ground_yaw;
						ao_ground_mpu6000.accel_x = ao_ground_accel_across;
						ao_ground_mpu6000.accel_y = ao_ground_accel_along;
						ao_ground_mpu6000.accel_z = ao_ground_accel_through;
						ao_ground_mpu6000.gyro_x = ao_ground_pitch >> 9;
						ao_ground_mpu6000.gyro_y = ao_ground_roll >> 9;
						ao_ground_mpu6000.gyro_z = ao_ground_yaw >> 9;
						break;
					case AO_LOG_STATE:
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_mega->tick;
						ao_data_static.ms5607_raw.pres = log_mega->u.sensor.pres;
						ao_data_static.ms5607_raw.temp = log_mega->u.sensor.temp;
						ao_data_static.mpu6000.accel_x = log_mega->u.sensor.accel_x;
						ao_data_static.mpu6000.accel_y = log_mega->u.sensor.accel_y;
						ao_data_static.mpu6000.accel_z = log_mega->u.sensor.accel_z;
						ao_data_static.mpu6000.gyro_x = log_mega->u.sensor.gyro_x;
						ao_data_static.mpu6000.gyro_y = log_mega->u.sensor.gyro_y;
						ao_data_static.mpu6000.gyro_z = log_mega->u.sensor.gyro_z;
						ao_data_static.hmc5883.x = log_mega->u.sensor.mag_x;
						ao_data_static.hmc5883.y = log_mega->u.sensor.mag_y;
						ao_data_static.hmc5883.z = log_mega->u.sensor.mag_z;
						ao_data_static.mma655x = log_mega->u.sensor.accel;
						if (ao_config.pad_orientation != AO_PAD_ORIENTATION_ANTENNA_UP)
							ao_data_static.mma655x = ao_data_accel_invert(ao_data_static.mma655x);
						ao_records_read++;
						ao_insert();
						return;
					case AO_LOG_TEMP_VOLT:
						if (pyros_fired != log_mega->u.volt.pyro) {
							printf("pyro changed %x -> %x\n", pyros_fired, log_mega->u.volt.pyro);
							pyros_fired = log_mega->u.volt.pyro;
						}
						break;
					case AO_LOG_GPS_TIME:
						ao_gps_prev = ao_gps_static;
						ao_gps_static.tick = log_mega->tick;
						ao_gps_static.latitude = log_mega->u.gps.latitude;
						ao_gps_static.longitude = log_mega->u.gps.longitude;
						{
							int16_t	altitude_low = log_mega->u.gps.altitude_low;
							int16_t altitude_high = log_mega->u.gps.altitude_high;
							int32_t altitude = altitude_low | ((int32_t) altitude_high << 16);

							AO_TELEMETRY_LOCATION_SET_ALTITUDE(&ao_gps_static, altitude);
						}
						ao_gps_static.flags = log_mega->u.gps.flags;
						if (!ao_gps_count)
							ao_gps_first = ao_gps_static;
						ao_gps_count++;
						break;
					case AO_LOG_GPS_SAT:
						break;
					}
					break;
#endif
#if TELEMETRUM_V2
				case AO_LOG_FORMAT_TELEMETRUM:
					log_metrum = (struct ao_log_metrum *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_metrum);
					switch (log_metrum->type) {
					case AO_LOG_FLIGHT:
						ao_flight_started = 1;
						ao_flight_number = log_metrum->u.flight.flight;
						ao_flight_ground_accel = log_metrum->u.flight.ground_accel;
						ao_ground_pres = log_metrum->u.flight.ground_pres;
						ao_ground_height = ao_pa_to_altitude(ao_ground_pres);
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_metrum->tick;
						ao_data_static.ms5607_raw.pres = log_metrum->u.sensor.pres;
						ao_data_static.ms5607_raw.temp = log_metrum->u.sensor.temp;
						ao_data_static.mma655x = log_metrum->u.sensor.accel;
						ao_records_read++;
						ao_insert();
						return;
					}
					break;
#endif
#if EASYMINI
				case AO_LOG_FORMAT_EASYMINI1:
				case AO_LOG_FORMAT_EASYMINI2:
				case AO_LOG_FORMAT_TELEMINI3:
					log_mini = (struct ao_log_mini *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_mini);
					switch (log_mini->type) {
					case AO_LOG_FLIGHT:
						ao_flight_started = 1;
						ao_flight_number = log_mini->u.flight.flight;
						ao_ground_pres = log_mini->u.flight.ground_pres;
						ao_ground_height = ao_pa_to_altitude(ao_ground_pres);
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_mini->tick;
						ao_data_static.ms5607_raw.pres = int24(log_mini->u.sensor.pres, 0);
						ao_data_static.ms5607_raw.temp = int24(log_mini->u.sensor.temp, 0);
						ao_records_read++;
						ao_insert();
						return;
					}
					break;
#endif
#if TELEMETRUM_V1
				case AO_LOG_FORMAT_FULL:
				case AO_LOG_FORMAT_TINY:
					log_record = (struct ao_log_record *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_record);
					switch (log_record->type) {
					case AO_LOG_FLIGHT:
						ao_flight_started = 1;
						ao_flight_ground_accel = log_record->u.flight.ground_accel;
						ao_flight_number = log_record->u.flight.flight;
						break;
					case AO_LOG_SENSOR:
					case 'P':	/* ancient telemini */
						ao_data_static.tick = log_record->tick;
						ao_data_static.adc.accel = log_record->u.sensor.accel;
						ao_data_static.adc.pres_real = log_record->u.sensor.pres;
						ao_data_static.adc.pres = log_record->u.sensor.pres;
						ao_records_read++;
						ao_insert();
						return;
					case AO_LOG_TEMP_VOLT:
						ao_data_static.tick = log_record->tick;;
						ao_data_static.adc.temp = log_record->u.temp_volt.temp;
						ao_data_static.adc.v_batt = log_record->u.temp_volt.v_batt;
						break;
					}
					break;
#endif
#if EASYMOTOR_V_2
				case AO_LOG_FORMAT_TELEMEGA_3:
					log_mega = (struct ao_log_mega *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_mega);
					switch (log_mega->type) {
					case AO_LOG_FLIGHT:
						ao_flight_number = log_mega->u.flight.flight;
						ao_flight_ground_accel = log_mega->u.flight.ground_accel;
						ao_flight_started = 1;
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_mega->tick;
						ao_data_static.adxl375.AO_ADXL375_AXIS = -log_mega->u.sensor.accel;
						ao_records_read++;
						ao_insert();
						return;
					}
					break;
				case AO_LOG_FORMAT_TELEMEGA_4:
					log_mega = (struct ao_log_mega *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_mega);
					switch (log_mega->type) {
					case AO_LOG_FLIGHT:
						ao_flight_number = log_mega->u.flight.flight;
						ao_flight_ground_accel = log_mega->u.flight.ground_accel;
						ao_flight_started = 1;
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_mega->tick;
						ao_data_static.adxl375.AO_ADXL375_AXIS = log_mega->u.sensor.accel;
						ao_records_read++;
						ao_insert();
						return;
					}
					break;
				case AO_LOG_FORMAT_EASYMOTOR:
					log_motor = (struct ao_log_motor *) &eeprom->data[eeprom_offset];
					eeprom_offset += sizeof (*log_motor);
					switch (log_motor->type) {
					case AO_LOG_FLIGHT:
						ao_flight_number = log_motor->u.flight.flight;
						ao_flight_ground_accel = log_motor->u.flight.ground_accel;
						ao_flight_started = 1;
						break;
					case AO_LOG_SENSOR:
						ao_data_static.tick = log_motor->tick;
						ao_data_static.adc.pressure = log_motor->u.sensor.pressure;
						ao_data_static.adc.v_batt = log_motor->u.sensor.v_batt;
						ao_data_static.adxl375.AO_ADXL375_AXIS = log_motor->u.sensor.accel_along;
						ao_data_static.adxl375.AO_ADXL375_ACROSS_AXIS = log_motor->u.sensor.accel_across;
						ao_data_static.adxl375.AO_ADXL375_THROUGH_AXIS = log_motor->u.sensor.accel_through;
						ao_records_read++;
						ao_insert();
						return;
					}
					break;
#endif
				default:
					printf ("invalid log format %d\n", log_format);
					ao_test_exit();
				}
			}
		}

	}
}
#define COUNTS_PER_G 264.8

void
ao_dump_state(void)
{
}

static const struct option options[] = {
	{ .name = "summary", .has_arg = 0, .val = 's' },
	{ .name = "debug", .has_arg = 0, .val = 'd' },
	{ .name = "info", .has_arg = 1, .val = 'i' },
	{ 0, 0, 0, 0},
};

void run_flight_fixed(char *name, FILE *f, int summary, char *info)
{
	emulator_name = name;
	emulator_in = f;
	emulator_info = info;
	ao_summary = summary;

	if (strstr(name, ".eeprom") != NULL) {
		char	c;

		c = getc(f);
		ungetc(c, f);
		if (c == '{')
			eeprom = ao_eeprom_read(f);
		else
			eeprom = ao_eeprom_read_old(f);

		if (eeprom) {
#if HAS_MS5607
			ao_ms5607_prom = eeprom->ms5607_prom;
#endif
			ao_config = eeprom->config;
			ao_serial_number = eeprom->serial_number;
			log_format = eeprom->log_format;
		}
	}

	ao_flight_init();
	ao_flight();
}

int
main (int argc, char **argv)
{
	int	summary = 0;
	int	c;
	int	i;
	char	*info = NULL;

#if HAS_ACCEL
	emulator_app="full";
#else
	emulator_app="baro";
#endif
	while ((c = getopt_long(argc, argv, "sdpi:", options, NULL)) != -1) {
		switch (c) {
		case 's':
			summary = 1;
			break;
		case 'd':
			ao_flight_debug = 1;
			break;
		case 'p':
#if PYRO_DBG
			pyro_dbg = 1;
#endif
			break;
		case 'i':
			info = optarg;
			break;
		}
	}

	if (optind == argc)
		run_flight_fixed("<stdin>", stdin, summary, info);
	else
		for (i = optind; i < argc; i++) {
			FILE	*f = fopen(argv[i], "r");
			if (!f) {
				perror(argv[i]);
				continue;
			}
			run_flight_fixed(argv[i], f, summary, info);
			fclose(f);
		}
	exit(0);
}
