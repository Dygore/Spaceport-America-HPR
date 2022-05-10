/*
 * Copyright © 2019 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_LCD_FONT_H_
#define _AO_LCD_FONT_H_

void
ao_lcd_font_init(void);

void
ao_lcd_font_string(char *s);

void
ao_lcd_font_char(uint8_t pos, char c, uint16_t flags);

#endif /* _AO_LCD_FONT_H_ */
