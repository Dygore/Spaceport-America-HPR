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

#ifndef _AO_LOG_MICRO_H_
#define _AO_LOG_MICRO_H_

#ifndef N_SAMPLES_TYPE
#define N_SAMPLES_TYPE	uint16_t
#endif

#define PA_GROUND_OFFSET	0
#define PA_MIN_OFFSET		4
#define N_SAMPLES_OFFSET	8
#define STARTING_LOG_OFFSET	(N_SAMPLES_OFFSET + sizeof (N_SAMPLES_TYPE))
#ifndef MAX_LOG_OFFSET
#define MAX_LOG_OFFSET		512
#endif

#define AO_LOG_ID_SHIFT	12
#define AO_LOG_ID_WIDTH	4
#define AO_LOG_ID_MASK	(((1 << AO_LOG_ID_WIDTH) - 1) << AO_LOG_ID_SHIFT);

void
ao_log_hex(uint8_t b);

void
ao_log_newline(void);

void
ao_log_micro_save(void);

void
ao_log_micro_restore(void);

void
ao_log_micro_data(void);

void
ao_log_micro_dump(void);

#endif /* _AO_LOG_MICRO_H_ */
