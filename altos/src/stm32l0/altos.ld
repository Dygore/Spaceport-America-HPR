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

__flash   = 0x08000000;
__flash_size = 12k;
__storage = __flash + __flash_size;
__storage_size = 16k - __flash_size;
__ram     = 0x20000000;
__ram_size = 2k;
__stack_size = 512;

PROVIDE(__storage = __storage);
PROVIDE(__storage_size = __storage_size);

INCLUDE picolibc.ld
INCLUDE stm32l0.ld
