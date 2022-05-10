/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

#include "ao_eeprom_read.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int
ao_eeprom_add_u8(struct ao_eeprom *ao_eeprom, uint8_t byte)
{
	if (ao_eeprom->len == ao_eeprom->size) {
		uint32_t	nsize = ao_eeprom->size * 2;
		uint8_t		*ndata = realloc(ao_eeprom->data, nsize);
		if (!ndata)
			return 0;
		ao_eeprom->data = ndata;
		ao_eeprom->size = nsize;
	}
	ao_eeprom->data[ao_eeprom->len++] = byte;
	return 1;
}

static int
ao_eeprom_add_u16(struct ao_eeprom *ao_eeprom, uint16_t u16)
{
	if (!ao_eeprom_add_u8(ao_eeprom, u16 & 0xff))
		return 0;

	return ao_eeprom_add_u8(ao_eeprom, u16 >> 8);
}

struct ao_eeprom *
ao_eeprom_read_old(FILE *file)
{
	struct ao_eeprom	*ao_eeprom;
	char			line[1024];
	char			*l;

	ao_eeprom = calloc(1, sizeof (struct ao_eeprom));
	if (!ao_eeprom)
		return NULL;

	ao_eeprom->log_format = -1;
	ao_eeprom->size = 64;
	ao_eeprom->data = malloc(ao_eeprom->size);
	if (!ao_eeprom->data) {
		free(ao_eeprom);
		return NULL;
	}
	while (fgets(line, sizeof(line), file) != NULL) {
		int nword;
		char *words[64];
		char *saveptr;
		l = line;
		for (nword = 0; nword < 64; nword++) {
			words[nword] = strtok_r(l, " \t\n", &saveptr);
			l = NULL;
			if (words[nword] == NULL)
				break;
		}
		if (((ao_eeprom->log_format == AO_LOG_FORMAT_TELEMEGA_OLD || ao_eeprom->log_format == AO_LOG_FORMAT_TELEMEGA) && nword == 30 && strlen(words[0]) == 1) ||
		    ((ao_eeprom->log_format == AO_LOG_FORMAT_EASYMINI1 || ao_eeprom->log_format == AO_LOG_FORMAT_EASYMINI2) && nword == 14 && strlen(words[0]) == 1) ||
		    (ao_eeprom->log_format == AO_LOG_FORMAT_TELEMETRUM && nword == 14 && strlen(words[0]) == 1))
		{
			int		i;
			uint8_t		type;
			uint16_t	tick;

			type = words[0][0];
			tick = strtoul(words[1], NULL, 16);
			ao_eeprom_add_u8(ao_eeprom, type);
			ao_eeprom_add_u8(ao_eeprom, 0);	/* checksum */
			ao_eeprom_add_u16(ao_eeprom, tick);
			for (i = 2; i < nword; i++)
				ao_eeprom_add_u8(ao_eeprom, strtoul(words[i], NULL, 16));
		}
		else if (nword == 4 && strlen(words[0]) == 1) {
			uint8_t		type;
			uint16_t	tick, a, b;
			type = words[0][0];
			tick = strtoul(words[1], NULL, 16);
			a = strtoul(words[2], NULL, 16);
			b = strtoul(words[3], NULL, 16);
			if (type == 'P')
				type = 'A';
			ao_eeprom_add_u8(ao_eeprom, type);
			ao_eeprom_add_u8(ao_eeprom, 0);	/* checksum */
			ao_eeprom_add_u16(ao_eeprom, tick);
			ao_eeprom_add_u16(ao_eeprom, a);
			ao_eeprom_add_u16(ao_eeprom, b);
		}
		else if (nword == 3 && strcmp(words[0], "ms5607") == 0) {
			if (strcmp(words[1], "reserved:") == 0)
				ao_eeprom->ms5607_prom.reserved = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "sens:") == 0)
				ao_eeprom->ms5607_prom.sens = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "off:") == 0)
				ao_eeprom->ms5607_prom.off = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "tcs:") == 0)
				ao_eeprom->ms5607_prom.tcs = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "tco:") == 0)
				ao_eeprom->ms5607_prom.tco = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "tref:") == 0)
				ao_eeprom->ms5607_prom.tref = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "tempsens:") == 0)
				ao_eeprom->ms5607_prom.tempsens = strtoul(words[2], NULL, 10);
			else if (strcmp(words[1], "crc:") == 0)
				ao_eeprom->ms5607_prom.crc = strtoul(words[2], NULL, 10);
			continue;
		}
#if AO_NUM_PYRO
		else if (nword >= 3 && strcmp(words[0], "Pyro") == 0) {
			int	p = strtoul(words[1], NULL, 10);
			int	i, j;
			struct ao_pyro	*pyro = &ao_eeprom->config.pyro[p];

			for (i = 2; i < nword; i++) {
				for (j = 0; j < NUM_PYRO_VALUES; j++)
					if (!strcmp (words[i], ao_pyro_values[j].name))
						break;
				if (j == NUM_PYRO_VALUES)
					continue;
				pyro->flags |= ao_pyro_values[j].flag;
				if (ao_pyro_values[j].offset != NO_VALUE && i + 1 < nword) {
					int16_t	val = strtoul(words[++i], NULL, 10);
					printf("pyro %d condition %s value %d\n", p, words[i-1], val);
					*((int16_t *) ((char *) pyro + ao_pyro_values[j].offset)) = val;
				}
			}
		}
#endif
		else if (nword == 2 && strcmp(words[0], "log-format") == 0) {
			ao_eeprom->log_format = strtoul(words[1], NULL, 10);
		} else if (nword == 2 && strcmp(words[0], "serial-number") == 0) {
			ao_eeprom->serial_number = strtoul(words[1], NULL, 10);
		} else if (nword >= 6 && strcmp(words[0], "Accel") == 0) {
			ao_eeprom->config.accel_plus_g = atoi(words[3]);
			ao_eeprom->config.accel_minus_g = atoi(words[5]);
#if HAS_GYRO
		} else if (nword >= 8 && strcmp(words[0], "IMU") == 0) {
			ao_eeprom->config.accel_zero_along = atoi(words[3]);
			ao_eeprom->config.accel_zero_across = atoi(words[5]);
			ao_eeprom->config.accel_zero_through = atoi(words[7]);
#endif
		} else if (nword >= 4 && strcmp(words[0], "Main") == 0) {
			ao_eeprom->config.main_deploy = atoi(words[2]);
		} else if (nword >= 3 && strcmp(words[0], "Apogee") == 0 &&
			   strcmp(words[1], "lockout:") == 0) {
			ao_eeprom->config.apogee_lockout = atoi(words[2]);
		} else if (nword >= 3 && strcmp(words[0], "Pad") == 0 &&
			   strcmp(words[1], "orientation:") == 0) {
			ao_eeprom->config.pad_orientation = atoi(words[2]);
		}
	}
	return ao_eeprom;
}

