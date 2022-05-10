/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

MEMORY {
	rom (rx) :   ORIGIN = 0x08000000, LENGTH = 1M
	ram (!w) :   ORIGIN = 0x20000000, LENGTH = 256k - 256
	stack (!w) : ORIGIN = 0x20000000 + 256k - 256, LENGTH = 256
}

INCLUDE registers.ld

EXTERN (stm_interrupt_vector)

SECTIONS {
	/*
	 * Rom contents
	 */

	.interrupt : {
		__text_start__ = .;
		*(.interrupt)	/* Interrupt vectors */
	} > rom

	.text : {
		*(.text*)	/* Executable code */
		*(.rodata*)	/* Constants */
	} > rom

	.exidx : {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > rom

	__text_end__ = .;

	/* Data -- relocated to RAM, but written to ROM
	 */
	.data : {
		_start__ = .;
		*(.data)	/* initialized data */
		. = ALIGN(4);
		_end__ = .;
	} >ram AT>rom

	.bss : {
		__bss_start__ = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} >ram

	PROVIDE(end = .);

	PROVIDE(__stack__ = ORIGIN(stack) + LENGTH(stack));
}

ENTRY(start);


