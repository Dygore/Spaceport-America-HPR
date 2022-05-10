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

#ifndef AO_FLIGHT_TEST
#include <ao.h>
#include <ao_sample.h>
#include <ao_flight.h>
#endif
#include <ao_pyro.h>

#if IS_COMPANION
#include <ao_companion.h>
#define ao_accel ao_companion_command.accel
#define ao_speed ao_companion_command.speed
#define ao_height ao_companion_command.height
#define ao_flight_state ao_companion_command.flight_state
#define ao_motor_number ao_companion_command.motor_number
#endif

#define ao_lowbit(x)	((x) & (-x))

#ifndef AO_FLIGHT_TEST
enum ao_igniter_status
ao_pyro_status(uint8_t p)
{
	struct ao_data packet;
	int16_t value;

	ao_arch_critical(
		ao_data_get(&packet);
		);

	value = (AO_IGNITER_CLOSED>>1);
	value = AO_SENSE_PYRO(&packet, p);
	if (value < AO_IGNITER_OPEN)
		return ao_igniter_open;
	else if (value > AO_IGNITER_CLOSED)
		return ao_igniter_ready;
	else
		return ao_igniter_unknown;
}

void
ao_pyro_print_status(void)
{
	uint8_t	p;

	for(p = 0; p < AO_PYRO_NUM; p++) {
		enum ao_igniter_status status = ao_pyro_status(p);
		printf("Igniter: %d Status: %s\n",
		       p, ao_igniter_status_names[status]);
	}
}
#endif

uint16_t	ao_pyro_fired;
uint16_t	ao_pyro_inhibited;

#ifndef PYRO_DBG
#define PYRO_DBG	0
#endif

#if PYRO_DBG
int pyro_dbg;
#define DBG(...)	do { if (pyro_dbg) { printf("\t%d: ", (int) (pyro - ao_config.pyro)); printf(__VA_ARGS__); } } while (0)
#else
#define DBG(...)
#endif

static angle_t
ao_sample_max_orient(void)
{
	uint8_t	i;
	angle_t	max = ao_sample_orients[0];

	for (i = 1; i < AO_NUM_ORIENT; i++) {
		angle_t a = ao_sample_orients[i];
		if (a > max)
			max = a;
	}
	return max;
}
/*
 * Given a pyro structure, figure out
 * if the current flight state satisfies all
 * of the requirements
 */
static uint8_t
ao_pyro_ready(const struct ao_pyro *pyro)
{
	enum ao_pyro_flag flag, flags;
#if HAS_GYRO
	angle_t max_orient;
#endif

	flags = pyro->flags;
	while (flags != ao_pyro_none) {
		flag = ao_lowbit(flags);
		flags &= ~flag;
		switch (flag) {

		case ao_pyro_accel_less:
			if (ao_accel <= pyro->accel_less)
				continue;
			DBG("accel %d > %d\n", ao_accel, pyro->accel_less);
			break;
		case ao_pyro_accel_greater:
			if (ao_accel >= pyro->accel_greater)
				continue;
			DBG("accel %d < %d\n", ao_accel, pyro->accel_greater);
			break;
		case ao_pyro_speed_less:
			if (ao_speed <= pyro->speed_less)
				continue;
			DBG("speed %d > %d\n", ao_speed, pyro->speed_less);
			break;
		case ao_pyro_speed_greater:
			if (ao_speed >= pyro->speed_greater)
				continue;
			DBG("speed %d < %d\n", ao_speed, pyro->speed_greater);
			break;
		case ao_pyro_height_less:
			if (ao_height <= pyro->height_less)
				continue;
			DBG("height %d > %d\n", ao_height, pyro->height_less);
			break;
		case ao_pyro_height_greater:
			if (ao_height >= pyro->height_greater)
				continue;
			DBG("height %d < %d\n", ao_height, pyro->height_greater);
			break;

#if HAS_GYRO
		case ao_pyro_orient_less:
			max_orient = ao_sample_max_orient();
			if (max_orient <= pyro->orient_less)
				continue;
			DBG("orient %d > %d\n", max_orient, pyro->orient_less);
			break;
		case ao_pyro_orient_greater:
			max_orient = ao_sample_max_orient();
			if (max_orient >= pyro->orient_greater)
				continue;
			DBG("orient %d < %d\n", max_orient, pyro->orient_greater);
			break;
#endif

		case ao_pyro_time_less:
			if ((AO_TICK_SIGNED) (ao_time() - ao_launch_tick) <= pyro->time_less)
				continue;
			DBG("time %d > %d\n", (AO_TICK_SIGNED)(ao_time() - ao_launch_tick), pyro->time_less);
			break;
		case ao_pyro_time_greater:
			if ((AO_TICK_SIGNED) (ao_time() - ao_launch_tick) >= pyro->time_greater)
				continue;
			DBG("time %d < %d\n", (AO_TICK_SIGNED)(ao_time() - ao_launch_tick), pyro->time_greater);
			break;

		case ao_pyro_ascending:
			if (ao_speed > 0)
				continue;
			DBG("not ascending speed %d\n", ao_speed);
			break;
		case ao_pyro_descending:
			if (ao_speed < 0)
				continue;
			DBG("not descending speed %d\n", ao_speed);
			break;

		case ao_pyro_after_motor:
			if (ao_motor_number >= pyro->motor)
				continue;
			DBG("motor %d != %d\n", ao_motor_number, pyro->motor);
			break;

		case ao_pyro_delay:
			/* handled separately */
			continue;

		case ao_pyro_state_less:
			if (ao_flight_state < pyro->state_less)
				continue;
			DBG("state %d >= %d\n", ao_flight_state, pyro->state_less);
			break;
		case ao_pyro_state_greater_or_equal:
			if (ao_flight_state >= pyro->state_greater_or_equal)
				continue;
			DBG("state %d < %d\n", ao_flight_state, pyro->state_greater_or_equal);
			break;

		default:
			continue;
		}
		return false;
	}
	return true;
}

#ifndef AO_FLIGHT_TEST
static void
ao_pyro_pin_set(uint8_t p, uint8_t v)
{
	switch (p) {
#if AO_PYRO_NUM > 0
	case 0: ao_gpio_set(AO_PYRO_PORT_0, AO_PYRO_PIN_0, v); break;
#endif
#if AO_PYRO_NUM > 1
	case 1: ao_gpio_set(AO_PYRO_PORT_1, AO_PYRO_PIN_1, v); break;
#endif
#if AO_PYRO_NUM > 2
	case 2: ao_gpio_set(AO_PYRO_PORT_2, AO_PYRO_PIN_2, v); break;
#endif
#if AO_PYRO_NUM > 3
	case 3: ao_gpio_set(AO_PYRO_PORT_3, AO_PYRO_PIN_3, v); break;
#endif
#if AO_PYRO_NUM > 4
	case 4: ao_gpio_set(AO_PYRO_PORT_4, AO_PYRO_PIN_4, v); break;
#endif
#if AO_PYRO_NUM > 5
	case 5: ao_gpio_set(AO_PYRO_PORT_5, AO_PYRO_PIN_5, v); break;
#endif
#if AO_PYRO_NUM > 6
	case 6: ao_gpio_set(AO_PYRO_PORT_6, AO_PYRO_PIN_6, v); break;
#endif
#if AO_PYRO_NUM > 7
	case 7: ao_gpio_set(AO_PYRO_PORT_7, AO_PYRO_PIN_7, v); break;
#endif
	default: break;
	}
}
#endif

uint8_t	ao_pyro_wakeup;

static void
ao_pyro_pins_fire(uint16_t fire)
{
	uint8_t p;

	for (p = 0; p < AO_PYRO_NUM; p++) {
		if (fire & (1 << p))
			ao_pyro_pin_set(p, 1);
	}
	ao_delay(ao_config.pyro_time);
	for (p = 0; p < AO_PYRO_NUM; p++) {
		if (fire & (1 << p))
			ao_pyro_pin_set(p, 0);
	}
	ao_delay(AO_MS_TO_TICKS(50));
}

static AO_TICK_TYPE pyro_delay_done[AO_PYRO_NUM];

static uint8_t
ao_pyro_check(void)
{
	const struct ao_pyro	*pyro;
	uint8_t		p, any_waiting;
	uint16_t	fire = 0;

	any_waiting = 0;
	for (p = 0; p < AO_PYRO_NUM; p++) {
		pyro = &ao_config.pyro[p];

		/* Ignore igniters which have already fired or inhibited
		 */
		if ((ao_pyro_fired|ao_pyro_inhibited) & (1 << p))
			continue;

		/* Ignore disabled igniters
		 */
		if (!pyro->flags)
			continue;

		any_waiting = 1;
		/* Check pyro state to see if it should fire
		 */
		if (!pyro_delay_done[p]) {
			if (!ao_pyro_ready(pyro))
				continue;

			/* If there's a delay set, then remember when
			 * it expires
			 */
			if (pyro->flags & ao_pyro_delay) {
				AO_TICK_TYPE delay_done;
				AO_TICK_SIGNED delay = pyro->delay;
				if (delay < 0)
					delay = 0;
				delay_done = ao_time() + (AO_TICK_TYPE) delay;

				/*
				 * delay_done is the time at which the
				 * delay ends, but it is also used as
				 * an indication that a delay is
				 * active -- non-zero values indicate
				 * an active delay. This code ensures
				 * the value is non-zero, which might
				 * introduce an additional tick
				 * of delay.
				 */
				if (delay_done == 0)
					delay_done = 1;
				pyro_delay_done[p] = delay_done;
			}
		}

		/* Check to see if we're just waiting for
		 * the delay to expire
		 */
		if (pyro_delay_done[p] != 0) {

			/* Check to make sure the required conditions
			 * remain valid. If not, inhibit the channel
			 * by setting the inhibited bit
			 */
			if (!ao_pyro_ready(pyro)) {
				ao_pyro_inhibited |= (uint16_t) (1 << p);
				continue;
			}

			if ((AO_TICK_SIGNED) (ao_time() - pyro_delay_done[p]) < 0)
				continue;
		}

		fire |= (uint16_t) (1U << p);
	}

	if (fire) {
		ao_pyro_fired |= fire;
		ao_pyro_pins_fire(fire);
	}

	return any_waiting;
}

#define NO_VALUE	0xff

#define AO_PYRO_NAME_LEN	4

#if !DISABLE_HELP
#define ENABLE_HELP 1
#endif

#if ENABLE_HELP
#define HELP(s)	(s)
#else
#define HELP(s)
#endif

const struct {
	char			name[AO_PYRO_NAME_LEN];
	enum ao_pyro_flag	flag;
	uint8_t			offset;
#if ENABLE_HELP
	char			*help;
#endif
} ao_pyro_values[] = {
	{ "a<",	ao_pyro_accel_less,	offsetof(struct ao_pyro, accel_less), HELP("accel less (m/ss * 16)") },
	{ "a>",	ao_pyro_accel_greater,	offsetof(struct ao_pyro, accel_greater), HELP("accel greater (m/ss * 16)") },

	{ "s<",	ao_pyro_speed_less,	offsetof(struct ao_pyro, speed_less), HELP("speed less (m/s * 16)") },
	{ "s>",	ao_pyro_speed_greater,	offsetof(struct ao_pyro, speed_greater), HELP("speed greater (m/s * 16)") },

	{ "h<",	ao_pyro_height_less,	offsetof(struct ao_pyro, height_less), HELP("height less (m)") },
	{ "h>",	ao_pyro_height_greater,	offsetof(struct ao_pyro, height_greater), HELP("height greater (m)") },

#if HAS_GYRO
	{ "o<",	ao_pyro_orient_less,	offsetof(struct ao_pyro, orient_less), HELP("orient less (deg)") },
	{ "o>",	ao_pyro_orient_greater,	offsetof(struct ao_pyro, orient_greater), HELP("orient greater (deg)")  },
#endif

	{ "t<",	ao_pyro_time_less,	offsetof(struct ao_pyro, time_less), HELP("time less (s * 100)") },
	{ "t>",	ao_pyro_time_greater,	offsetof(struct ao_pyro, time_greater), HELP("time greater (s * 100)")  },

	{ "f<",	ao_pyro_state_less,	offsetof(struct ao_pyro, state_less), HELP("state less") },
	{ "f>=",ao_pyro_state_greater_or_equal,	offsetof(struct ao_pyro, state_greater_or_equal), HELP("state greater or equal")  },

	{ "A", ao_pyro_ascending,	NO_VALUE, HELP("ascending") },
	{ "D", ao_pyro_descending,	NO_VALUE, HELP("descending") },

	{ "m", ao_pyro_after_motor,	offsetof(struct ao_pyro, motor), HELP("after motor") },

	{ "d", ao_pyro_delay,		offsetof(struct ao_pyro, delay), HELP("delay before firing (s * 100)") },
	{ "", ao_pyro_none,		NO_VALUE, HELP(NULL) },
};

#define NUM_PYRO_VALUES (sizeof ao_pyro_values / sizeof ao_pyro_values[0])

#ifndef AO_FLIGHT_TEST
static void
ao_pyro(void)
{
	uint8_t		any_waiting;

	ao_config_get();
	while (ao_flight_state < ao_flight_boost)
		ao_sleep(&ao_flight_state);

	for (;;) {
		ao_sleep_for(&ao_pyro_wakeup, AO_MS_TO_TICKS(100));
		if (ao_flight_state >= ao_flight_landed)
			break;
		any_waiting = ao_pyro_check();
		if (!any_waiting)
			break;
	}
	ao_exit();
}

struct ao_task ao_pyro_task;


static void
ao_pyro_print_name(uint8_t v)
{
	const char *s = ao_pyro_values[v].name;
	printf ("%s%s", s, "   " + strlen(s));
}

#if ENABLE_HELP
static void
ao_pyro_help(void)
{
	uint8_t v;
	for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
		ao_pyro_print_name(v);
		if (ao_pyro_values[v].offset != NO_VALUE)
			printf ("<n> ");
		else
			printf ("    ");
		printf ("%s\n", ao_pyro_values[v].help);
	}
}
#endif

static int32_t
ao_pyro_get(void *base, uint8_t offset, uint8_t size)
{
	int32_t value;
	switch (size) {
	case 8:
		value = *((uint8_t *) ((char *) base + offset));
		break;
	case 16:
	default:
		value = *((int16_t *) (void *) ((char *) base + offset));
		break;
	case 32:
		value = *((int32_t *) (void *) ((char *) base + offset));
		break;
	}
	return value;
}

static bool
ao_pyro_put(void *base, uint8_t offset, uint8_t size, int32_t value)
{
	switch (size) {
	case 8:
		if (value < 0)
			return false;
		*((uint8_t *) ((char *) base + offset)) = (uint8_t) value;
		break;
	case 16:
	default:
		*((int16_t *) (void *) ((char *) base + offset)) = (int16_t) value;
		break;
	case 32:
		*((int32_t *) (void *) ((char *) base + offset)) = value;
		break;
	}
	return true;
}

static uint8_t
ao_pyro_size(enum ao_pyro_flag flag)
{
	if (flag & AO_PYRO_8_BIT_VALUE)
		return 8;
	if (flag & AO_PYRO_32_BIT_VALUE)
		return 32;
	return 16;
}

void
ao_pyro_show(void)
{
	uint8_t 	p;
	uint8_t 	v;
	struct ao_pyro	*pyro;

	printf ("Pyro-count: %d\n", AO_PYRO_NUM);
	for (p = 0; p < AO_PYRO_NUM; p++) {
		printf ("Pyro %2d: ", p);
		pyro = &ao_config.pyro[p];
		if (!pyro->flags) {
			printf ("<disabled>\n");
			continue;
		}
		for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
			if (!(pyro->flags & ao_pyro_values[v].flag))
				continue;
			ao_pyro_print_name(v);
			if (ao_pyro_values[v].offset != NO_VALUE) {
				printf ("%6ld ",
					(long) ao_pyro_get(pyro,
							   ao_pyro_values[v].offset,
							   ao_pyro_size(ao_pyro_values[v].flag)));
			} else {
				printf ("       ");
			}
		}
		printf ("\n");
	}
}

void
ao_pyro_set(void)
{
	uint32_t	p;
	struct ao_pyro pyro_tmp;
	char	name[AO_PYRO_NAME_LEN];
	uint8_t	c;
	uint8_t	v;

	ao_cmd_white();

#if ENABLE_HELP
	switch (ao_cmd_lex_c) {
	case '?':
		ao_pyro_help();
		return;
	}
#endif

	p = ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (AO_PYRO_NUM <= p) {
		printf ("invalid pyro channel %lu\n", p);
		return;
	}
	memset(&pyro_tmp, '\0', sizeof (pyro_tmp));
	for (;;) {
		ao_cmd_white();
		if (ao_cmd_lex_c == '\n')
			break;

		for (c = 0; c < AO_PYRO_NAME_LEN - 1; c++) {
			if (ao_cmd_is_white() || ao_cmd_lex_c == '\n')
				break;
			name[c] = ao_cmd_lex_c;
			ao_cmd_lex();
		}
		name[c] = '\0';
		for (v = 0; ao_pyro_values[v].flag != ao_pyro_none; v++) {
			if (!strcmp (ao_pyro_values[v].name, name))
				break;
		}
		if (ao_pyro_values[v].flag == ao_pyro_none) {
			printf ("invalid pyro field %s\n", name);
			ao_cmd_status = ao_cmd_syntax_error;
			return;
		}
		pyro_tmp.flags |= ao_pyro_values[v].flag;
		if (ao_pyro_values[v].offset != NO_VALUE) {
			int32_t r = 1;
			ao_cmd_white();
			if (ao_cmd_lex_c == '-') {
				r = -1;
				ao_cmd_lex();
			}
			r *= (int32_t) ao_cmd_decimal();
			if (ao_cmd_status != ao_cmd_success)
				return;
			if (!ao_pyro_put(&pyro_tmp, ao_pyro_values[v].offset,
					 ao_pyro_size(ao_pyro_values[v].flag), r))
			{
				ao_cmd_status = ao_cmd_syntax_error;
				return;
			}
		}
	}
	_ao_config_edit_start();
	ao_config.pyro[p] = pyro_tmp;
	_ao_config_edit_finish();
}

void
ao_pyro_manual(uint8_t p)
{
	printf ("ao_pyro_manual %d\n", p);
	if (p >= AO_PYRO_NUM) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}
	ao_pyro_pins_fire(1 << p);
}

struct ao_pyro_old_values {
	enum ao_pyro_flag	flag;
	uint8_t			offset;
	uint8_t			size;
};

static const struct ao_pyro_old_values ao_pyro_1_24_values[] = {
	{ .flag = ao_pyro_accel_less, .offset = offsetof(struct ao_pyro_1_24, accel_less), 16 },
	{ .flag = ao_pyro_accel_greater, .offset = offsetof(struct ao_pyro_1_24, accel_greater), 16 },
	{ .flag = ao_pyro_speed_less, .offset = offsetof(struct ao_pyro_1_24, speed_less), 16 },
	{ .flag = ao_pyro_speed_greater, .offset = offsetof(struct ao_pyro_1_24, speed_greater), 16 },
	{ .flag = ao_pyro_height_less, .offset = offsetof(struct ao_pyro_1_24, height_less), 16 },
	{ .flag = ao_pyro_height_greater, .offset = offsetof(struct ao_pyro_1_24, height_greater), 16 },
	{ .flag = ao_pyro_orient_less, .offset = offsetof(struct ao_pyro_1_24, orient_less), 16 },
	{ .flag = ao_pyro_orient_greater, .offset = offsetof(struct ao_pyro_1_24, orient_greater), 16 },
	{ .flag = ao_pyro_time_less, .offset = offsetof(struct ao_pyro_1_24, time_less), 16 },
	{ .flag = ao_pyro_time_greater, .offset = offsetof(struct ao_pyro_1_24, time_greater), 16 },
	{ .flag = ao_pyro_delay, .offset = offsetof(struct ao_pyro_1_24, delay), 16 },
	{ .flag = ao_pyro_state_less, .offset = offsetof(struct ao_pyro_1_24, state_less), 8 },
	{ .flag = ao_pyro_state_greater_or_equal, .offset = offsetof(struct ao_pyro_1_24, state_greater_or_equal), 8 },
	{ .flag = ao_pyro_after_motor, .offset = offsetof(struct ao_pyro_1_24, motor), 16 },
};

#define NUM_PYRO_1_24_VALUES (sizeof ao_pyro_1_24_values / sizeof ao_pyro_1_24_values[0])

static int32_t
ao_pyro_get_1_24(void *base, enum ao_pyro_flag flag)
{
	unsigned v;

	for (v = 0; v < NUM_PYRO_1_24_VALUES; v++) {
		if (ao_pyro_1_24_values[v].flag == flag)
			return ao_pyro_get(base, ao_pyro_1_24_values[v].offset, ao_pyro_1_24_values[v].size);
	}
	return 0;
}

void
ao_pyro_update_version(void)
{
	if (ao_config.minor <= 24)
	{

		/* First, move all of the config bits that follow the pyro data */

		struct ao_config_1_24 *ao_config_1_24 = (void *) &ao_config;
		struct ao_pyro		*pyro_1_25 = &ao_config.pyro[0];
		struct ao_pyro_1_24	*pyro_1_24 = &(ao_config_1_24->pyro)[0];


		char	*pyro_base_1_25 = (void *) pyro_1_25;
		char	*pyro_base_1_24 = (void *) pyro_1_24;
		char	*after_pyro_1_25 = pyro_base_1_25 + AO_PYRO_NUM * sizeof (struct ao_pyro);
		char	*after_pyro_1_24 = pyro_base_1_24 + AO_PYRO_NUM * sizeof (struct ao_pyro_1_24);

		char	*config_end_1_25 = (void *) (&ao_config + 1);
		size_t	to_move = (size_t) (config_end_1_25 - after_pyro_1_25);

		memmove(after_pyro_1_25, after_pyro_1_24, to_move);

		/* Now, adjust all of the pyro entries */

		int p = AO_PYRO_NUM;

		/* New struct is larger than the old, so start at the
		 * last one and work towards the first
		 */
		while (p-- > 0) {
			unsigned v;
			int32_t value;
			struct ao_pyro	tmp;

			memset(&tmp, '\0', sizeof(tmp));
			tmp.flags = pyro_1_24[p].flags;

			for (v = 0; v < NUM_PYRO_VALUES; v++)
			{
				value = ao_pyro_get_1_24(&pyro_1_24[p], ao_pyro_values[v].flag);
				ao_pyro_put(&tmp, ao_pyro_values[v].offset,
					    ao_pyro_size(ao_pyro_values[v].flag), value);
			}
			memcpy(&pyro_1_25[p], &tmp, sizeof(tmp));
		}
	}
}

void
ao_pyro_init(void)
{
#if AO_PYRO_NUM > 0
	ao_enable_output(AO_PYRO_PORT_0, AO_PYRO_PIN_0, 0);
#endif
#if AO_PYRO_NUM > 1
	ao_enable_output(AO_PYRO_PORT_1, AO_PYRO_PIN_1, 0);
#endif
#if AO_PYRO_NUM > 2
	ao_enable_output(AO_PYRO_PORT_2, AO_PYRO_PIN_2, 0);
#endif
#if AO_PYRO_NUM > 3
	ao_enable_output(AO_PYRO_PORT_3, AO_PYRO_PIN_3, 0);
#endif
#if AO_PYRO_NUM > 4
	ao_enable_output(AO_PYRO_PORT_4, AO_PYRO_PIN_4, 0);
#endif
#if AO_PYRO_NUM > 5
	ao_enable_output(AO_PYRO_PORT_5, AO_PYRO_PIN_5, 0);
#endif
#if AO_PYRO_NUM > 6
	ao_enable_output(AO_PYRO_PORT_6, AO_PYRO_PIN_6, 0);
#endif
#if AO_PYRO_NUM > 7
	ao_enable_output(AO_PYRO_PORT_7, AO_PYRO_PIN_7, 0);
#endif
	ao_add_task(&ao_pyro_task, ao_pyro, "pyro");
}
#endif
