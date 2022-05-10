/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_PS2_H_
#define _AO_PS2_H_

extern uint8_t			ao_ps2_stdin;

int
ao_ps2_poll(void);

uint8_t
ao_ps2_get(void);

void
ao_ps2_put(uint8_t b);

uint8_t
ao_ps2_is_down(uint8_t code);

int
ao_ps2_poll_key(void);

uint8_t
ao_ps2_get_key(void);

int
ao_ps2_ascii(uint8_t key);

int
_ao_ps2_pollchar(void);

char
ao_ps2_getchar(void);

void
ao_ps2_init(void);

/* From http://computer-engineering.org/ps2keyboard/ */

/* Device responds with ACK and then resets */
#define AO_PS2_RESET				0xff

/* Device retransmits last byte */
#define AO_PS2_RESEND				0xfe

/* Setting key report only works in mode 3 */

/* Disable break and typematic for specified mode 3 keys. Terminate with invalid key */
#define AO_PS2_SET_KEY_MAKE			0xfd

/* Disable typematic for keys */
#define AO_PS2_SET_KEY_MAKE_BREAK		0xfc

/* Disable break code for keys */
#define AO_PS2_SET_KEY_TYPEMATIC		0xfb

/* Enable make, break and typematic */
#define AO_PS2_SET_KEY_TYPEMATIC_MAKE_BREAK	0xfa

/* Disable break and typematic for all */
#define AO_PS2_SET_ALL_MAKE			0xf9

/* Disable typematic for all */
#define AO_PS2_SET_ALL_MAKE_BREAK		0xf8

/* Disable break for all */
#define AO_PS2_SET_ALL_TYPEMATIC		0xf7

/* Set keyboard to default (repeat, report and scan code set 2) */
#define AO_PS2_SET_DEFAULT			0xf6

/* Disable and reset to default */
#define AO_PS2_DISABLE				0xf5

/* Enable */
#define AO_PS2_ENABLE				0xf4

/* Set repeat rate. Bytes 5-6 are the start delay, bits 0-4 are the rate */
#define AO_PS2_SET_REPEAT_RATE			0xf3

/* Read keyboard id. Returns two bytes */
#define AO_PS2_GETID				0xf2

/* Set scan code (1, 2, or 3) */
#define AO_PS2_SET_SCAN_CODE_SET		0xf0

/* Echo. Keyboard replies with Echo */
#define AO_PS2_ECHO				0xee

/* Set LEDs */
#define AO_PS2_SET_LEDS				0xed
# define AO_PS2_SET_LEDS_SCROLL			0x01
# define AO_PS2_SET_LEDS_NUM			0x02
# define AO_PS2_SET_LEDS_CAPS			0x04

#define AO_PS2_BREAK				0xf0
#define AO_PS2_ACK				0xfa
#define AO_PS2_ERROR				0xfc
#define AO_PS2_NAK				0xfe

/* Scan code set 3 */

#define AO_PS2_A		0x1c
#define AO_PS2_B		0x32
#define AO_PS2_C		0x21
#define AO_PS2_D		0x23
#define AO_PS2_E		0x24
#define AO_PS2_F		0x2b
#define AO_PS2_G		0x34
#define AO_PS2_H		0x33
#define AO_PS2_I		0x43
#define AO_PS2_J		0x3b
#define AO_PS2_K		0x42
#define AO_PS2_L		0x4b
#define AO_PS2_M		0x3a
#define AO_PS2_N		0x31
#define AO_PS2_O		0x44
#define AO_PS2_P		0x4d
#define AO_PS2_Q		0x15
#define AO_PS2_R		0x2d
#define AO_PS2_S		0x1b
#define AO_PS2_T		0x2c
#define AO_PS2_U		0x3c
#define AO_PS2_V		0x2a
#define AO_PS2_W		0x1d
#define AO_PS2_X		0x22
#define AO_PS2_Y		0x35
#define AO_PS2_Z		0x1a
#define AO_PS2_0		0x45
#define AO_PS2_1		0x16
#define AO_PS2_2		0x1e
#define AO_PS2_3		0x26
#define AO_PS2_4		0x25
#define AO_PS2_5		0x2e
#define AO_PS2_6		0x36
#define AO_PS2_7		0x3d
#define AO_PS2_8		0x3e
#define AO_PS2_9		0x46
#define AO_PS2_GRAVE		0x0e
#define AO_PS2_HYPHEN		0x4e
#define AO_PS2_EQUAL		0x55
#define AO_PS2_BACKSLASH	0x5c
#define AO_PS2_BACKSPACE	0x66
#define AO_PS2_SPACE		0x29
#define AO_PS2_TAB		0x0d
#define AO_PS2_CAPS_LOCK	0x14
#define AO_PS2_L_SHIFT		0x12
#define AO_PS2_L_CTRL		0x11
#define AO_PS2_L_WIN		0x8b
#define AO_PS2_L_ALT		0x19
#define AO_PS2_R_SHIFT		0x59
#define AO_PS2_R_CTRL		0x58
#define AO_PS2_R_WIN		0x8c
#define AO_PS2_R_ALT		0x39
#define AO_PS2_APPS		0x8d
#define AO_PS2_ENTER		0x5a
#define AO_PS2_ESC		0x08
#define AO_PS2_F1		0x07
#define AO_PS2_F2		0x0f
#define AO_PS2_F3		0x17
#define AO_PS2_F4		0x1f
#define AO_PS2_F5		0x27
#define AO_PS2_F6		0x2f
#define AO_PS2_F7		0x37
#define AO_PS2_F8		0x3f
#define AO_PS2_F9		0x47
#define AO_PS2_F10		0x4f
#define AO_PS2_F11		0x56
#define AO_PS2_F12		0x5e
#define AO_PS2_PRNT_SCRN	0x57
#define AO_PS2_SCROLL_LOCK	0x5f
#define AO_PS2_PAUSE		0x62
#define AO_PS2_OPEN_SQ		0x54
#define AO_PS2_INSERT		0x67
#define AO_PS2_HOME		0x6e
#define AO_PS2_PG_UP		0x6f
#define AO_PS2_DELETE		0x64
#define AO_PS2_END		0x65
#define AO_PS2_PG_DN		0x6d
#define AO_PS2_UP		0x63
#define AO_PS2_LEFT		0x61
#define AO_PS2_DOWN		0x60
#define AO_PS2_RIGHT		0x6a
#define AO_PS2_NUM_LOCK		0x76
#define AO_PS2_KP_TIMES		0x7e
#define AO_PS2_KP_PLUS		0x7c
#define AO_PS2_KP_ENTER		0x79
#define AO_PS2_KP_DECIMAL	0x71
#define AO_PS2_KP_0		0x70
#define AO_PS2_KP_1		0x69
#define AO_PS2_KP_2		0x72
#define AO_PS2_KP_3		0x7a
#define AO_PS2_KP_4		0x6b
#define AO_PS2_KP_5		0x73
#define AO_PS2_KP_6		0x74
#define AO_PS2_KP_7		0x6c
#define AO_PS2_KP_8		0x75
#define AO_PS2_KP_9		0x7d
#define AO_PS2_CLOSE_SQ		0x5b
#define AO_PS2_SEMICOLON	0x4c
#define AO_PS2_ACUTE		0x52
#define AO_PS2_COMMA		0x41
#define AO_PS2_PERIOD		0x49
#define AO_PS2_SLASH		0x4a

#define AO_PS2_RELEASE_FLAG	0x80

#endif /* _AO_PS2_H_ */
