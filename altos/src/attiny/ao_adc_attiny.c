/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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

#include <ao.h>

/*
 * ATtiny ADC interface
 */

uint16_t
ao_adc_read(uint8_t mux)
{
	uint8_t	low, high;

	/* Set the mux */
	ADMUX = mux;

	/* Start conversion */
	ADCSRA = ((1 << ADEN) |
		  (1 << ADSC) |
		  (0 << ADATE) |
		  (0 << ADIF) |
		  (0 << ADIE) |
		  (0 << ADPS2) |
		  (0 << ADPS1) |
		  (0 << ADPS0));

	/* Await conversion complete */
	while ((ADCSRA & (1 << ADSC)) != 0)
		;

	/* Read low first */
	low = ADCL;
	high = ADCH;

	return (((uint16_t) high) << 8) | low;
}
