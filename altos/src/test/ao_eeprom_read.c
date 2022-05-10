/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include "ao_eeprom_read.h"
#include <json-c/json.h>
#include <string.h>
#include <errno.h>

static struct json_object *
ao_eeprom_read_config(FILE *file)
{
	char			line[1024];
	struct json_tokener	*tok;
	struct json_object	*obj = NULL;
	enum json_tokener_error	err;

	tok = json_tokener_new();
	if (!tok)
		goto fail_tok;

	for (;;) {
		if (fgets(line, sizeof(line), file) == NULL)
			goto fail_read;
		obj = json_tokener_parse_ex(tok, line, strlen(line));
		err = json_tokener_get_error(tok);
		if (err == json_tokener_success)
			break;
		if (err != json_tokener_continue)
			goto fail_read;
	}
	json_tokener_free(tok);
	return obj;
fail_read:
	json_tokener_free(tok);
fail_tok:
	return NULL;
}

static int
ao_eeprom_read_byte(FILE *file)
{
	int	byte;
	if (fscanf(file, "%x", &byte) != 1)
		return EOF;
	return byte;
}

static int
ao_eeprom_read_data(FILE *file, struct ao_eeprom *eeprom)
{
	uint8_t	*data = NULL, *ndata;
	int	len = 0;
	int	size = 0;
	int	byte;

	data = malloc(size = 64);
	if (!data)
		goto fail_alloc;
	while ((byte = ao_eeprom_read_byte(file)) != EOF) {
		if (len == size) {
			ndata = realloc(data, size *= 2);
			if (!ndata)
				goto fail_realloc;
			data = ndata;
		}
		data[len++] = (uint8_t) byte;
	}
	eeprom->data = data;
	eeprom->len = len;
	return 1;
fail_realloc:
	free(data);
fail_alloc:
	return 0;
}

static int
ao_json_get_int(struct json_object *obj, const char *key, int def)
{
	struct json_object *value;
	int i;

	if (!json_object_object_get_ex(obj, key, &value))
		return def;
	errno = 0;
	i = (int) json_object_get_int(value);
	if (errno != 0)
		return def;
	return i;
}

static const char *
ao_json_get_string(struct json_object *obj, const char *key, const char *def)
{
	struct json_object *value;
	const char *str;

	if (!json_object_object_get_ex(obj, key, &value))
		return def;
	errno = 0;
	str = json_object_get_string(value);
	if (errno)
		return def;
	if (!str)
		return def;
	return str;
}

#if AO_PYRO_NUM
static int
ao_eeprom_get_pyro(struct ao_config *config, struct json_object *obj)
{
	struct json_object	*pyros;
	struct json_object	*pyro;
	int			i, p;

	if (!json_object_object_get_ex(obj, "pyros", &pyros))
		return 1;

	if (json_object_get_type(pyros) != json_type_array)
		return 0;

	for (i = 0; i < json_object_array_length(pyros); i++) {
		pyro = json_object_array_get_idx(pyros, i);
		if (pyro) {
			p = ao_json_get_int(pyro, "channel", -1);
			if (0 <= p && p < AO_PYRO_NUM) {
				config->pyro[p].flags		= ao_json_get_int(pyro, "flags", 0);
				config->pyro[p].accel_less	= ao_json_get_int(pyro, "accel_less", 0);
				config->pyro[p].accel_greater	= ao_json_get_int(pyro, "accel_greater", 0);
				config->pyro[p].speed_less	= ao_json_get_int(pyro, "speed_less", 0);
				config->pyro[p].speed_greater	= ao_json_get_int(pyro, "speed_greater", 0);
				config->pyro[p].height_less	= ao_json_get_int(pyro, "height_less", 0);
				config->pyro[p].height_greater	= ao_json_get_int(pyro, "height_greater", 0);
				config->pyro[p].orient_less	= ao_json_get_int(pyro, "orient_less", 0);
				config->pyro[p].orient_greater	= ao_json_get_int(pyro, "orient_greater", 0);
				config->pyro[p].time_less	= ao_json_get_int(pyro, "time_less", 0);
				config->pyro[p].time_greater	= ao_json_get_int(pyro, "time_greater", 0);
				config->pyro[p].delay		= ao_json_get_int(pyro, "delay", 0);
				config->pyro[p].state_less	= ao_json_get_int(pyro, "state_less", 0);
				config->pyro[p].state_greater_or_equal	= ao_json_get_int(pyro, "state_greater_or_equal", 0);
				config->pyro[p].motor		= ao_json_get_int(pyro, "motor", 0);
			}
		}
	}
	return 1;
}
#endif

static int
ao_eeprom_get_ms5607(struct ao_ms5607_prom *ms5607_prom, struct json_object *obj)
{
	struct json_object	*ms5607;

	if (!json_object_object_get_ex(obj, "ms5607", &ms5607))
		return 1;

	if (json_object_get_type(ms5607) != json_type_object)
		return 0;

	ms5607_prom->reserved =	ao_json_get_int(ms5607, "reserved", 0);
	ms5607_prom->sens =	ao_json_get_int(ms5607, "sens", 0);
	ms5607_prom->off =	ao_json_get_int(ms5607, "off", 0);
	ms5607_prom->tcs =	ao_json_get_int(ms5607, "tcs", 0);
	ms5607_prom->tco =	ao_json_get_int(ms5607, "tco", 0);
	ms5607_prom->tref =	ao_json_get_int(ms5607, "tref", 0);
	ms5607_prom->tempsens =	ao_json_get_int(ms5607, "tempsens", 0);
	ms5607_prom->crc =	ao_json_get_int(ms5607, "crc", 0);
	return 1;
}

static int
ao_eeprom_get_config(struct ao_eeprom *ao_eeprom, struct json_object *obj)
{
	struct ao_config	*config = &ao_eeprom->config;
	const char		*s;

	if (json_object_get_type(obj) != json_type_object)
		return 0;

	ao_eeprom->log_format = 	ao_json_get_int(obj, "log_format", 0);
	ao_eeprom->serial_number =	ao_json_get_int(obj, "serial", 0);

	config->major =			ao_json_get_int(obj, "config_major", 0);
	config->minor = 		ao_json_get_int(obj, "config_minor", 0);
	if (config->major == 0 || config->minor == 0)
		return 0;

	config->main_deploy = 		ao_json_get_int(obj, "main_deploy", 250);
	config->accel_plus_g = 		ao_json_get_int(obj, "accel_cal_plus", 0);

	s = ao_json_get_string(obj, "callsign", "N0CALL");
	strncpy(config->callsign, s, sizeof(config->callsign));

	config->apogee_delay =		ao_json_get_int(obj, "apogee_delay", 0);
	config->accel_minus_g =		ao_json_get_int(obj, "accel_cal_minus", 0);
	config->radio_cal =		ao_json_get_int(obj, "radio_calibration", 0);
	config->flight_log_max =	ao_json_get_int(obj, "flight_log_max", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->pad_orientation = 	ao_json_get_int(obj, "pad_orientation", 0);
	config->radio_setting = 	ao_json_get_int(obj, "radio_setting", 0);
	config->radio_enable = 		ao_json_get_int(obj, "radio_enable", 1);
	config->frequency = 		ao_json_get_int(obj, "frequency", 434550);
	config->apogee_lockout = 	ao_json_get_int(obj, "apogee_lockout", 0);
#if AO_PYRO_NUM
	if (!ao_eeprom_get_pyro(config, obj))
		return 0;
#endif
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);
	config->ignite_mode = 		ao_json_get_int(obj, "ignite_mode", 0);

	if (!ao_eeprom_get_ms5607(&ao_eeprom->ms5607_prom, obj))
		return 0;

	return 1;
}

struct ao_eeprom *
ao_eeprom_read(FILE *file)
{
	struct ao_eeprom	*ao_eeprom;
	struct json_object	*obj;
	int			ret;

	ao_eeprom = calloc(1, sizeof (struct ao_eeprom));
	if (!ao_eeprom)
		goto fail_ao_eeprom;

	obj = ao_eeprom_read_config(file);
	if (!obj)
		goto fail_config;

	ret = ao_eeprom_get_config(ao_eeprom, obj);
	json_object_put(obj);
	if (!ret)
		goto fail_config;

	if (!ao_eeprom_read_data(file, ao_eeprom))
		goto fail_data;

	return ao_eeprom;
fail_data:
fail_config:
	free(ao_eeprom);
fail_ao_eeprom:
	return NULL;
}
