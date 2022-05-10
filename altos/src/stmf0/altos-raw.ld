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

MEMORY {
	rom (rx) :   ORIGIN = 0x08000000, LENGTH = 32K
	ram (!w) :   ORIGIN = 0x20000000, LENGTH = 6k - 128
	stack (!w) : ORIGIN = 0x20000000 + 6k - 128, LENGTH = 128
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

	.text ORIGIN(rom) + 0x100 : {

		/* Ick. What I want is to specify the
		 * addresses of some global constants so
		 * that I can find them across versions
		 * of the application. I can't figure out
		 * how to make gnu ld do that, so instead
		 * we just load the two files that include
		 * these defines in the right order here and
		 * expect things to 'just work'. Don't change
		 * the contents of those files, ok?
		 */
		ao_romconfig.o(.romconfig*)
		ao_product.o(.romconfig*)

		*(.text*)	/* Executable code */
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		*(.rodata*)	/* Constants */

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


