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

#ifndef _AO_EEPROM_H_
#define _AO_EEPROM_H_

/*
 * Write to eeprom
 */

uint8_t
ao_eeprom_write(ao_pos_t pos32, void *v, uint16_t len);

/*
 * Read from eeprom
 */
uint8_t
ao_eeprom_read(ao_pos_t pos, void *v, uint16_t len);

/*
 * Initialize eeprom
 */

void
ao_eeprom_init(void);

#endif /* _AO_EEPROM_H_ */
