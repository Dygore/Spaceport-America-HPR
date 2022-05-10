/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_SERVO_H_
#define _AO_SERVO_H_

void
ao_servo_run(uint16_t speed, uint8_t dir, uint16_t timeout);

void
ao_servo_init(void);

#define AO_SERVO_FORE	0
#define AO_SERVO_BACK	1

/* To configure the servo:
 *
 *	#define AO_SERVO_PWM			<pwm index>
 *	#define AO_SERVO_DIR_PORT		<gpio>
 *	#define AO_SERVO_DIR_PIN		<pin>
 *	#define AO_SERVO_LIMIT_FORE_PORT	<gpio>
 *	#define AO_SERVO_LIMIT_FORE_PIN		<pin>
 *	#define AO_SERVO_LIMIT_BACK_PORT	<gpio>
 *	#define AO_SERVO_LIMIT_BACK_PIN		<pin>
 */

#endif /* _AO_SERVO_H_ */
