/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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

AO_ROMCONFIG_SYMBOL uint16_t ao_romconfig_version = AO_ROMCONFIG_VERSION;
AO_ROMCONFIG_SYMBOL uint16_t ao_romconfig_check = (uint16_t) ~AO_ROMCONFIG_VERSION;
AO_ROMCONFIG_SYMBOL uint16_t ao_serial_number = 0;
#if HAS_RADIO
AO_ROMCONFIG_SYMBOL uint32_t ao_radio_cal = AO_RADIO_CAL_DEFAULT;
#endif
