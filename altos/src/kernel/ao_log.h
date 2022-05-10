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

#ifndef _AO_LOG_H_
#define _AO_LOG_H_

#include <ao_flight.h>

/*
 * ao_log.c
 */

/* We record flight numbers in the first record of
 * the log. Tasks may wait for this to be initialized
 * by sleeping on this variable.
 */
extern uint16_t ao_flight_number;
extern uint8_t	ao_log_mutex;
extern uint32_t ao_log_current_pos;
extern uint32_t ao_log_end_pos;
extern uint32_t ao_log_start_pos;
extern uint8_t	ao_log_running;
extern enum ao_flight_state ao_log_state;

/* required functions from the underlying log system */

#define AO_LOG_FORMAT_UNKNOWN		0	/* unknown; altosui will have to guess */
#define AO_LOG_FORMAT_FULL		1	/* 8 byte typed log records */
#define AO_LOG_FORMAT_TINY		2	/* two byte state/baro records */
#define AO_LOG_FORMAT_TELEMETRY		3	/* 32 byte ao_telemetry records */
#define AO_LOG_FORMAT_TELESCIENCE	4	/* 32 byte typed telescience records */
#define AO_LOG_FORMAT_TELEMEGA_OLD	5	/* 32 byte typed telemega records */
#define AO_LOG_FORMAT_EASYMINI1		6	/* 16-byte MS5607 baro only, 3.0V supply */
#define AO_LOG_FORMAT_TELEMETRUM	7	/* 16-byte typed telemetrum records */
#define AO_LOG_FORMAT_TELEMINI2		8	/* 16-byte MS5607 baro only, 3.3V supply, cc1111 SoC */
#define AO_LOG_FORMAT_TELEGPS		9	/* 32 byte telegps records */
#define AO_LOG_FORMAT_TELEMEGA		10	/* 32 byte typed telemega records with 32 bit gyro cal */
#define AO_LOG_FORMAT_DETHERM		11	/* 16-byte MS5607 baro only, no ADC */
#define AO_LOG_FORMAT_TELEMINI3		12	/* 16-byte MS5607 baro only, 3.3V supply, stm32f042 SoC */
#define AO_LOG_FORMAT_TELEFIRETWO	13	/* 32-byte test stand data */
#define AO_LOG_FORMAT_EASYMINI2		14	/* 16-byte MS5607 baro only, 3.3V supply, stm32f042 SoC */
#define AO_LOG_FORMAT_TELEMEGA_3	15	/* 32 byte typed telemega records with 32 bit gyro cal and mpu9250 */
#define AO_LOG_FORMAT_EASYMEGA_2	16	/* 32 byte typed telemega records with 32 bit gyro cal, mpu9250 rotated 90° and adxl375 */
#define AO_LOG_FORMAT_TELESTATIC	17	/* 32 byte typed telestatic records */
#define AO_LOG_FORMAT_MICROPEAK2	18	/* 2-byte baro values with header */
#define AO_LOG_FORMAT_TELEMEGA_4	19	/* 32 byte typed telemega records with 32 bit gyro cal and Bmx160 */
#define AO_LOG_FORMAT_EASYMOTOR		20	/* ? byte typed easymotor records with pressure sensor and adxl375 */
#define AO_LOG_FORMAT_TELEMEGA_5	21	/* 32 byte typed telemega records with 32 bit gyro cal, mpu6000 and mmc5983 */
#define AO_LOG_FORMAT_NONE		127	/* No log at all */

/* Return the flight number from the given log slot, 0 if none, -slot on failure */

int32_t
ao_log_flight(uint8_t slot);

/* Checksum the loaded log record */
uint8_t
ao_log_check_data(void);

/* Check to see if the loaded log record is empty */
uint8_t
ao_log_check_clear(void);

/* Check if there is valid log data at the specified location */
#define AO_LOG_VALID	1
#define AO_LOG_EMPTY	0
#define AO_LOG_INVALID 	-1

int8_t
ao_log_check(uint32_t pos);

/* Flush the log */
void
ao_log_flush(void);

/* Logging thread main routine */
void
ao_log(void);

/* functions provided in ao_log.c */

/* Figure out the current flight number */
uint8_t
ao_log_scan(void);

/* Start logging to eeprom */
void
ao_log_start(void);

/* Stop logging */
void
ao_log_stop(void);

/* Initialize the logging system */
void
ao_log_init(void);

/* Write out the current flight number to the erase log */
void
ao_log_write_erase(uint8_t pos);

/* Returns true if there are any logs stored in eeprom */
uint8_t
ao_log_present(void);

/* Returns true if there is no more storage space available */
uint8_t
ao_log_full(void);

/*
 * ao_log_big.c
 */

/*
 * The data log is recorded in the eeprom as a sequence
 * of data packets.
 *
 * Each packet starts with a 4-byte header that has the
 * packet type, the packet checksum and the tick count. Then
 * they all contain 2 16 bit values which hold packet-specific
 * data.
 *
 * For each flight, the first packet
 * is FLIGHT packet, indicating the serial number of the
 * device and a unique number marking the number of flights
 * recorded by this device.
 *
 * During flight, data from the accelerometer and barometer
 * are recorded in SENSOR packets, using the raw 16-bit values
 * read from the A/D converter.
 *
 * Also during flight, but at a lower rate, the deployment
 * sensors are recorded in DEPLOY packets. The goal here is to
 * detect failure in the deployment circuits.
 *
 * STATE packets hold state transitions as the flight computer
 * transitions through different stages of the flight.
 */
#define AO_LOG_FLIGHT		'F'
#define AO_LOG_SENSOR		'A'
#define AO_LOG_TEMP_VOLT	'T'
#define AO_LOG_DEPLOY		'D'
#define AO_LOG_STATE		'S'
#define AO_LOG_GPS_TIME		'G'
#define AO_LOG_GPS_LAT		'N'
#define AO_LOG_GPS_LON		'W'
#define AO_LOG_GPS_ALT		'H'
#define AO_LOG_GPS_SAT		'V'
#define AO_LOG_GPS_DATE		'Y'
#define AO_LOG_GPS_POS		'P'

#define AO_LOG_POS_NONE		(~0UL)

struct ao_log_record {
	char			type;				/* 0 */
	uint8_t			csum;				/* 1 */
	uint16_t		tick;				/* 2 */
	union {
		struct {
			int16_t		ground_accel;		/* 4 */
			uint16_t	flight;			/* 6 */
		} flight;
		struct {
			int16_t		accel;			/* 4 */
			int16_t		pres;			/* 6 */
		} sensor;
		struct {
			int16_t		temp;
			int16_t		v_batt;
		} temp_volt;
		struct {
			int16_t		drogue;
			int16_t		main;
		} deploy;
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		struct {
			uint8_t		hour;
			uint8_t		minute;
			uint8_t		second;
			uint8_t		flags;
		} gps_time;
		int32_t		gps_latitude;
		int32_t		gps_longitude;
		struct {
			uint16_t	altitude_low;
			int16_t		altitude_high;
		} gps_altitude;
		struct {
			uint16_t	svid;
			uint8_t		unused;
			uint8_t		c_n;
		} gps_sat;
		struct {
			uint8_t		year;
			uint8_t		month;
			uint8_t		day;
			uint8_t		extra;
		} gps_date;
		struct {
			uint16_t	d0;
			uint16_t	d1;
		} anon;
	} u;
};

struct ao_log_mega {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		ground_accel;		/* 6 */
			int32_t		ground_pres;		/* 8 */
			int16_t		ground_accel_along;	/* 12 */
			int16_t		ground_accel_across;	/* 14 */
			int16_t		ground_accel_through;	/* 16 */
			int16_t		pad_18;			/* 18 */
			int32_t		ground_roll;		/* 20 */
			int32_t		ground_pitch;		/* 24 */
			int32_t		ground_yaw;		/* 28 */
		} flight;					/* 32 */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		ground_accel;		/* 6 */
			int32_t		ground_pres;		/* 8 */
			int16_t		ground_accel_along;	/* 12 */
			int16_t		ground_accel_across;	/* 14 */
			int16_t		ground_accel_through;	/* 16 */
			int16_t		ground_roll;		/* 18 */
			int16_t		ground_pitch;		/* 20 */
			int16_t		ground_yaw;		/* 22 */
		} flight_old;					/* 24 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		/* AO_LOG_SENSOR */
		struct {
			uint32_t	pres;		/* 4 */
			uint32_t	temp;		/* 8 */
			union {
				struct {
					int16_t		accel_along;	/* 12 */
					int16_t		accel_across;	/* 14 */
					int16_t		accel_through;	/* 16 */
					int16_t		gyro_roll;	/* 18 */
					int16_t		gyro_pitch;	/* 20 */
					int16_t		gyro_yaw;	/* 22 */
					int16_t		mag_along;	/* 24 */
					int16_t		mag_across;	/* 26 */
					int16_t		mag_through;	/* 28 */
				};
				struct {
					int16_t		accel_x;	/* 12 */
					int16_t		accel_y;	/* 14 */
					int16_t		accel_z;	/* 16 */
					int16_t		gyro_x;		/* 18 */
					int16_t		gyro_y;		/* 20 */
					int16_t		gyro_z;		/* 22 */
					int16_t		mag_x;		/* 24 */
					int16_t		mag_z;		/* 26 */
					int16_t		mag_y;		/* 28 */
				};
			};
			int16_t		accel;		/* 30 */
		} sensor;	/* 32 */
		/* AO_LOG_TEMP_VOLT */
		struct {
			int16_t		v_batt;		/* 4 */
			int16_t		v_pbatt;	/* 6 */
			int16_t		n_sense;	/* 8 */
			int16_t		sense[10];	/* 10 */
			uint16_t	pyro;		/* 30 */
		} volt;					/* 32 */
		/* AO_LOG_GPS_TIME */
		struct {
			int32_t		latitude;	/* 4 */
			int32_t		longitude;	/* 8 */
			uint16_t	altitude_low;	/* 12 */
			uint8_t		hour;		/* 14 */
			uint8_t		minute;		/* 15 */
			uint8_t		second;		/* 16 */
			uint8_t		flags;		/* 17 */
			uint8_t		year;		/* 18 */
			uint8_t		month;		/* 19 */
			uint8_t		day;		/* 20 */
			uint8_t		course;		/* 21 */
			uint16_t	ground_speed;	/* 22 */
			int16_t		climb_rate;	/* 24 */
			uint8_t		pdop;		/* 26 */
			uint8_t		hdop;		/* 27 */
			uint8_t		vdop;		/* 28 */
			uint8_t		mode;		/* 29 */
			int16_t		altitude_high;	/* 30 */
		} gps;	/* 32 */
		/* AO_LOG_GPS_SAT */
		struct {
			uint16_t	channels;	/* 4 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[12];			/* 6 */
		} gps_sat;				/* 30 */
	} u;
};

#define AO_LOG_MEGA_GPS_ALTITUDE(l)	((int32_t) ((l)->u.gps.altitude_high << 16) | ((l)->u.gps.altitude_low))
#define AO_LOG_MEGA_SET_GPS_ALTITUDE(l,a)	(((l)->u.gps.mode |= AO_GPS_MODE_ALTITUDE_24), \
						 ((l)->u.gps.altitude_high = (a) >> 16), \
						 (l)->u.gps.altitude_low = (a))

struct ao_log_firetwo {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
		} flight;	/* 6 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;	/* 8 */
		/* AO_LOG_SENSOR */
		struct {
			uint16_t	pressure;	/* 4 */
			uint16_t	thrust;		/* 6 */
			uint16_t	thermistor[4];	/* 8 */
		} sensor;	/* 24 */
		uint8_t		align[28];		/* 4 */
	} u;	/* 32 */
};

struct ao_log_telestatic {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
		} flight;	/* 6 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;	/* 8 */
		/* AO_LOG_SENSOR */
		struct {
			uint32_t	pressure;	/* 4 */
			uint32_t	pressure2;	/* 8 */
			uint32_t	thrust;		/* 12 */
			uint32_t	mass;		/* 16 */
			uint16_t	t_low;		/* 20 */
			uint16_t	t_high[4];	/* 22 */
		} sensor;	/* 30 */
		uint8_t		align[28];		/* 4 */
	} u;	/* 32 */
};

struct ao_log_metrum {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
			int16_t		ground_accel;	/* 6 */
			int32_t		ground_pres;	/* 8 */
			uint32_t	ground_temp;	/* 12 */
		} flight;	/* 16 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;	/* 8 */
		/* AO_LOG_SENSOR */
		struct {
			uint32_t	pres;		/* 4 */
			uint32_t	temp;		/* 8 */
			int16_t		accel;		/* 12 */
		} sensor;	/* 14 */
		/* AO_LOG_TEMP_VOLT */
		struct {
			int16_t		v_batt;		/* 4 */
			int16_t		sense_a;	/* 6 */
			int16_t		sense_m;	/* 8 */
		} volt;		/* 10 */
		/* AO_LOG_GPS_POS */
		struct {
			int32_t		latitude;	/* 4 */
			int32_t		longitude;	/* 8 */
			uint16_t	altitude_low;	/* 12 */
			int16_t		altitude_high;	/* 14 */
		} gps;		/* 16 */
		/* AO_LOG_GPS_TIME */
		struct {
			uint8_t		hour;		/* 4 */
			uint8_t		minute;		/* 5 */
			uint8_t		second;		/* 6 */
			uint8_t		flags;		/* 7 */
			uint8_t		year;		/* 8 */
			uint8_t		month;		/* 9 */
			uint8_t		day;		/* 10 */
			uint8_t		pdop;		/* 11 */
		} gps_time;	/* 12 */
		/* AO_LOG_GPS_SAT (up to three packets) */
		struct {
			uint8_t	channels;		/* 4 */
			uint8_t	more;			/* 5 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[4];			/* 6 */
		} gps_sat;				/* 14 */
		uint8_t		raw[12];		/* 4 */
	} u;	/* 16 */
};

struct ao_log_mini {
	char		type;				/* 0 */
	uint8_t		csum;				/* 1 */
	uint16_t	tick;				/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
			uint16_t	r6;
			int32_t		ground_pres;	/* 8 */
		} flight;
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;		/* 4 */
			uint16_t	reason;		/* 6 */
		} state;
		/* AO_LOG_SENSOR */
		struct {
			uint8_t		pres[3];	/* 4 */
			uint8_t		temp[3];	/* 7 */
			int16_t		sense_a;	/* 10 */
			int16_t		sense_m;	/* 12 */
			int16_t		v_batt;		/* 14 */
		} sensor;				/* 16 */
	} u;						/* 16 */
};							/* 16 */

#define ao_log_pack24(dst,value) do {		\
		(dst)[0] = (uint8_t) (value);	\
		(dst)[1] = (uint8_t) ((value) >> 8);	\
		(dst)[2] = (uint8_t) ((value) >> 16);	\
	} while (0)

struct ao_log_gps {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		start_altitude;		/* 6 */
			int32_t		start_latitude;		/* 8 */
			int32_t		start_longitude;	/* 12 */
		} flight;					/* 16 */
		/* AO_LOG_GPS_TIME */
		struct {
			int32_t		latitude;	/* 4 */
			int32_t		longitude;	/* 8 */
			uint16_t	altitude_low;	/* 12 */
			uint8_t		hour;		/* 14 */
			uint8_t		minute;		/* 15 */
			uint8_t		second;		/* 16 */
			uint8_t		flags;		/* 17 */
			uint8_t		year;		/* 18 */
			uint8_t		month;		/* 19 */
			uint8_t		day;		/* 20 */
			uint8_t		course;		/* 21 */
			uint16_t	ground_speed;	/* 22 */
			int16_t		climb_rate;	/* 24 */
			uint8_t		pdop;		/* 26 */
			uint8_t		hdop;		/* 27 */
			uint8_t		vdop;		/* 28 */
			uint8_t		mode;		/* 29 */
			int16_t		altitude_high;	/* 30 */
		} gps;	/* 31 */
		/* AO_LOG_GPS_SAT */
		struct {
			uint16_t	channels;	/* 4 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[12];			/* 6 */
		} gps_sat;				/* 30 */
	} u;
};

struct ao_log_motor {
	char			type;			/* 0 */
	uint8_t			csum;			/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;			/* 4 */
			int16_t		ground_accel;		/* 6 */
			int16_t		ground_accel_along;	/* 8 */
			int16_t		ground_accel_across;	/* 10 */
			int16_t		ground_accel_through;	/* 12 */
			int16_t		ground_motor_pressure;	/* 14 */
		} flight;					/* 16 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;			/* 4 */
			uint16_t	reason;			/* 6 */
		} state;
		/* AO_LOG_SENSOR */
		struct {
			uint16_t	pressure;		/* 4 */
			uint16_t	v_batt;			/* 6 */
			int16_t		accel;			/* 8 */
			int16_t		accel_across;		/* 10 */
			int16_t		accel_along;		/* 12 */
			int16_t		accel_through;		/* 14 */
		} sensor;					/* 16 */
	} u;
};

#if AO_LOG_FORMAT == AO_LOG_FOMAT_TELEMEGA_OLD || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMEGA || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMEGA_3 || AO_LOG_FORMAT == AO_LOG_FORMAT_EASYMEGA_2 || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMEGA_4 || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMEGA_5
typedef struct ao_log_mega ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMETRUM
typedef struct ao_log_metrum ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELEFIRETWO
typedef struct ao_log_firetwo ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_EASYMINI1 || AO_LOG_FORMAT == AO_LOG_FORMAT_EASYMINI2 || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMINI2 || AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMINI3
typedef struct ao_log_mini ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_EASYMOTOR
typedef struct ao_log_motor ao_log_type;
#endif 

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELEGPS
typedef struct ao_log_gps ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_FULL
typedef struct ao_log_record ao_log_type;
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TINY
#define AO_LOG_UNCOMMON	1
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELEMETRY
#define AO_LOG_UNCOMMON	1
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_TELESCIENCE
#define AO_LOG_UNCOMMON	1
#endif

#if AO_LOG_FORMAT == AO_LOG_FORMAT_MICROPEAK2
#define AO_LOG_UNCOMMON	1
#endif

#ifndef AO_LOG_UNCOMMON
extern ao_log_type ao_log_data;

#define AO_LOG_SIZE sizeof(ao_log_type)

/* Write a record to the eeprom log */

uint8_t
ao_log_write(ao_log_type *log);
#endif

void
ao_log_flush(void);

void
ao_gps_report_metrum_init(void);

#endif /* _AO_LOG_H_ */
