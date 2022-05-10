/*
 * Copyright © 2011 Keth Packard <keithp@keithp.com>
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
#include "ao_log.h"
#include "ao_product.h"

static uint16_t ao_telemetry_interval;

#if HAS_RADIO_RATE
static uint16_t ao_telemetry_desired_interval;
#endif

/* TeleMetrum v1.0 just doesn't have enough space to
 * manage the more complicated telemetry scheduling, so
 * it loses the ability to disable telem/rdf separately
 */

#if defined(TELEMETRUM_V_1_0)
#define SIMPLIFY
#endif

#ifdef SIMPLIFY
#define ao_telemetry_time time
#define RDF_SPACE	
#else
#define RDF_SPACE	
static AO_TICK_TYPE	ao_telemetry_time;
#endif

#if HAS_RDF
static RDF_SPACE uint8_t ao_rdf = 0;
static RDF_SPACE AO_TICK_TYPE	ao_rdf_time;
#endif

#if HAS_APRS
static AO_TICK_TYPE ao_aprs_time;

#include <ao_aprs.h>
#endif

#if defined(TELEMEGA)
#define AO_SEND_MEGA	1
#endif

#if defined (TELEMETRUM_V_2_0) || defined (TELEMETRUM_V_3_0)
#define AO_SEND_METRUM	1
#endif

#if defined(TELEMETRUM_V_0_1) || defined(TELEMETRUM_V_0_2) || defined(TELEMETRUM_V_1_0) || defined(TELEMETRUM_V_1_1) || defined(TELEBALLOON_V_1_1) || defined(TELEMETRUM_V_1_2)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMETRUM
#endif

#if defined(TELEMINI_V_1_0)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELEMINI
#endif

#if defined(TELENANO_V_0_1)
#define AO_TELEMETRY_SENSOR	AO_TELEMETRY_SENSOR_TELENANO
#endif

static union ao_telemetry_all	telemetry;

static void
ao_telemetry_send(void)
{
	ao_radio_send(&telemetry, sizeof (telemetry));
	ao_delay(1);
}

#if defined AO_TELEMETRY_SENSOR
/* Send sensor packet */
static void
ao_send_sensor(void)
{
		struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];

	telemetry.generic.tick = packet->tick;
	telemetry.generic.type = AO_TELEMETRY_SENSOR;

	telemetry.sensor.state = ao_flight_state;
#if HAS_ACCEL
	telemetry.sensor.accel = packet->adc.accel;
#else
	telemetry.sensor.accel = 0;
#endif
	telemetry.sensor.pres = ao_data_pres(packet);
	telemetry.sensor.temp = packet->adc.temp;
	telemetry.sensor.v_batt = packet->adc.v_batt;
#if HAS_IGNITE
	telemetry.sensor.sense_d = packet->adc.sense_d;
	telemetry.sensor.sense_m = packet->adc.sense_m;
#else
	telemetry.sensor.sense_d = 0;
	telemetry.sensor.sense_m = 0;
#endif

	telemetry.sensor.acceleration = ao_accel;
	telemetry.sensor.speed = ao_speed;
	telemetry.sensor.height = ao_height;

	telemetry.sensor.ground_pres = ao_ground_pres;
#if HAS_ACCEL
	telemetry.sensor.ground_accel = ao_ground_accel;
	telemetry.sensor.accel_plus_g = ao_config.accel_plus_g;
	telemetry.sensor.accel_minus_g = ao_config.accel_minus_g;
#else
	telemetry.sensor.ground_accel = 0;
	telemetry.sensor.accel_plus_g = 0;
	telemetry.sensor.accel_minus_g = 0;
#endif

	ao_telemetry_send();
}
#endif


#ifdef AO_SEND_MEGA

/* Send mega sensor packet */
static void
ao_send_mega_sensor(void)
{
	struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];

	telemetry.generic.tick = (uint16_t) packet->tick;
#if AO_LOG_NORMALIZED
#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMEGA_5
	telemetry.generic.type = AO_TELEMETRY_MEGA_NORM_MPU6000_MMC5983;
#else
#error unknown normalized log type
#endif

#if HAS_GYRO
	telemetry.mega_norm.orient = (uint8_t) ao_sample_orient;
#endif
	telemetry.mega_norm.accel = ao_data_accel(packet);
	telemetry.mega_norm.pres = ao_data_pres(packet);
	telemetry.mega_norm.temp = ao_data_temp(packet);

#if HAS_MPU6000
	telemetry.mega_norm.accel_along = ao_data_along(packet);
	telemetry.mega_norm.accel_across = ao_data_across(packet);
	telemetry.mega_norm.accel_through = ao_data_through(packet);

	telemetry.mega_norm.gyro_roll = ao_data_roll(packet);
	telemetry.mega_norm.gyro_pitch = ao_data_pitch(packet);
	telemetry.mega_norm.gyro_yaw = ao_data_yaw(packet);
#endif

#if HAS_MMC5983
	telemetry.mega_norm.mag_along = ao_data_mag_along(packet);
	telemetry.mega_norm.mag_across = ao_data_mag_across(packet);
	telemetry.mega_norm.mag_through = ao_data_mag_through(packet);
#endif

#else

#if HAS_BMX160
	telemetry.generic.type = AO_TELEMETRY_MEGA_SENSOR_BMX160;
#else
#if HAS_MPU6000 || HAS_MPU9250
	telemetry.generic.type = AO_TELEMETRY_MEGA_SENSOR_MPU;
#else
#error unknown IMU
#endif
#endif

#if HAS_GYRO
	telemetry.mega_sensor.orient = (uint8_t) ao_sample_orient;
#endif
	telemetry.mega_sensor.accel = ao_data_accel(packet);
	telemetry.mega_sensor.pres = ao_data_pres(packet);
	telemetry.mega_sensor.temp = ao_data_temp(packet);

#if HAS_MPU6000
	telemetry.mega_sensor.accel_x = packet->mpu6000.accel_x;
	telemetry.mega_sensor.accel_y = packet->mpu6000.accel_y;
	telemetry.mega_sensor.accel_z = packet->mpu6000.accel_z;

	telemetry.mega_sensor.gyro_x = packet->mpu6000.gyro_x;
	telemetry.mega_sensor.gyro_y = packet->mpu6000.gyro_y;
	telemetry.mega_sensor.gyro_z = packet->mpu6000.gyro_z;
#endif

#if HAS_HMC5883
	telemetry.mega_sensor.mag_x = packet->hmc5883.x;
	telemetry.mega_sensor.mag_z = packet->hmc5883.z;
	telemetry.mega_sensor.mag_y = packet->hmc5883.y;
#endif

#if HAS_MPU9250
	telemetry.mega_sensor.accel_x = packet->mpu9250.accel_x;
	telemetry.mega_sensor.accel_y = packet->mpu9250.accel_y;
	telemetry.mega_sensor.accel_z = packet->mpu9250.accel_z;

	telemetry.mega_sensor.gyro_x = packet->mpu9250.gyro_x;
	telemetry.mega_sensor.gyro_y = packet->mpu9250.gyro_y;
	telemetry.mega_sensor.gyro_z = packet->mpu9250.gyro_z;

	telemetry.mega_sensor.mag_x = packet->mpu9250.mag_x;
	telemetry.mega_sensor.mag_z = packet->mpu9250.mag_z;
	telemetry.mega_sensor.mag_y = packet->mpu9250.mag_y;
#endif

#if HAS_BMX160
	telemetry.mega_sensor.accel_x = packet->bmx160.acc_x;
	telemetry.mega_sensor.accel_y = packet->bmx160.acc_y;
	telemetry.mega_sensor.accel_z = packet->bmx160.acc_z;

	telemetry.mega_sensor.gyro_x = packet->bmx160.gyr_x;
	telemetry.mega_sensor.gyro_y = packet->bmx160.gyr_y;
	telemetry.mega_sensor.gyro_z = packet->bmx160.gyr_z;

	telemetry.mega_sensor.mag_x = packet->bmx160.mag_x;
	telemetry.mega_sensor.mag_z = packet->bmx160.mag_z;
	telemetry.mega_sensor.mag_y = packet->bmx160.mag_y;
#endif
#endif
	ao_telemetry_send();
}

static int16_t ao_telemetry_mega_data_max;
static int16_t ao_telemetry_mega_data_cur;

/* Send mega data packet */
static void
ao_send_mega_data(void)
{
	if (--ao_telemetry_mega_data_cur <= 0) {
		struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];
		uint8_t	i;

		telemetry.generic.tick = (uint16_t) packet->tick;
		telemetry.generic.type = AO_TELEMETRY_MEGA_DATA;

		telemetry.mega_data.state = ao_flight_state;
		telemetry.mega_data.v_batt = packet->adc.v_batt;
		telemetry.mega_data.v_pyro = packet->adc.v_pbatt;

		/* ADC range is 0-4095, so shift by four to save the high 8 bits */
		for (i = 0; i < AO_ADC_NUM_SENSE; i++)
			telemetry.mega_data.sense[i] = (int8_t) (packet->adc.sense[i] >> 4);

		telemetry.mega_data.ground_pres = ao_ground_pres;
		telemetry.mega_data.ground_accel = ao_ground_accel;
		telemetry.mega_data.accel_plus_g = ao_config.accel_plus_g;
		telemetry.mega_data.accel_minus_g = ao_config.accel_minus_g;

		telemetry.mega_data.acceleration = (int16_t) ao_accel;
		telemetry.mega_data.speed = (int16_t) ao_speed;
		telemetry.mega_data.height = (int16_t) ao_height;

		ao_telemetry_mega_data_cur = ao_telemetry_mega_data_max;
		ao_telemetry_send();
	}
}
#endif /* AO_SEND_MEGA */

#ifdef AO_SEND_METRUM
/* Send telemetrum sensor packet */
static void
ao_send_metrum_sensor(void)
{
	struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];

	telemetry.generic.tick = (uint16_t) packet->tick;
	telemetry.generic.type = AO_TELEMETRY_METRUM_SENSOR;

	telemetry.metrum_sensor.state = ao_flight_state;
#if HAS_ACCEL
	telemetry.metrum_sensor.accel = ao_data_accel(packet);
#endif
	telemetry.metrum_sensor.pres = ao_data_pres(packet);
	telemetry.metrum_sensor.temp = ao_data_temp(packet);

	telemetry.metrum_sensor.acceleration = (int16_t) ao_accel;
	telemetry.metrum_sensor.speed = (int16_t) ao_speed;
	telemetry.metrum_sensor.height = (int16_t) ao_height;

	telemetry.metrum_sensor.v_batt = packet->adc.v_batt;
	telemetry.metrum_sensor.sense_a = packet->adc.sense_a;
	telemetry.metrum_sensor.sense_m = packet->adc.sense_m;

	ao_telemetry_send();
}

static int16_t ao_telemetry_metrum_data_max;
static int16_t ao_telemetry_metrum_data_cur;

/* Send telemetrum data packet */
static void
ao_send_metrum_data(void)
{
	if (--ao_telemetry_metrum_data_cur <= 0) {
		struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];

		telemetry.generic.tick = (uint16_t) packet->tick;
		telemetry.generic.type = AO_TELEMETRY_METRUM_DATA;

		telemetry.metrum_data.ground_pres = ao_ground_pres;
#if HAS_ACCEL
		telemetry.metrum_data.ground_accel = ao_ground_accel;
		telemetry.metrum_data.accel_plus_g = ao_config.accel_plus_g;
		telemetry.metrum_data.accel_minus_g = ao_config.accel_minus_g;
#else
		telemetry.metrum_data.ground_accel = 1;
		telemetry.metrum_data.accel_plus_g = 0;
		telemetry.metrum_data.accel_minus_g = 2;
#endif

		ao_telemetry_metrum_data_cur = ao_telemetry_metrum_data_max;
		ao_telemetry_send();
	}
}
#endif /* AO_SEND_METRUM */

#ifdef AO_SEND_MINI

static void
ao_send_mini(void)
{
	struct ao_data *packet = (struct ao_data *) &ao_data_ring[ao_data_ring_prev(ao_sample_data)];

	telemetry.generic.tick = (uint16_t) packet->tick;
	telemetry.generic.type = AO_SEND_MINI;

	telemetry.mini.state = ao_flight_state;

	telemetry.mini.v_batt = packet->adc.v_batt;
	telemetry.mini.sense_a = packet->adc.sense_a;
	telemetry.mini.sense_m = packet->adc.sense_m;

	telemetry.mini.pres = ao_data_pres(packet);
	telemetry.mini.temp = ao_data_temp(packet);

	telemetry.mini.acceleration = (int16_t) ao_accel;
	telemetry.mini.speed = (int16_t) ao_speed;
	telemetry.mini.height = (int16_t) ao_height;

	telemetry.mini.ground_pres = ao_ground_pres;

	ao_telemetry_send();
}

#endif /* AO_SEND_MINI */

static int16_t ao_telemetry_config_max;
static int16_t ao_telemetry_config_cur;
static uint16_t ao_telemetry_flight_number;

#ifndef ao_telemetry_battery_convert
#define ao_telemetry_battery_convert(a) (a)
#endif

static void
ao_send_configuration(void)
{
	if (--ao_telemetry_config_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_CONFIGURATION;
		telemetry.configuration.device = AO_idProduct_NUMBER;
		telemetry.configuration.flight = ao_telemetry_flight_number;
		telemetry.configuration.config_major = AO_CONFIG_MAJOR;
		telemetry.configuration.config_minor = AO_CONFIG_MINOR;
#if AO_idProduct_NUMBER == 0x25 && HAS_ADC
		/* TeleGPS gets battery voltage instead of apogee delay */
		telemetry.configuration.apogee_delay = (uint16_t) ao_telemetry_battery_convert(ao_data_ring[ao_data_ring_prev(ao_data_head)].adc.v_batt);
#else
		telemetry.configuration.apogee_delay = ao_config.apogee_delay;
		telemetry.configuration.main_deploy = ao_config.main_deploy;
#endif

		telemetry.configuration.flight_log_max = (uint16_t) (ao_config.flight_log_max >> 10);
		memcpy (telemetry.configuration.callsign,
			ao_config.callsign,
			AO_MAX_CALLSIGN);
		memcpy (telemetry.configuration.version,
			ao_version,
			AO_MAX_VERSION);
		ao_telemetry_config_cur = ao_telemetry_config_max;
		ao_telemetry_send();
	}
}

#if HAS_GPS

static int16_t ao_telemetry_gps_max;
static int16_t ao_telemetry_loc_cur;
static int16_t ao_telemetry_sat_cur;

static inline void *
telemetry_bits(struct ao_telemetry_location *l)
{
	return ((uint8_t *) l) + offsetof(struct ao_telemetry_location, flags);
}

static inline size_t
telemetry_size(void)
{
	return sizeof(struct ao_telemetry_location) - offsetof(struct ao_telemetry_location, flags);
}

static void
ao_send_location(void)
{
	if (--ao_telemetry_loc_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_LOCATION;
		ao_mutex_get(&ao_gps_mutex);
		memcpy(telemetry_bits(&telemetry.location),
		       telemetry_bits(&ao_gps_data),
		       telemetry_size());
		telemetry.location.tick = (uint16_t) ao_gps_tick;
		ao_mutex_put(&ao_gps_mutex);
		ao_telemetry_loc_cur = ao_telemetry_gps_max;
		ao_telemetry_send();
	}
}

static void
ao_send_satellite(void)
{
	if (--ao_telemetry_sat_cur <= 0)
	{
		telemetry.generic.type = AO_TELEMETRY_SATELLITE;
		ao_mutex_get(&ao_gps_mutex);
		telemetry.satellite.channels = ao_gps_tracking_data.channels;
		memcpy(&telemetry.satellite.sats,
		       &ao_gps_tracking_data.sats,
		       AO_MAX_GPS_TRACKING * sizeof (struct ao_telemetry_satellite_info));
		ao_mutex_put(&ao_gps_mutex);
		ao_telemetry_sat_cur = ao_telemetry_gps_max;
		ao_telemetry_send();
	}
}
#endif

#if HAS_COMPANION

static int16_t ao_telemetry_companion_max;
static int16_t ao_telemetry_companion_cur;

static void
ao_send_companion(void)
{
	if (--ao_telemetry_companion_cur <= 0) {
		telemetry.generic.type = AO_TELEMETRY_COMPANION;
		telemetry.companion.board_id = (uint8_t) ao_companion_setup.board_id;
		telemetry.companion.update_period = ao_companion_setup.update_period;
		telemetry.companion.channels = ao_companion_setup.channels;
		ao_mutex_get(&ao_companion_mutex);
		memcpy(&telemetry.companion.companion_data,
		       ao_companion_data,
		       ao_companion_setup.channels * 2);
		ao_mutex_put(&ao_companion_mutex);
		ao_telemetry_companion_cur = ao_telemetry_companion_max;
		ao_telemetry_send();
	}
}
#endif

#if HAS_APRS
static void
ao_set_aprs_time(void)
{
	uint16_t interval = ao_config.aprs_interval;

	if ((ao_gps_data.flags & AO_GPS_DATE_VALID) && interval != 0) {
		int second = (ao_gps_data.second / interval + 1) * interval + ao_config.aprs_offset;
		int delta;
		if (second >= 60) {
			second = ao_config.aprs_offset;
			delta = second + 60 - ao_gps_data.second;
		} else {
			delta = second - ao_gps_data.second;
		}
		ao_aprs_time = ao_gps_tick + AO_SEC_TO_TICKS(delta);
	} else {
		ao_aprs_time += AO_SEC_TO_TICKS(ao_config.aprs_interval);
	}
}
#endif

static void
ao_telemetry(void)
{
	AO_TICK_TYPE	time;
	AO_TICK_SIGNED	delay;

	ao_config_get();
	if (!ao_config.radio_enable)
		ao_exit();
	while (!ao_flight_number)
		ao_sleep(&ao_flight_number);

	ao_telemetry_flight_number = ao_flight_number;
#if HAS_LOG
	if (ao_log_full())
		ao_telemetry_flight_number = 0;
#endif
	telemetry.generic.serial = ao_serial_number;
	for (;;) {
		while (ao_telemetry_interval == 0)
			ao_sleep(&telemetry);
		time = ao_time();
		ao_telemetry_time = time;
#if HAS_RDF
		ao_rdf_time = time;
#endif
#if HAS_APRS
		ao_aprs_time = time;
		ao_set_aprs_time();
#endif
		while (ao_telemetry_interval) {
			time = ao_time() + AO_SEC_TO_TICKS(100);
#ifndef SIMPLIFY
			if (!(ao_config.radio_enable & AO_RADIO_DISABLE_TELEMETRY))
#endif
			{
#ifndef SIMPLIFY
				if ( (int16_t) (ao_time() - ao_telemetry_time) >= 0)
#endif
				{
					ao_telemetry_time = ao_time() + ao_telemetry_interval;
# ifdef AO_SEND_MEGA
					ao_send_mega_sensor();
					ao_send_mega_data();
# endif
# ifdef AO_SEND_METRUM
					ao_send_metrum_sensor();
					ao_send_metrum_data();
# endif
# ifdef AO_SEND_MINI
					ao_send_mini();
# endif
# ifdef AO_TELEMETRY_SENSOR
					ao_send_sensor();
# endif
#if HAS_COMPANION
					if (ao_companion_running)
						ao_send_companion();
#endif
#if HAS_GPS
					ao_send_location();
					ao_send_satellite();
#endif
					ao_send_configuration();
				}
#ifndef SIMPLIFY
				time = ao_telemetry_time;
#endif
			}
#if HAS_RDF
			if (ao_rdf
#ifndef SIMPLIFY
			    && !(ao_config.radio_enable & AO_RADIO_DISABLE_RDF)
#endif
				)
			{
				if ((int16_t) (ao_time() - ao_rdf_time) >= 0) {
#if HAS_IGNITE_REPORT
					uint8_t	c;
#endif
					ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
#if HAS_IGNITE_REPORT
					if (ao_flight_state == ao_flight_pad && (c = ao_report_igniter()))
						ao_radio_continuity(c);
					else
#endif
						ao_radio_rdf();
				}
#ifndef SIMPLIFY
				if ((int16_t) (time - ao_rdf_time) > 0)
					time = ao_rdf_time;
#endif
			}
#endif /* HAS_RDF */
#if HAS_APRS
			if (ao_config.aprs_interval != 0) {
				if ((int16_t) (ao_time() - ao_aprs_time) >= 0) {
					ao_set_aprs_time();
					ao_aprs_send();
				}
				if ((int16_t) (time - ao_aprs_time) > 0)
					time = ao_aprs_time;
			}
#endif /* HAS_APRS */
			delay = (AO_TICK_SIGNED) (time - ao_time());
			if (delay > 0) {
				ao_sleep_for(&telemetry, (AO_TICK_TYPE) delay);
			}
		}
	}
}

#if HAS_RADIO_RATE
void
ao_telemetry_reset_interval(void)
{
	ao_telemetry_set_interval(ao_telemetry_desired_interval);
}
#endif

void
ao_telemetry_set_interval(uint16_t interval)
{
	int16_t	cur = 0;

#if HAS_RADIO_RATE
	/* Limit max telemetry rate based on available radio bandwidth.
	 */
	static const uint16_t min_interval[] = {
		/* [AO_RADIO_RATE_38400] = */ AO_MS_TO_TICKS(100),
		/* [AO_RADIO_RATE_9600] = */ AO_MS_TO_TICKS(500),
		/* [AO_RADIO_RATE_2400] = */ AO_MS_TO_TICKS(1000)
	};

	ao_telemetry_desired_interval = interval;
	if (interval && interval < min_interval[ao_config.radio_rate])
		interval = min_interval[ao_config.radio_rate];
#endif
	ao_telemetry_interval = interval;
#if AO_SEND_MEGA
	if (interval > 1)
		ao_telemetry_mega_data_max = 1;
	else
		ao_telemetry_mega_data_max = 2;
	if (ao_telemetry_mega_data_max > cur)
		cur++;
	ao_telemetry_mega_data_cur = cur;
#endif
#if AO_SEND_METRUM
	ao_telemetry_metrum_data_max = (int16_t) (AO_SEC_TO_TICKS(1) / interval);
	if (ao_telemetry_metrum_data_max > cur)
		cur++;
	ao_telemetry_metrum_data_cur = cur;
#endif

#if HAS_COMPANION
	if (!ao_companion_setup.update_period)
		ao_companion_setup.update_period = AO_SEC_TO_TICKS(1);
	ao_telemetry_companion_max = (int16_t) (ao_companion_setup.update_period / interval);
	if (ao_telemetry_companion_max > cur)
		cur++;
	ao_telemetry_companion_cur = cur;
#endif

#if HAS_GPS
	ao_telemetry_gps_max = (int16_t) (AO_SEC_TO_TICKS(1) / interval);
	if (ao_telemetry_gps_max > cur)
		cur++;
	ao_telemetry_loc_cur = cur;
	if (ao_telemetry_gps_max > cur)
		cur++;
	ao_telemetry_sat_cur = cur;
#endif

	ao_telemetry_config_max = (int16_t) (AO_SEC_TO_TICKS(5) / interval);
	if (ao_telemetry_config_max > cur)
		cur++;
	ao_telemetry_config_cur = cur;

#ifndef SIMPLIFY
	ao_telemetry_time = 
#if HAS_RDF
		ao_rdf_time =
#endif
#if HAS_APRS
		ao_aprs_time =
#endif
		ao_time();
#endif
	ao_wakeup(&telemetry);
}

#if HAS_RDF
void
ao_rdf_set(uint8_t rdf)
{
	ao_rdf = rdf;
	if (rdf == 0)
		ao_radio_rdf_abort();
	else {
		ao_rdf_time = ao_time() + AO_RDF_INTERVAL_TICKS;
	}
}
#endif

struct ao_task	ao_telemetry_task;

void
ao_telemetry_init()
{
	ao_add_task(&ao_telemetry_task, ao_telemetry, "telemetry");
}
