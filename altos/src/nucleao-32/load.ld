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
	rom (rx) :   ORIGIN = 0x08001000, LENGTH = 27K
	flash (rx) : ORIGIN = 0x08007c00, LENGTH = 1K
	ram (!w) :   ORIGIN = 0x20000000, LENGTH = 6k - 128
	stack (!w) : ORIGIN = 0x20000000 + 6k - 128, LENGTH = 128
}

INCLUDE registers.ld

EXTERN (stm_interrupt_vector)

SECTIONS {
	/*
	 * Rom contents
	 */

	.interrupt ORIGIN(ram) : AT (ORIGIN(rom)) {
		__interrupt_start__ = .;
		__interrupt_rom__ = ORIGIN(rom);
		*(.interrupt)	/* Interrupt vectors */
		__interrupt_end__ = .;
	} > ram

	.text ORIGIN(rom) + 0x100 : {
		__text_start__ = .;

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
	} > rom

	.ARM.exidx : {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > rom

	.rodata : {
		*(.rodata*)	/* Constants */
	} > rom

	__text_end__ = .;

	/* Boot data which must live at the start of ram so that
	 * the application and bootloader share the same addresses.
	 * This must be all uninitialized data
	 */
	.boot (NOLOAD) : {
		__boot_start__ = .;
		*(.boot)
		. = ALIGN(4);
		__boot_end__ = .;
	} >ram

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

	__flash__ = ORIGIN(flash);
}

ENTRY(start);


