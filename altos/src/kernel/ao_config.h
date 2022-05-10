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

#ifndef _AO_CONFIG_H_
#define _AO_CONFIG_H_

#include <ao_pyro.h>

#ifndef USE_STORAGE_CONFIG
#define USE_STORAGE_CONFIG 1
#endif

#ifndef USE_EEPROM_CONFIG
#define USE_EEPROM_CONFIG 0
#endif

#ifndef FLIGHT_LOG_APPEND
#define FLIGHT_LOG_APPEND 0
#endif

#if USE_STORAGE_CONFIG

#include <ao_storage.h>

#define ao_config_setup() 		ao_storage_setup()
#define ao_config_erase()		ao_storage_erase(ao_storage_config, ao_storage_block)
#define ao_config_write(pos,bytes, len)	ao_storage_write(ao_storage_config+(pos), bytes, len)
#define ao_config_read(pos,bytes, len)	ao_storage_read(ao_storage_config+(pos), bytes, len)
#define ao_config_flush()		ao_storage_flush()

#endif

#if USE_EEPROM_CONFIG

#include <ao_eeprom.h>

#define ao_config_setup()
#define ao_config_erase()
#define ao_config_write(pos,bytes, len)	ao_eeprom_write(pos, bytes, len)
#define ao_config_read(pos,bytes, len)	ao_eeprom_read(pos, bytes, len)
#define ao_config_flush()

#endif

#define AO_CONFIG_MAJOR	1
#define AO_CONFIG_MINOR	25

/* All cc1200 devices support limiting TX power to 10mW */
#if !defined(HAS_RADIO_10MW) && defined(AO_CC1200_SPI)
#define HAS_RADIO_10MW 1
#endif

#define AO_AES_LEN 16

extern uint8_t ao_config_aes_seq;

struct ao_config {
	uint8_t		major;
	uint8_t		minor;
	uint16_t	main_deploy;
	int16_t		accel_plus_g;		/* changed for minor version 2 */
	uint8_t		_legacy_radio_channel;
	char		callsign[AO_MAX_CALLSIGN + 1];
	uint8_t		apogee_delay;		/* minor version 1 */
	int16_t		accel_minus_g;		/* minor version 2 */
	uint32_t	radio_cal;		/* minor version 3 */
	uint32_t	flight_log_max;		/* minor version 4 */
	uint8_t		ignite_mode;		/* minor version 5 */
	uint8_t		pad_orientation;	/* minor version 6 */
	uint32_t	radio_setting;		/* minor version 7 */
	uint8_t		radio_enable;		/* minor version 8 */
	uint8_t		aes_key[AO_AES_LEN];	/* minor version 9 */
	uint32_t	frequency;		/* minor version 10 */
	uint16_t	apogee_lockout;		/* minor version 11 */
#if AO_PYRO_NUM
	struct ao_pyro	pyro[AO_PYRO_NUM];	/* minor version 12 */
#endif
	uint16_t	aprs_interval;		/* minor version 13 */
#if HAS_RADIO_POWER
	uint8_t		radio_power;		/* minor version 14 */
#endif
#if HAS_RADIO_AMP
	uint8_t		radio_amp;		/* minor version 14 */
#endif
#if HAS_IMU
	int16_t		accel_zero_along;	/* minor version 15 */
	int16_t		accel_zero_across;	/* minor version 15 */
	int16_t		accel_zero_through;	/* minor version 15 */
#endif
#if HAS_BEEP
	uint8_t		mid_beep;		/* minor version 16 */
#endif
#if HAS_TRACKER
	uint16_t	tracker_motion;		/* minor version 17 */
	uint8_t		tracker_interval;	/* minor version 17 */
#endif
#if AO_PYRO_NUM
	uint16_t	pyro_time;		/* minor version 18 */
#endif
#if HAS_APRS
	uint8_t		aprs_ssid;		/* minor version 19 */
#endif
#if HAS_RADIO_RATE
	uint8_t		radio_rate;		/* minor version 20 */
#endif
#if HAS_RADIO_FORWARD
	uint32_t	send_frequency;		/* minor version 21 */
#endif
#if HAS_APRS
	uint8_t		aprs_format;		/* minor version 22 */
#endif
#if HAS_FIXED_PAD_BOX
	uint8_t		pad_box;		/* minor version 22 */
	uint8_t		pad_idle;		/* minor version 23 */
#endif
#if HAS_APRS
	uint8_t		aprs_offset;		/* minor version 24 */
#endif
#if HAS_RADIO_10MW
	uint8_t		radio_10mw;		/* minor version 25 */
#endif
};

struct ao_config_1_24 {
	uint8_t		major;
	uint8_t		minor;
	uint16_t	main_deploy;
	int16_t		accel_plus_g;		/* changed for minor version 2 */
	uint8_t		_legacy_radio_channel;
	char		callsign[AO_MAX_CALLSIGN + 1];
	uint8_t		apogee_delay;		/* minor version 1 */
	int16_t		accel_minus_g;		/* minor version 2 */
	uint32_t	radio_cal;		/* minor version 3 */
	uint32_t	flight_log_max;		/* minor version 4 */
	uint8_t		ignite_mode;		/* minor version 5 */
	uint8_t		pad_orientation;	/* minor version 6 */
	uint32_t	radio_setting;		/* minor version 7 */
	uint8_t		radio_enable;		/* minor version 8 */
	uint8_t		aes_key[AO_AES_LEN];	/* minor version 9 */
	uint32_t	frequency;		/* minor version 10 */
	uint16_t	apogee_lockout;		/* minor version 11 */
#if AO_PYRO_NUM
	struct ao_pyro_1_24	pyro[AO_PYRO_NUM];	/* minor version 12 */
#endif
	uint16_t	aprs_interval;		/* minor version 13 */
#if HAS_RADIO_POWER
	uint8_t		radio_power;		/* minor version 14 */
#endif
#if HAS_RADIO_AMP
	uint8_t		radio_amp;		/* minor version 14 */
#endif
#if HAS_IMU
	int16_t		accel_zero_along;	/* minor version 15 */
	int16_t		accel_zero_across;	/* minor version 15 */
	int16_t		accel_zero_through;	/* minor version 15 */
#endif
#if HAS_BEEP
	uint8_t		mid_beep;		/* minor version 16 */
#endif
#if HAS_TRACKER
	uint16_t	tracker_motion;		/* minor version 17 */
	uint8_t		tracker_interval;	/* minor version 17 */
#endif
#if AO_PYRO_NUM
	uint16_t	pyro_time;		/* minor version 18 */
#endif
#if HAS_APRS
	uint8_t		aprs_ssid;		/* minor version 19 */
#endif
#if HAS_RADIO_RATE
	uint8_t		radio_rate;		/* minor version 20 */
#endif
#if HAS_RADIO_FORWARD
	uint32_t	send_frequency;		/* minor version 21 */
#endif
#if HAS_APRS
	uint8_t		aprs_format;		/* minor version 22 */
#endif
#if HAS_FIXED_PAD_BOX
	uint8_t		pad_box;		/* minor version 22 */
	uint8_t		pad_idle;		/* minor version 23 */
#endif
#if HAS_APRS
	uint8_t		aprs_offset;		/* minor version 24 */
#endif
};

#define AO_APRS_FORMAT_COMPRESSED	0
#define AO_APRS_FORMAT_UNCOMPRESSED	1
#define AO_CONFIG_DEFAULT_APRS_FORMAT	AO_APRS_FORMAT_COMPRESSED

#if HAS_RADIO_FORWARD
extern uint32_t	ao_send_radio_setting;
#endif

#define AO_IGNITE_MODE_DUAL		0
#define AO_IGNITE_MODE_APOGEE		1
#define AO_IGNITE_MODE_MAIN		2
#define AO_IGNITE_MODE_BOOSTER		3

#define AO_RADIO_ENABLE_CORE		1
#define AO_RADIO_DISABLE_TELEMETRY	2
#define AO_RADIO_DISABLE_RDF		4

#define AO_PAD_ORIENTATION_ANTENNA_UP	0
#define AO_PAD_ORIENTATION_ANTENNA_DOWN	1

#ifndef AO_CONFIG_MAX_SIZE
#define AO_CONFIG_MAX_SIZE	128
#endif

/* Make sure AO_CONFIG_MAX_SIZE is big enough */
typedef uint8_t	config_check_space[(int) (AO_CONFIG_MAX_SIZE - sizeof (struct ao_config))];

extern struct ao_config ao_config;
extern uint8_t ao_config_loaded;

void
_ao_config_edit_start(void);

void
_ao_config_edit_finish(void);

void
ao_config_get(void);

void
ao_config_put(void);

void
ao_config_set_radio(void);

void
ao_config_log_fix_append(void);

void
ao_config_init(void);

#endif /* _AO_CONFIG_H_ */
