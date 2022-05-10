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

#ifndef _CC_H_
#define _CC_H_

#include <stdio.h>
#include <stdint.h>
#include "cc-telemetry.h"

char *
cc_fullname (char *dir, char *file);

char *
cc_basename(char *file);

int
cc_mkdir(char *dir);

struct cc_usbdev {
	char	*sys;
	char	*tty;
	char	*manufacturer;
	char	*product;
	int	serial;	/* AltOS always uses simple integer serial numbers */
	int	idProduct;
	int	idVendor;
};

struct cc_usbdevs {
	struct cc_usbdev	**dev;
	int			ndev;
};

void
cc_usbdevs_free(struct cc_usbdevs *usbdevs);

struct cc_usbdevs *
cc_usbdevs_scan(int non_tty);

char *
cc_usbdevs_find_by_arg(char *arg, char *default_product);

void
cc_set_log_dir(char *dir);

char *
cc_get_log_dir(void);

char *
cc_make_filename(int serial, int flight, char *ext);

/*
 * For sequential data which are not evenly spaced
 */

struct cc_timedataelt {
	double	time;
	double	value;
};

struct cc_timedata {
	int			num;
	int			size;
	struct cc_timedataelt	*data;
	double			time_offset;
};


/*
 * For GPS data
 */

struct cc_gpselt {
	double		time;
	int		hour;
	int		minute;
	int		second;
	int		flags;
	double		lat;
	double		lon;
	double		alt;
};

#define SIRF_SAT_STATE_ACQUIRED			(1 << 0)
#define SIRF_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define SIRF_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define SIRF_SAT_SUBFRAME_SYNC_COMPLETE		(1 << 3)
#define SIRF_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define SIRF_SAT_CODE_LOCKED			(1 << 5)
#define SIRF_SAT_ACQUISITION_FAILED		(1 << 6)
#define SIRF_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct cc_gpssat {
	double		time;
	uint16_t	svid;
	uint8_t		c_n;
};

struct cc_gpssats {
	int			nsat;
	struct cc_gpssat	sat[12];
};

struct cc_gpsdata {
	int			num;
	int			size;
	struct cc_gpselt	*data;
	double			time_offset;
	int			numsats;
	int			sizesats;
	struct cc_gpssats	*sats;
};

/*
 * For sequential data which are evenly spaced
 */
struct cc_perioddata {
	int		num;
	double		start;
	double		step;
	double		*data;
};

enum ao_flight_state {
	ao_flight_startup = 0,
	ao_flight_idle = 1,
	ao_flight_pad = 2,
	ao_flight_boost = 3,
	ao_flight_fast = 4,
	ao_flight_coast = 5,
	ao_flight_drogue = 6,
	ao_flight_main = 7,
	ao_flight_landed = 8,
	ao_flight_invalid = 9
};

struct cc_flightraw {
	int			flight;
	int			serial;
	double			ground_accel;
	double			ground_pres;
	int			year, month, day;
	struct cc_timedata	accel;
	struct cc_timedata	pres;
	struct cc_timedata	temp;
	struct cc_timedata	volt;
	struct cc_timedata	main;
	struct cc_timedata	drogue;
	struct cc_timedata	state;
	struct cc_gpsdata	gps;
};

struct cc_flightraw *
cc_log_read(FILE *file);

void
cc_flightraw_free(struct cc_flightraw *raw);

struct cc_flightcooked {
	double			flight_start;
	double			flight_stop;

	struct cc_perioddata	accel_accel;
	struct cc_perioddata	accel_speed;
	struct cc_perioddata	accel_pos;
	struct cc_perioddata	pres_pos;
	struct cc_perioddata	pres_speed;
	struct cc_perioddata	pres_accel;
	struct cc_perioddata	gps_lat;
	struct cc_perioddata	gps_lon;
	struct cc_perioddata	gps_alt;

	/* unfiltered, but converted */
	struct cc_timedata	pres;
	struct cc_timedata	accel;
	struct cc_timedata	state;
};

/*
 * Telemetry data contents
 */


struct cc_gps_time {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

struct cc_gps {
	int	nsat;
	int	gps_locked;
	int	gps_connected;
	struct cc_gps_time gps_time;
	double	lat;		/* degrees (+N -S) */
	double	lon;		/* degrees (+E -W) */
	int	alt;		/* m */

	int	gps_extended;	/* has extra data */
	double	ground_speed;	/* m/s */
	int	course;		/* degrees */
	double	climb_rate;	/* m/s */
	double	hdop;		/* unitless? */
	int	h_error;	/* m */
	int	v_error;	/* m */
};

#define SIRF_SAT_STATE_ACQUIRED			(1 << 0)
#define SIRF_SAT_STATE_CARRIER_PHASE_VALID	(1 << 1)
#define SIRF_SAT_BIT_SYNC_COMPLETE		(1 << 2)
#define SIRF_SAT_SUBFRAME_SYNC_COMPLETE		(1 << 3)
#define SIRF_SAT_CARRIER_PULLIN_COMPLETE	(1 << 4)
#define SIRF_SAT_CODE_LOCKED			(1 << 5)
#define SIRF_SAT_ACQUISITION_FAILED		(1 << 6)
#define SIRF_SAT_EPHEMERIS_AVAILABLE		(1 << 7)

struct cc_gps_sat {
	int	svid;
	int	c_n0;
};

struct cc_gps_tracking {
	int			channels;
	struct cc_gps_sat	sats[12];
};

struct cc_telem {
	char	callsign[16];
	int	serial;
	int	flight;
	int	rssi;
	char	state[16];
	int	tick;
	int	accel;
	int	pres;
	int	temp;
	int	batt;
	int	drogue;
	int	main;
	int	flight_accel;
	int	ground_accel;
	int	flight_vel;
	int	flight_pres;
	int	ground_pres;
	int	accel_plus_g;
	int	accel_minus_g;
	struct cc_gps	gps;
	struct cc_gps_tracking	gps_tracking;
};

int
cc_telem_parse(const char *input_line, struct cc_telem *telem);

struct ao_log_mega {
	char			type;			/* 0 */
	uint8_t			is_config;		/* 1 */
	uint16_t		tick;			/* 2 */
	union {						/* 4 */
		/* AO_LOG_FLIGHT */
		struct {
			uint16_t	flight;		/* 4 */
			int16_t		ground_accel;	/* 6 */
			uint32_t	ground_pres;	/* 8 */
		} flight;				/* 12 */
		/* AO_LOG_STATE */
		struct {
			uint16_t	state;
			uint16_t	reason;
		} state;
		/* AO_LOG_SENSOR */
		struct {
			uint32_t	pres;		/* 4 */
			uint32_t	temp;		/* 8 */
			int16_t		accel_x;	/* 12 */
			int16_t		accel_y;	/* 14 */
			int16_t		accel_z;	/* 16 */
			int16_t		gyro_x;		/* 18 */
			int16_t		gyro_y;		/* 20 */
			int16_t		gyro_z;		/* 22 */
			int16_t		mag_x;		/* 24 */
			int16_t		mag_y;		/* 26 */
			int16_t		mag_z;		/* 28 */
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
			int16_t		altitude;	/* 12 */
			uint8_t		hour;		/* 14 */
			uint8_t		minute;		/* 15 */
			uint8_t		second;		/* 16 */
			uint8_t		flags;		/* 17 */
			uint8_t		year;		/* 18 */
			uint8_t		month;		/* 19 */
			uint8_t		day;		/* 20 */
			uint8_t		pad;		/* 21 */
		} gps;	/* 22 */
		/* AO_LOG_GPS_SAT */
		struct {
			uint16_t	channels;	/* 4 */
			struct {
				uint8_t	svid;
				uint8_t c_n;
			} sats[12];			/* 6 */
		} gps_sat;				/* 30 */

		struct {
			uint32_t		kind;
			int32_t			data[6];
		} config_int;

		struct {
			uint32_t		kind;
			char			string[24];
		} config_str;

		/* Raw bytes */
		uint8_t	bytes[28];
	} u;
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
			uint32_t	ground_pres;	/* 8 */
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
			uint32_t	ground_pres;	/* 8 */
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
		(dst)[0] = (value);		\
		(dst)[1] = (value) >> 8;	\
		(dst)[2] = (value) >> 16;	\
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

#define AO_CONFIG_CONFIG		1
#define AO_CONFIG_MAIN			2
#define AO_CONFIG_APOGEE		3
#define AO_CONFIG_LOCKOUT		4
#define AO_CONFIG_FREQUENCY		5
#define AO_CONFIG_RADIO_ENABLE		6
#define AO_CONFIG_ACCEL_CAL		7
#define AO_CONFIG_RADIO_CAL		8
#define AO_CONFIG_MAX_LOG		9
#define AO_CONFIG_IGNITE_MODE		10
#define AO_CONFIG_PAD_ORIENTATION	11
#define AO_CONFIG_SERIAL_NUMBER		12
#define AO_CONFIG_LOG_FORMAT		13
#define AO_CONFIG_MS5607_RESERVED	14
#define AO_CONFIG_MS5607_SENS		15
#define AO_CONFIG_MS5607_OFF		16
#define AO_CONFIG_MS5607_TCS		17
#define AO_CONFIG_MS5607_TCO		18
#define AO_CONFIG_MS5607_TREF		19
#define AO_CONFIG_MS5607_TEMPSENS	20
#define AO_CONFIG_MS5607_CRC		21


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

#define AO_LOG_CONFIG		'c'

#define AO_GPS_NUM_SAT_MASK	(0xf << 0)
#define AO_GPS_NUM_SAT_SHIFT	(0)

#define AO_GPS_VALID		(1 << 4)
#define AO_GPS_RUNNING		(1 << 5)
#define AO_GPS_DATE_VALID	(1 << 6)
#define AO_GPS_COURSE_VALID	(1 << 7)

#define AO_LOG_FORMAT_UNKNOWN		0	/* unknown; altosui will have to guess */
#define AO_LOG_FORMAT_FULL		1	/* 8 byte typed log records */
#define AO_LOG_FORMAT_TINY		2	/* two byte state/baro records */
#define AO_LOG_FORMAT_TELEMETRY		3	/* 32 byte ao_telemetry records */
#define AO_LOG_FORMAT_TELESCIENCE	4	/* 32 byte typed telescience records */
#define AO_LOG_FORMAT_TELEMEGA		5	/* 32 byte typed telemega records */
#define AO_LOG_FORMAT_EASYMINI		6	/* 16-byte MS5607 baro only, 3.0V supply */
#define AO_LOG_FORMAT_TELEMETRUM	7	/* 16-byte typed telemetrum records */
#define AO_LOG_FORMAT_TELEMINI		8	/* 16-byte MS5607 baro only, 3.3V supply */
#define AO_LOG_FORMAT_TELEGPS		9	/* 32 byte telegps records */
#define AO_LOG_FORMAT_NONE		127	/* No log at all */

int
cc_mega_parse(const char *input_line, struct ao_log_mega *l);

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* Conversion functions */
double
cc_pressure_to_altitude(double pressure);

double
cc_altitude_to_pressure(double altitude);

double
cc_altitude_to_temperature(double altitude);

double
cc_barometer_to_pressure(double baro);

double
cc_barometer_to_altitude(double baro);

double
cc_accelerometer_to_acceleration(double accel, double ground_accel);

double
cc_thermometer_to_temperature(double thermo);

double
cc_battery_to_voltage(double battery);

double
cc_ignitor_to_voltage(double ignite);

void
cc_great_circle (double start_lat, double start_lon,
		 double end_lat, double end_lon,
		 double *dist, double *bearing);

void
cc_timedata_limits(struct cc_timedata *d, double min_time, double max_time, int *start, int *stop);

int
cc_timedata_min(struct cc_timedata *d, double min_time, double max_time);

int
cc_timedata_min_mag(struct cc_timedata *d, double min_time, double max_time);

int
cc_timedata_max(struct cc_timedata *d, double min_time, double max_time);

int
cc_timedata_max_mag(struct cc_timedata *d, double min_time, double max_time);

double
cc_timedata_average(struct cc_timedata *d, double min_time, double max_time);

double
cc_timedata_average_mag(struct cc_timedata *d, double min_time, double max_time);

int
cc_perioddata_limits(struct cc_perioddata *d, double min_time, double max_time, int *start, int *stop);

int
cc_perioddata_min(struct cc_perioddata *d, double min_time, double max_time);

int
cc_perioddata_min_mag(struct cc_perioddata *d, double min_time, double max_time);

int
cc_perioddata_max(struct cc_perioddata *d, double min_time, double max_time);

int
cc_perioddata_max_mag(struct cc_perioddata *d, double min_time, double max_time);

double
cc_perioddata_average(struct cc_perioddata *d, double min_time, double max_time);

double
cc_perioddata_average_mag(struct cc_perioddata *d, double min_time, double max_time);

double *
cc_low_pass(double *data, int data_len, double omega_pass, double omega_stop, double error);

struct cc_perioddata *
cc_period_make(struct cc_timedata *td, double start_time, double stop_time);

struct cc_perioddata *
cc_period_low_pass(struct cc_perioddata *raw, double omega_pass, double omega_stop, double error);

struct cc_timedata *
cc_timedata_convert(struct cc_timedata *d, double (*f)(double v, double a), double a);

struct cc_timedata *
cc_timedata_integrate(struct cc_timedata *d, double min_time, double max_time);

struct cc_perioddata *
cc_perioddata_differentiate(struct cc_perioddata *i);

struct cc_flightcooked *
cc_flight_cook(struct cc_flightraw *raw);

void
cc_flightcooked_free(struct cc_flightcooked *cooked);

#endif /* _CC_H_ */
