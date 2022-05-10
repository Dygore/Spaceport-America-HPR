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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <ao-eeprom-read.h>
#include <ao-atmosphere.h>

static const struct option options[] = {
	{ .name = "raw", .has_arg = 0, .val = 'r' },
	{ .name = "csum", .has_arg = 0, .val = 'c' },
	{ .name = "verbose", .has_arg = 0, .val = 'v' },
	{ .name = "len", .has_arg = 1, .val = 'l' },
	{ 0, 0, 0, 0},
};

static void usage(char *program)
{
	fprintf(stderr, "usage: %s [--raw] [--csum] [--verbose] [--len <record-len>] {flight.eeprom} ...\n", program);
	exit(1);
}

static bool
ao_csum_valid(uint8_t *d, int len)
{
	uint8_t sum = 0x5a;
	int i;
	for (i = 0; i < len; i++)
		sum += d[i];
	return sum == 0;
}

static void
ao_ms5607(uint32_t pres, uint32_t temp, struct ao_eeprom *eeprom, bool is_ms5611)
{
	struct ao_ms5607_sample ms5607_sample = { .pres = pres, .temp = temp };
	struct ao_ms5607_value ms5607_value;

	ao_ms5607_convert(&ms5607_sample, &ms5607_value,
			  &eeprom->ms5607_prom, is_ms5611);
	printf(" pres %9u %7.3f kPa  %7.1f m temp %9u %6.2f °C",
	       pres,
	       ms5607_value.pres / 1000.0,
	       ao_pressure_to_altitude(ms5607_value.pres),
	       temp,
	       ms5607_value.temp / 100.0);
}

#define GRAVITY 9.80665

static void
ao_accel(int16_t accel, struct ao_eeprom *eeprom)
{
	double accel_2g = eeprom->config.accel_minus_g - eeprom->config.accel_plus_g;
	double accel_scale = GRAVITY * 2.0 / accel_2g;
	printf(" accel %6d %7.2f m/s²",
	       accel, (eeprom->config.accel_plus_g - accel) * accel_scale);
}

static const char *state_names[] = {
	"startup",
	"idle",
	"pad",
	"boost",
	"fast",
	"coast",
	"drogue",
	"main",
	"landed",
	"invalid"
};

#define NUM_STATE	(sizeof state_names/sizeof state_names[0])

static const char *
ao_state_name(uint16_t state)
{
	if (state < NUM_STATE)
		return state_names[state];
	return "UNKNOWN";
}

static void
ao_state(uint16_t state, uint16_t reason)
{
	printf(" state %5u %s reason %5u",
	       state, ao_state_name(state), reason);
}

static double
ao_adc_to_volts(int16_t value, int16_t max_adc, double ref, double r1, double r2)
{
	return ref * ((double) value / max_adc) * (r1 + r2) / r2;
}

static void
ao_volts(const char *name, int16_t value, int16_t max_adc, double ref, double r1, double r2)
{
	printf(" %s %5d",
	       name, value);
	if (r1 && r2 && ref)
		printf(" %6.3f V", ao_adc_to_volts(value, max_adc, ref, r1, r2));
}

static double lb_to_n(double lb)
{
	return lb / 0.22480894;
}

static double psi_to_pa(double psi)
{
	return psi * 6894.76;
}

static double
ao_volts_to_newtons(double volts)
{
	/* this is a total guess */
	return lb_to_n(volts * 57.88645 * GRAVITY);
}

static void
ao_thrust(int16_t value, int16_t max_adc, double ref, double r1, double r2)
{
	printf(" thrust %5d", value);
	if (r1 && r2 && ref) {
		double volts = ao_adc_to_volts(value, max_adc, ref, r1, r2);
		printf(" %6.3f V %8.1f N", volts, ao_volts_to_newtons(volts));
	}
}

static void
ao_pressure(int16_t value, int16_t max_adc, double ref, double r1, double r2)
{
	printf(" pressure %5d", value);
	if (r1 && r2 && ref) {
		double volts = ao_adc_to_volts(value, max_adc, ref, r1, r2);
		if (volts < 0.5) volts = 0.5;
		if (volts > 4.5) volts = 4.5;

		double psi = (volts - 0.5) / 4.0 * 2500.0;
		double pa = psi_to_pa(psi);
		printf(" %9.3f kPa", pa / 1000.0);
	}
}

#if 0
static uint16_t
uint16(uint8_t *bytes, int off)
{
	return (uint16_t) bytes[off] | (((uint16_t) bytes[off+1]) << 8);
}

static int16_t
int16(uint8_t *bytes, int off)
{
	return (int16_t) uint16(bytes, off);
}

static uint32_t
uint32(uint8_t *bytes, int off)
{
	return (uint32_t) bytes[off] | (((uint32_t) bytes[off+1]) << 8) |
		(((uint32_t) bytes[off+2]) << 16) |
		(((uint32_t) bytes[off+3]) << 24);
}

static int32_t
int32(uint8_t *bytes, int off)
{
	return (int32_t) uint32(bytes, off);
}
#endif

static uint32_t
uint24(uint8_t *bytes, int off)
{
	return (uint32_t) bytes[off] | (((uint32_t) bytes[off+1]) << 8) |
		(((uint32_t) bytes[off+2]) << 16);
}

static int32_t
int24(uint8_t *bytes, int off)
{
	return (int32_t) uint24(bytes, off);
}

int
main (int argc, char **argv)
{
	struct ao_eeprom	*eeprom;
	FILE			*file;
	int			c;
	bool			raw = false;
	bool			csum = false;
	bool			verbose = false;
	int			arg_len = 0;
	char			*end;
	int			ret = 0;
	int			i;

	while ((c = getopt_long(argc, argv, "rcvl:", options, NULL)) != -1) {
		switch (c) {
		case 'r':
			raw = true;
			break;
		case 'c':
			csum = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'l':
			arg_len = strtol(optarg, &end, 0);
			if (!*optarg || *end)
				usage(argv[0]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	for (i = optind; i < argc; i++) {
		file = fopen(argv[i], "r");
		if (!file) {
			perror(argv[i]);
			ret++;
			continue;
		}
		eeprom = ao_eeprom_read(file);
		fclose(file);
		if (!eeprom) {
			perror(argv[i]);
			ret++;
			continue;
		}
		int 	len = 0;
		bool	is_ms5611 = false;

		int64_t	current_tick = 0;
		int64_t	first_tick = 0x7fffffffffffffffLL;

		double	sense_r1 = 0.0, sense_r2 = 0.0;
		double	batt_r1 = 0.0, batt_r2 = 0.0;
		double	adc_ref = 0.0;
		int16_t	max_adc = 0;

		switch (eeprom->log_format) {
		case AO_LOG_FORMAT_TELEMEGA_OLD:
			len = 32;
			break;
		case AO_LOG_FORMAT_EASYMINI1:
			len = 16;
			max_adc = 32767;
			if (eeprom->serial_number < 1000)
				adc_ref = 3.0;
			else
				adc_ref = 3.3;
			batt_r1 = sense_r1 = 100e3;
			batt_r2 = sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_TELEMETRUM:
			len = 16;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_TELEMINI2:
			len = 16;
			break;
		case AO_LOG_FORMAT_TELEGPS:
			len = 32;
			break;
		case AO_LOG_FORMAT_TELEMEGA:
			len = 32;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_DETHERM:
			len = 16;
			break;
		case AO_LOG_FORMAT_TELEMINI3:
			len = 16;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_TELEFIRETWO:
			len = 32;
			max_adc = 4095;
			adc_ref = 3.3;
			sense_r1 = batt_r1 = 5600;
			sense_r2 = batt_r2 = 10000;
			break;
		case AO_LOG_FORMAT_EASYMINI2:
			len = 16;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = sense_r1 = 100e3;
			batt_r2 = sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_TELEMEGA_3:
			len = 32;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_EASYMEGA_2:
			len = 32;
			max_adc = 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		case AO_LOG_FORMAT_TELESTATIC:
			len = 32;
			break;
		case AO_LOG_FORMAT_MICROPEAK2:
			len = 2;
			break;
		case AO_LOG_FORMAT_TELEMEGA_4:
			len = 32;
			break;
			max_adc= 4095;
			adc_ref = 3.3;
			batt_r1 = 5600;
			batt_r2 = 10000;
			sense_r1 = 100e3;
			sense_r2 = 27e3;
			break;
		}
		if (arg_len)
			len = arg_len;
		if (verbose)
			printf("config major %d minor %d log format %d total %u len %d\n",
			       eeprom->config.major,
			       eeprom->config.minor,
			       eeprom->log_format,
			       eeprom->len,
			       len);

		uint32_t	pos;
		for (pos = 0; pos < eeprom->len; pos += len) {
			int i;
			if (raw) {
				printf("%9u", pos);
				for (i = 0; i < len; i++)
					printf(" %02x", eeprom->data[pos + i]);
			} else {
				struct ao_log_mega *log_mega;
				struct ao_log_mini *log_mini;
				struct ao_log_metrum *log_metrum;
				struct ao_log_gps *log_gps;
				struct ao_log_firetwo *log_firetwo;

				if (!csum && !ao_csum_valid(&eeprom->data[pos], len)) {
					if (verbose)
						printf("\tchecksum error at %d\n", pos);
					continue;
				}

				struct ao_log_header *log_header = (struct ao_log_header *) &eeprom->data[pos];

				if (first_tick == 0x7fffffffffffffffLL) {
					current_tick = first_tick = log_header->tick;
				} else {
					int16_t diff = (int16_t) (log_header->tick - (uint16_t) current_tick);

					current_tick += diff;
				}
				printf("type %c tick %5u %6.2f S", log_header->type, log_header->tick, (current_tick - first_tick) / 100.0);

				switch (eeprom->log_format) {
				case AO_LOG_FORMAT_TELEMEGA_OLD:
				case AO_LOG_FORMAT_TELEMEGA:
				case AO_LOG_FORMAT_TELEMEGA_3:
				case AO_LOG_FORMAT_EASYMEGA_2:
				case AO_LOG_FORMAT_TELEMEGA_4:
					log_mega = (struct ao_log_mega *) &eeprom->data[pos];
					switch (log_mega->type) {
					case AO_LOG_FLIGHT:
						printf(" serial %5u flight %5u ground_accel %6d ground_pres %9u",
						       eeprom->serial_number,
						       log_mega->u.flight.flight,
						       log_mega->u.flight.ground_accel,
						       log_mega->u.flight.ground_pres);
						printf(" along %6d aross %6d through %6d",
						       log_mega->u.flight.ground_accel_along,
						       log_mega->u.flight.ground_accel_across,
						       log_mega->u.flight.ground_accel_through);
						printf(" roll %6d pitch %6d yaw %6d",
						       log_mega->u.flight.ground_roll,
						       log_mega->u.flight.ground_pitch,
						       log_mega->u.flight.ground_yaw);
						break;
					case AO_LOG_STATE:
						ao_state(log_mega->u.state.state,
							 log_mega->u.state.reason);
						break;
					case AO_LOG_SENSOR:
						ao_ms5607(log_mega->u.sensor.pres,
							  log_mega->u.sensor.temp,
							  eeprom, is_ms5611);
						printf(" accel_x %6d accel_y %6d accel_z %6d",
						       log_mega->u.sensor.accel_x,
						       log_mega->u.sensor.accel_y,
						       log_mega->u.sensor.accel_z);
						printf (" gyro_x %6d gyro_y %6d gyro_z %6d",
						       log_mega->u.sensor.gyro_x,
						       log_mega->u.sensor.gyro_y,
						       log_mega->u.sensor.gyro_z);
						printf (" mag_x %6d mag_y %6d mag_z %6d",
						       log_mega->u.sensor.mag_x,
						       log_mega->u.sensor.mag_y,
						       log_mega->u.sensor.mag_z);
						ao_accel(log_mega->u.sensor.accel, eeprom);
						break;
					case AO_LOG_TEMP_VOLT:
						ao_volts("v_batt",
							 log_mega->u.volt.v_batt,
							 max_adc,
							 adc_ref,
							 batt_r1, batt_r2);
						ao_volts("v_pbatt",
							 log_mega->u.volt.v_pbatt,
							 max_adc,
							 adc_ref,
							 sense_r1, sense_r2);
						printf(" n_sense %1d",
						       log_mega->u.volt.n_sense);
						for (i = 0; i < log_mega->u.volt.n_sense; i++) {
							char name[10];
							sprintf(name, "sense%d", i);
							ao_volts(name,
								 log_mega->u.volt.sense[i],
								 max_adc,
								 adc_ref,
								 sense_r1, sense_r2);
						}
						printf(" pyro %04x", log_mega->u.volt.pyro);
						break;
					case AO_LOG_GPS_TIME:
						printf(" lat %10.7f ° lon %10.7f ° alt %8d m",
						       log_mega->u.gps.latitude / 10000000.0,
						       log_mega->u.gps.longitude/ 10000000.0,
						       (int32_t) (log_mega->u.gps.altitude_low |
								  (log_mega->u.gps.altitude_high << 16)));
						printf(" time %02d:%02d:%02d %04d-%02d-%02d flags %02x",
						       log_mega->u.gps.hour,
						       log_mega->u.gps.minute,
						       log_mega->u.gps.second,
						       log_mega->u.gps.year + 2000,
						       log_mega->u.gps.month,
						       log_mega->u.gps.day,
						       log_mega->u.gps.flags);
						printf(" course %3d ground_speed %5u climb_rate %6d pdop %3d hdop %3d vdop %3d mode %3d",
						       log_mega->u.gps.course,
						       log_mega->u.gps.ground_speed,
						       log_mega->u.gps.climb_rate,
						       log_mega->u.gps.pdop,
						       log_mega->u.gps.hdop,
						       log_mega->u.gps.vdop,
						       log_mega->u.gps.mode);
						break;
					case AO_LOG_GPS_SAT:
						printf(" channels %2d",
						       log_mega->u.gps_sat.channels);
						for (i = 0; i < 12; i++) {
							printf(" svid %3d c_n %2d",
							       log_mega->u.gps_sat.sats[i].svid,
							       log_mega->u.gps_sat.sats[i].c_n);
						}
						break;
					}
					break;
				case AO_LOG_FORMAT_EASYMINI1:
				case AO_LOG_FORMAT_EASYMINI2:
				case AO_LOG_FORMAT_TELEMINI2:
				case AO_LOG_FORMAT_TELEMINI3:
					log_mini = (struct ao_log_mini *) &eeprom->data[pos];
					switch (log_mini->type) {
					case AO_LOG_FLIGHT:
						printf(" serial %5u flight %5u ground_pres %9u",
						       eeprom->serial_number,
						       log_mini->u.flight.flight,
						       log_mini->u.flight.ground_pres);
						break;
					case AO_LOG_STATE:
						ao_state(log_mini->u.state.state,
							 log_mini->u.state.reason);
						break;
					case AO_LOG_SENSOR:
						ao_ms5607(int24(log_mini->u.sensor.pres, 0),
							  int24(log_mini->u.sensor.temp, 0),
							  eeprom, is_ms5611);
						ao_volts("sense_a",
							 log_mini->u.sensor.sense_a, max_adc,
							 adc_ref, sense_r1, sense_r2);
						ao_volts("sense_m",
							 log_mini->u.sensor.sense_m, max_adc,
							 adc_ref, sense_r1, sense_r2);
						ao_volts("v_batt",
							 log_mini->u.sensor.v_batt, max_adc,
							 adc_ref, batt_r1, batt_r2);
						break;
					} /*  */
					break;
				case AO_LOG_FORMAT_TELEMETRUM:
					log_metrum = (struct ao_log_metrum *) &eeprom->data[pos];
					switch (log_metrum->type) {
					case AO_LOG_FLIGHT:
						printf(" serial %5u flight %5u ground_accel %6d ground_pres %9u ground_temp %9u",
						       eeprom->serial_number,
						       log_metrum->u.flight.flight,
						       log_metrum->u.flight.ground_accel,
						       log_metrum->u.flight.ground_pres,
						       log_metrum->u.flight.ground_temp);
						break;
					case AO_LOG_SENSOR:
						ao_ms5607(log_metrum->u.sensor.pres,
							  log_metrum->u.sensor.temp,
							  eeprom, is_ms5611);
						ao_accel(log_metrum->u.sensor.accel, eeprom);
						break;
					case AO_LOG_TEMP_VOLT:
						ao_volts("v_batt",
							 log_metrum->u.volt.v_batt, max_adc,
							 adc_ref, batt_r1, batt_r2);
						ao_volts("sense_a",
							 log_metrum->u.volt.sense_a, max_adc,
							 adc_ref, sense_r1, sense_r2);
						ao_volts("sense_m",
							 log_metrum->u.volt.sense_m, max_adc,
							 adc_ref, sense_r1, sense_r2);
						break;
					case AO_LOG_DEPLOY:
						break;
					case AO_LOG_STATE:
						ao_state(log_metrum->u.state.state,
							 log_metrum->u.state.reason);
						break;
					case AO_LOG_GPS_TIME:
						printf(" time %02d:%02d:%02d 20%02d-%02d-%02d flags %02x pdop %3u",
						       log_metrum->u.gps_time.hour,
						       log_metrum->u.gps_time.minute,
						       log_metrum->u.gps_time.second,
						       log_metrum->u.gps_time.year,
						       log_metrum->u.gps_time.month,
						       log_metrum->u.gps_time.day,
						       log_metrum->u.gps_time.flags,
						       log_metrum->u.gps_time.pdop);
						break;
					case AO_LOG_GPS_SAT:
						printf(" channels %2d more %1d",
						       log_metrum->u.gps_sat.channels,
						       log_metrum->u.gps_sat.more);
						for (i = 0; i < 4; i++) {
							printf(" svid %3d c_n %2d",
							       log_metrum->u.gps_sat.sats[i].svid,
							       log_metrum->u.gps_sat.sats[i].c_n);
						}
						break;
					case AO_LOG_GPS_POS:
						printf(" lat %10.7f° lon %10.7f° alt %8d m",
						       log_metrum->u.gps.latitude / 10000000.0,
						       log_metrum->u.gps.longitude/ 10000000.0,
						       (int32_t) (log_metrum->u.gps.altitude_low |
								  (log_metrum->u.gps.altitude_high << 16)));
						break;
					default:
						printf(" unknown");
					}
					break;
				case AO_LOG_FORMAT_TELEFIRETWO:
					log_firetwo = (struct ao_log_firetwo *) &eeprom->data[pos];
					switch (log_firetwo->type) {
					case AO_LOG_FLIGHT:
						printf(" serial %5u flight %5u",
						       eeprom->serial_number,
						       log_firetwo->u.flight.flight);
						break;
					case AO_LOG_STATE:
						ao_state(log_firetwo->u.state.state,
							 log_firetwo->u.state.reason);
						break;
					case AO_LOG_SENSOR:
						ao_pressure(log_firetwo->u.sensor.pressure,
							    max_adc, adc_ref,
							    sense_r1, sense_r2);
						ao_thrust(log_firetwo->u.sensor.thrust,
							  max_adc, adc_ref,
							  sense_r1, sense_r2);
						for (i = 0; i < 4; i++) {
							char name[20];
							sprintf(name, "thermistor%d", i);
							ao_volts(name,
								 log_firetwo->u.sensor.thermistor[i],
								 max_adc, adc_ref,
								 sense_r1, sense_r2);
						}
						break;
					}
					break;
				case AO_LOG_FORMAT_TELEGPS:
					log_gps = (struct ao_log_gps *) &eeprom->data[pos];
					switch (log_gps->type) {
					case AO_LOG_GPS_TIME:
						printf(" lat %10.7f ° lon %10.7f ° alt %8d m",
						       log_gps->u.gps.latitude / 10000000.0,
						       log_gps->u.gps.longitude/ 10000000.0,
						       (int32_t) (log_gps->u.gps.altitude_low |
								  (log_gps->u.gps.altitude_high << 16)));
						printf(" time %02d:%02d:%02d %04d-%02d-%02d flags %02x",
						       log_gps->u.gps.hour,
						       log_gps->u.gps.minute,
						       log_gps->u.gps.second,
						       log_gps->u.gps.year + 2000,
						       log_gps->u.gps.month,
						       log_gps->u.gps.day,
						       log_gps->u.gps.flags);
						printf(" course %3d ground_speed %5u climb_rate %6d pdop %3d hdop %3d vdop %3d mode %3d",
						       log_gps->u.gps.course,
						       log_gps->u.gps.ground_speed,
						       log_gps->u.gps.climb_rate,
						       log_gps->u.gps.pdop,
						       log_gps->u.gps.hdop,
						       log_gps->u.gps.vdop,
						       log_gps->u.gps.mode);
						break;
					case AO_LOG_GPS_SAT:
						printf(" channels %2d",
						       log_gps->u.gps_sat.channels);
						for (i = 0; i < 12; i++) {
							printf(" svid %3d c_n %2d",
							       log_gps->u.gps_sat.sats[i].svid,
							       log_gps->u.gps_sat.sats[i].c_n);
						}
						break;
					default:
						printf (" unknown");
						break;
					}
					break;
				case AO_LOG_FORMAT_DETHERM:
					break;
				}
			}
			printf("\n");
		}
	}
	return ret;
}
