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

#include "ao.h"
#include "ao_servo.h"
#include "ao_pwm.h"
#include "ao_exti.h"

static uint8_t	limit_wakeup;

static void
ao_limit_isr(void)
{
	ao_wakeup(&limit_wakeup);
}

static void
ao_limit_enable(uint8_t dir)
{
	if (dir == AO_SERVO_FORE)
		ao_exti_enable(AO_SERVO_LIMIT_FORE_PORT, AO_SERVO_LIMIT_FORE_BIT);
	else
		ao_exti_enable(AO_SERVO_LIMIT_BACK_PORT, AO_SERVO_LIMIT_BACK_BIT);
}

static void
ao_limit_disable(uint8_t dir)
{
	if (dir == AO_SERVO_FORE)
		ao_exti_disable(AO_SERVO_LIMIT_FORE_PORT, AO_SERVO_LIMIT_FORE_BIT);
	else
		ao_exti_disable(AO_SERVO_LIMIT_BACK_PORT, AO_SERVO_LIMIT_BACK_BIT);
}

static uint8_t
ao_limit_get(uint8_t dir)
{
	if (dir == AO_SERVO_FORE)
		return !ao_gpio_get(AO_SERVO_LIMIT_FORE_PORT, AO_SERVO_LIMIT_FORE_BIT, PIN);
	else
		return !ao_gpio_get(AO_SERVO_LIMIT_BACK_PORT, AO_SERVO_LIMIT_BACK_BIT, PIN);
}

void
ao_servo_run(uint16_t speed, uint8_t dir, uint16_t timeout)
{
	printf ("speed %d dir %d\n", speed, dir);

	/* Turn on the motor */
	ao_gpio_set(AO_SERVO_DIR_PORT, AO_SERVO_DIR_BIT, AO_SERVO_DIR_PIN, dir);
	ao_pwm_set(AO_SERVO_SPEED_PWM, speed);

	/* Wait until the limit sensor is triggered */
	ao_arch_block_interrupts();
	ao_limit_enable(dir);
	while (!ao_limit_get(dir))
		if (ao_sleep_for(&limit_wakeup, timeout))
			break;
	ao_limit_disable(dir);
	ao_arch_release_interrupts();

	/* Turn off the motor */
	ao_pwm_set(AO_SERVO_SPEED_PWM, 0);
}

#define init_limit(p,b) do {						\
		ao_enable_port(p);					\
		ao_exti_setup(p, b,					\
			      AO_EXTI_MODE_PULL_UP|AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED, \
			      ao_limit_isr);				\
	} while (0)


static void
ao_servo_cmd(void)
{
	uint8_t	dir;
	uint16_t speed;

	ao_cmd_decimal();
	dir = ao_cmd_lex_u32;
	ao_cmd_decimal();
	speed = ao_cmd_lex_u32;
	if (ao_cmd_status != ao_cmd_success)
		return;

	printf("Run servo %d\n", dir);
	ao_servo_run(speed, dir, AO_MS_TO_TICKS(200));
}

static const struct ao_cmds ao_servo_cmds[] = {
	{ ao_servo_cmd,	"S <dir> <speed>\0Run servo in indicated direction" },
	{ 0, NULL },
};


void
ao_servo_init(void)
{
	init_limit(AO_SERVO_LIMIT_FORE_PORT, AO_SERVO_LIMIT_FORE_BIT);
	init_limit(AO_SERVO_LIMIT_BACK_PORT, AO_SERVO_LIMIT_BACK_BIT);
	ao_enable_output(AO_SERVO_DIR_PORT, AO_SERVO_DIR_BIT, AO_SERVO_DIR_PIN, 0);
	ao_cmd_register(&ao_servo_cmds[0]);
}
