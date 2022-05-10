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

#ifndef _AO_EEPROM_READ_H_
#define _AO_EEPROM_READ_H_

#define const

#include <stdint.h>
#include <stdio.h>
#include <ao_telemetry.h>
#include <ao_config.h>
#include <ao_ms5607.h>
#include <ao_log.h>

struct ao_eeprom {
	struct ao_config	config;
	struct ao_ms5607_prom	ms5607_prom;
	int			log_format;
	uint16_t		serial_number;
	uint8_t			*data;
	uint32_t		len;
	uint32_t		size;
};

struct ao_eeprom *ao_eeprom_read(FILE *file);

struct ao_eeprom *ao_eeprom_read_old(FILE *file);

void ao_eeprom_free_data(struct ao_eeprom *ao_eeprom);

#endif /* _AO_EEPROM_READ_H_ */
