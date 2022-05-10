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

#include <ao.h>
#include <ao_exti.h>
#include <ao_pad.h>
#include <ao_74hc165.h>
#include <ao_radio_cmac.h>

static uint8_t ao_pad_ignite;
static struct ao_pad_command	command;
static struct ao_pad_query	query;
static uint8_t	ao_pad_armed;
static AO_TICK_TYPE	ao_pad_arm_time;
static uint8_t	ao_pad_box;
static uint8_t	ao_pad_disabled;
static AO_TICK_TYPE	ao_pad_packet_time;

#ifndef AO_PAD_RSSI_MINIMUM
#define AO_PAD_RSSI_MINIMUM	-90
#endif

#define DEBUG	1

#if DEBUG
static uint8_t	ao_pad_debug;
#define PRINTD(...) (ao_pad_debug ? (printf(__VA_ARGS__), 0) : 0)
#define FLUSHD()    (ao_pad_debug ? (flush(), 0) : 0)
#else
#define PRINTD(...)
#define FLUSHD()
#endif

static void
ao_siren(uint8_t v)
{
#ifdef AO_SIREN
	ao_gpio_set(AO_SIREN_PORT, AO_SIREN_PIN, v);
#else
#if HAS_BEEP
	ao_beep(v ? AO_BEEP_MID : 0);
#else
	(void) v;
#endif
#endif
}

static void
ao_strobe(uint8_t v)
{
#ifdef AO_STROBE
	ao_gpio_set(AO_STROBE_PORT, AO_STROBE_PIN, v);
#else
	(void) v;
#endif
}

#ifdef AO_PAD_PORT_0
#define pins_pad(pad)	(*((AO_PAD_ ## pad ## _PORT) == AO_PAD_PORT_0 ? (&pins0) : (&pins1)))
#else
#define pins_pad(pad)	pins0
#define AO_PAD_PORT_0 AO_PAD_PORT
#endif

static void
ao_pad_run(void)
{
	AO_PORT_TYPE	pins0;
#ifdef AO_PAD_PORT_1
	AO_PORT_TYPE	pins1;
#endif

	for (;;) {
		while (!ao_pad_ignite)
			ao_sleep(&ao_pad_ignite);
		/*
		 * Actually set the pad bits
		 */
		pins0 = 0;
#ifdef AO_PAD_PORT_1
		pins1 = 0;
#endif
#if AO_PAD_NUM > 0
		if (ao_pad_ignite & (1 << 0))
			pins_pad(0) |= (1 << AO_PAD_PIN_0);
#endif
#if AO_PAD_NUM > 1
		if (ao_pad_ignite & (1 << 1))
			pins_pad(1) |= (1 << AO_PAD_PIN_1);
#endif
#if AO_PAD_NUM > 2
		if (ao_pad_ignite & (1 << 2))
			pins_pad(2) |= (1 << AO_PAD_PIN_2);
#endif
#if AO_PAD_NUM > 3
		if (ao_pad_ignite & (1 << 3))
			pins_pad(3) |= (1 << AO_PAD_PIN_3);
#endif
#if AO_PAD_NUM > 4
		if (ao_pad_ignite & (1 << 4))
			pins_pad(4) |= (1 << AO_PAD_PIN_4);
#endif
#if AO_PAD_NUM > 5
		if (ao_pad_ignite & (1 << 5))
			pins_pad(5) |= (1 << AO_PAD_PIN_5);
#endif
#if AO_PAD_NUM > 6
		if (ao_pad_ignite & (1 << 6))
			pins_pad(6) |= (1 << AO_PAD_PIN_6);
#endif
#if AO_PAD_NUM > 7
		if (ao_pad_ignite & (1 << 7))
			pins_pad(7) |= (1 << AO_PAD_PIN_7);
#endif
#ifdef AO_PAD_PORT_1
		PRINTD("ignite pins 0x%x 0x%x\n", pins0, pins1);
		ao_gpio_set_bits(AO_PAD_PORT_0, pins0);
		ao_gpio_set_bits(AO_PAD_PORT_1, pins1);
#else
		PRINTD("ignite pins 0x%x\n", pins0);
		ao_gpio_set_bits(AO_PAD_PORT_0, pins0);
#endif
		while (ao_pad_ignite) {
			ao_pad_ignite = 0;

			ao_delay(AO_PAD_FIRE_TIME);
		}
#ifdef AO_PAD_PORT_1
		ao_gpio_clr_bits(AO_PAD_PORT_0, pins0);
		ao_gpio_clr_bits(AO_PAD_PORT_1, pins1);
		PRINTD("turn off pins 0x%x 0x%x\n", pins0, pins1);
#else
		ao_gpio_set_bits(AO_PAD_PORT_0, pins0);
		PRINTD("turn off pins 0x%x\n", pins0);
#endif
	}
}

#define AO_PAD_ARM_SIREN_INTERVAL	200

/* Resistor values needed for various voltage test ratios:
 *
 *	Net names involved:
 *
 *	V_BATT		Battery power, after the initial power switch
 *	V_PYRO		Pyro power, after the pyro power switch (and initial power switch)
 *	PYRO_SENSE	ADC input to sense V_PYRO voltage
 *	BATT_SENSE	ADC input to sense V_BATT voltage
 *	IGNITER		FET output to pad (the other pad lead hooks to V_PYRO)
 *	IGNITER_SENSE	ADC input to sense igniter voltage
 *
 *	AO_PAD_R_V_BATT_BATT_SENSE	Resistor from battery rail to battery sense input
 *	AO_PAD_R_BATT_SENSE_GND		Resistor from battery sense input to ground
 *
 *	AO_PAD_R_V_BATT_V_PYRO		Resistor from battery rail to pyro rail
 *	AO_PAD_R_V_PYRO_PYRO_SENSE	Resistor from pyro rail to pyro sense input
 *	AO_PAD_R_PYRO_SENSE_GND		Resistor from pyro sense input to ground
 *
 *	AO_PAD_R_V_PYRO_IGNITER		Optional resistors from pyro rail to FET igniter output
 *	AO_PAD_R_IGNITER_IGNITER_SENSE	Resistors from FET igniter output to igniter sense ADC inputs
 *	AO_PAD_R_IGNITER_SENSE_GND	Resistors from igniter sense ADC inputs to ground
 */

static int16_t
ao_pad_decivolt(int16_t adc, int16_t r_plus, int16_t r_minus)
{
	int32_t	mul = (int32_t) AO_ADC_REFERENCE_DV * (r_plus + r_minus);
	int32_t div = (int32_t) AO_ADC_MAX * r_minus;
	return (int16_t) (((int32_t) adc * mul + mul/2) / div);
}

static void
ao_pad_monitor(void)
{
	uint8_t			c;
	uint8_t			sample;
	AO_LED_TYPE	prev = 0, cur = 0;
	uint8_t		beeping = 0;
	volatile struct ao_data	*packet;
	uint16_t	arm_beep_time = 0;

	sample = ao_data_head;
	ao_led_set(LEDS_AVAILABLE);
	ao_delay(AO_MS_TO_TICKS(1000));
	ao_led_set(0);
	for (;;) {
		int16_t			pyro;

		ao_arch_critical(
			while (sample == ao_data_head)
				ao_sleep((void *) &ao_data_head);
			);


		packet = &ao_data_ring[sample];
		sample = ao_data_ring_next(sample);

		/* Reply battery voltage */
		query.battery = (uint8_t) ao_pad_decivolt(packet->adc.batt, AO_PAD_R_V_BATT_BATT_SENSE, AO_PAD_R_BATT_SENSE_GND);

		/* Current pyro voltage */
		pyro = ao_pad_decivolt(packet->adc.pyro,
				       AO_PAD_R_V_PYRO_PYRO_SENSE,
				       AO_PAD_R_PYRO_SENSE_GND);

		cur = 0;
		if (pyro > query.battery * 7 / 8) {
			query.arm_status = AO_PAD_ARM_STATUS_ARMED;
			cur |= AO_LED_ARMED;
		} else {
			query.arm_status = AO_PAD_ARM_STATUS_DISARMED;
			arm_beep_time = 0;
		}
		if ((ao_time() - ao_pad_packet_time) > AO_SEC_TO_TICKS(2))
			cur |= AO_LED_RED;
		else if (ao_radio_cmac_rssi < AO_PAD_RSSI_MINIMUM)
			cur |= AO_LED_AMBER;
		else
			cur |= AO_LED_GREEN;

		for (c = 0; c < AO_PAD_NUM; c++) {
			int16_t		sense = ao_pad_decivolt(packet->adc.sense[c],
								AO_PAD_R_IGNITER_IGNITER_SENSE,
								AO_PAD_R_IGNITER_SENSE_GND);
			uint8_t	status = AO_PAD_IGNITER_STATUS_UNKNOWN;

			/*
			 *	Here's the resistor stack on each
			 *	igniter channel. Note that
			 *	AO_PAD_R_V_PYRO_IGNITER is optional
			 *
			 *					v_pyro \
			 *	AO_PAD_R_V_PYRO_IGNITER			igniter
			 *					output /
			 *	AO_PAD_R_IGNITER_IGNITER_SENSE         \
			 *					sense   relay
			 *	AO_PAD_R_IGNITER_SENSE_GND	       /
			 *					gnd ---
			 *
			 */

#ifdef AO_PAD_R_V_PYRO_IGNITER
			if (sense <= pyro / 8) {
				/* close to zero → relay is closed */
				status = AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_CLOSED;
				if ((ao_time() % 100) < 50)
					cur |= AO_LED_CONTINUITY(c);
			}
			else
#endif
			{
				if (sense >= (pyro * 7) / 8) {

					/* sense close to pyro voltage; igniter is good
					 */
					status = AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN;
					cur |= AO_LED_CONTINUITY(c);
				} else {

					/* relay not shorted (if we can tell),
					 * and igniter not obviously present
					 */
					status = AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN;
				}
			}
			query.igniter_status[c] = status;
		}
		if (cur != prev) {
			PRINTD("change leds from %02x to %02x\n",
			       prev, cur);
			FLUSHD();
			ao_led_set(cur);
			prev = cur;
		}

		if (ao_pad_armed && (AO_TICK_SIGNED) (ao_time() - ao_pad_arm_time) > (AO_TICK_SIGNED) AO_PAD_ARM_TIME)
			ao_pad_armed = 0;

		if (ao_pad_armed) {
			ao_strobe(1);
			ao_siren(1);
			beeping = 1;
		} else if (query.arm_status == AO_PAD_ARM_STATUS_ARMED && !beeping) {
			if (arm_beep_time == 0) {
				arm_beep_time = AO_PAD_ARM_SIREN_INTERVAL;
				beeping = 1;
				ao_siren(1);
			}
			--arm_beep_time;
		} else if (beeping) {
			beeping = 0;
			ao_siren(0);
			ao_strobe(0);
		}
	}
}

void
ao_pad_disable(void)
{
	if (!ao_pad_disabled) {
		ao_pad_disabled = 1;
		ao_radio_recv_abort();
	}
}

void
ao_pad_enable(void)
{
	ao_pad_disabled = 0;
	ao_wakeup (&ao_pad_disabled);
}

#if HAS_74HC165
static uint8_t
ao_pad_read_box(void)
{
	uint8_t		byte = ao_74hc165_read();
	uint8_t		h, l;

	h = byte >> 4;
	l = byte & 0xf;
	return h * 10 + l;
}
#endif

#ifdef AO_PAD_SELECTOR_PORT
static uint8_t ao_pad_read_box(void) {
	AO_PORT_TYPE	value = ao_gpio_get_all(AO_PAD_SELECTOR_PORT);
	unsigned	pin;
	uint8_t		select = 1;

	for (pin = 0; pin < sizeof (AO_PORT_TYPE) * 8; pin++) {
		if (AO_PAD_SELECTOR_PINS & (1 << pin)) {
			if ((value & (1 << pin)) == 0)
				return select;
			select++;
		}
	}
	return ao_config.pad_box;
}
#else

#if HAS_FIXED_PAD_BOX
#define ao_pad_read_box()	ao_config.pad_box
#endif

#ifdef PAD_BOX
#define ao_pad_read_box()	PAD_BOX
#endif

#endif

static void
ao_pad(void)
{
	int16_t	tick_difference;
	int8_t	ret;

	ao_pad_box = 0;
	for (;;) {
		FLUSHD();
		while (ao_pad_disabled)
			ao_sleep(&ao_pad_disabled);
		ret = ao_radio_cmac_recv(&command, sizeof (command), 0);
		PRINTD ("receive packet status %d rssi %d\n", ret, ao_radio_cmac_rssi);
		if (ret != AO_RADIO_CMAC_OK)
			continue;
		ao_pad_packet_time = ao_time();

		ao_pad_box = ao_pad_read_box();

		PRINTD ("tick %d box %d (me %d) cmd %d channels %02x\n",
			command.tick, command.box, ao_pad_box, command.cmd, command.channels);

		switch (command.cmd) {
		case AO_PAD_ARM:
			if (command.box != ao_pad_box) {
				PRINTD ("box number mismatch\n");
				break;
			}

			if (command.channels & ~(AO_PAD_ALL_CHANNELS))
				break;

			tick_difference = (int16_t) (command.tick - (uint16_t) ao_time());
			PRINTD ("arm tick %d local tick %d\n", command.tick, (uint16_t) ao_time());
			if (tick_difference < 0)
				tick_difference = -tick_difference;
			if (tick_difference > 10) {
				PRINTD ("tick difference too large %d\n", tick_difference);
				break;
			}
			if (query.arm_status != AO_PAD_ARM_STATUS_ARMED) {
				PRINTD ("box not armed locally\n");
				break;
			}
			PRINTD ("armed\n");
			ao_pad_armed = command.channels;
			ao_pad_arm_time = ao_time();
			break;

		case AO_PAD_QUERY:
			if (command.box != ao_pad_box) {
				PRINTD ("box number mismatch\n");
				break;
			}

			query.tick = (uint16_t) ao_time();
			query.box = ao_pad_box;
			query.channels = AO_PAD_ALL_CHANNELS;
			query.armed = ao_pad_armed;
			PRINTD ("query tick %d box %d channels %02x arm %d arm_status %d igniter %d,%d,%d,%d\n",
				query.tick, query.box, query.channels, query.armed,
				query.arm_status,
				query.igniter_status[0],
				query.igniter_status[1],
				query.igniter_status[2],
				query.igniter_status[3]);
			ao_radio_cmac_send(&query, sizeof (query));
			break;
		case AO_PAD_FIRE:
			if (!ao_pad_armed) {
				PRINTD ("not armed\n");
				break;
			}
			if ((AO_TICK_SIGNED) (ao_time() - ao_pad_arm_time) > (AO_TICK_SIGNED) AO_SEC_TO_TICKS(20)) {
				PRINTD ("late pad arm_time %ld time %ld\n",
					(long) ao_pad_arm_time, ao_time());
				break;
			}
			PRINTD ("ignite\n");
			ao_pad_ignite = ao_pad_armed;
			ao_pad_arm_time = ao_time();
			ao_wakeup(&ao_pad_ignite);
			break;
		case AO_PAD_STATIC:
			if (!ao_pad_armed) {
				PRINTD ("not armed\n");
				break;
			}
#if HAS_LOG
			if (!ao_log_running) ao_log_start();
#endif
			if ((AO_TICK_SIGNED) (ao_time() - ao_pad_arm_time) > (AO_TICK_SIGNED) AO_SEC_TO_TICKS(20)) {
				PRINTD ("late pad arm_time %ld time %ld\n",
					(long) ao_pad_arm_time, (long) ao_time());
				break;
			}
			PRINTD ("ignite\n");
			ao_pad_ignite = ao_pad_armed;
			ao_pad_arm_time = ao_time();
			ao_wakeup(&ao_pad_ignite);
			break;
		case AO_PAD_ENDSTATIC:
#if HAS_LOG
			ao_log_stop();
#endif
			break;
		}
	}
}

static void
ao_pad_test(void)
{
	uint8_t	c;

	printf ("Arm switch: ");
	switch (query.arm_status) {
	case AO_PAD_ARM_STATUS_ARMED:
		printf ("Armed\n");
		break;
	case AO_PAD_ARM_STATUS_DISARMED:
		printf ("Disarmed\n");
		break;
	case AO_PAD_ARM_STATUS_UNKNOWN:
		printf ("Unknown\n");
		break;
	}

	for (c = 0; c < AO_PAD_NUM; c++) {
		printf ("Pad %d: ", c);
		switch (query.igniter_status[c]) {
		case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_CLOSED:	printf ("No igniter. Relay closed\n"); break;
		case AO_PAD_IGNITER_STATUS_NO_IGNITER_RELAY_OPEN:	printf ("No igniter. Relay open\n"); break;
		case AO_PAD_IGNITER_STATUS_GOOD_IGNITER_RELAY_OPEN:	printf ("Good igniter. Relay open\n"); break;
		case AO_PAD_IGNITER_STATUS_UNKNOWN:			printf ("Unknown\n"); break;
		}
	}
}

static void
ao_pad_manual(void)
{
	uint8_t	ignite;
	int	repeat;
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ignite = 1 << ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	repeat = (uint8_t) ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success) {
		repeat = 1;
		ao_cmd_status = ao_cmd_success;
	}
	while (repeat-- > 0) {
		ao_pad_ignite = ignite;
		ao_wakeup(&ao_pad_ignite);
		ao_delay(AO_PAD_FIRE_TIME>>1);
	}
}

static struct ao_task ao_pad_task;
static struct ao_task ao_pad_ignite_task;
static struct ao_task ao_pad_monitor_task;

#if DEBUG
static void
ao_pad_set_debug(void)
{
	uint32_t r = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success)
		ao_pad_debug = r != 0;
}


static void
ao_pad_alarm_debug(void)
{
	uint8_t	which, value;
	which = ao_cmd_decimal() != 0;
	if (ao_cmd_status != ao_cmd_success)
		return;
	value = ao_cmd_decimal() != 0;
	if (ao_cmd_status != ao_cmd_success)
		return;
	printf ("Set %s to %d\n", which ? "siren" : "strobe", value);
	if (which)
		ao_siren(value);
	else
		ao_strobe(value);
}
#endif

const struct ao_cmds ao_pad_cmds[] = {
	{ ao_pad_test,	"t\0Test pad continuity" },
	{ ao_pad_manual,	"i <key> <n>\0Fire igniter. <key> is doit with D&I" },
#if DEBUG
	{ ao_pad_set_debug,	"D <0 off, 1 on>\0Debug" },
	{ ao_pad_alarm_debug,	"S <0 strobe, 1 siren> <0 off, 1 on>\0Set alarm output" },
#endif
	{ 0, NULL }
};

#ifndef AO_PAD_PORT_1
#define AO_PAD_0_PORT	AO_PAD_PORT
#define AO_PAD_1_PORT	AO_PAD_PORT
#define AO_PAD_2_PORT	AO_PAD_PORT
#define AO_PAD_3_PORT	AO_PAD_PORT
#define AO_PAD_4_PORT	AO_PAD_PORT
#define AO_PAD_5_PORT	AO_PAD_PORT
#define AO_PAD_6_PORT	AO_PAD_PORT
#define AO_PAD_7_PORT	AO_PAD_PORT
#endif

void
ao_pad_init(void)
{
#ifdef AO_PAD_SELECTOR_PORT
	int pin;

	for (pin = 0; pin < (int) sizeof (AO_PORT_TYPE) * 8; pin++) {
		if (AO_PAD_SELECTOR_PINS & (1 << pin))
			ao_enable_input(AO_PAD_SELECTOR_PORT, pin, AO_EXTI_MODE_PULL_UP);
	}
#endif
#if AO_PAD_NUM > 0
	ao_enable_output(AO_PAD_0_PORT, AO_PAD_PIN_0, 0);
#endif
#if AO_PAD_NUM > 1
	ao_enable_output(AO_PAD_1_PORT, AO_PAD_PIN_1, 0);
#endif
#if AO_PAD_NUM > 2
	ao_enable_output(AO_PAD_2_PORT, AO_PAD_PIN_2, 0);
#endif
#if AO_PAD_NUM > 3
	ao_enable_output(AO_PAD_3_PORT, AO_PAD_PIN_3, 0);
#endif
#if AO_PAD_NUM > 4
	ao_enable_output(AO_PAD_4_PORT, AO_PAD_PIN_4, 0);
#endif
#if AO_PAD_NUM > 5
	ao_enable_output(AO_PAD_5_PORT, AO_PAD_PIN_5, 0);
#endif
#if AO_PAD_NUM > 5
	ao_enable_output(AO_PAD_6_PORT, AO_PAD_PIN_6, 0);
#endif
#if AO_PAD_NUM > 7
	ao_enable_output(AO_PAD_7_PORT, AO_PAD_PIN_7, 0);
#endif
#ifdef AO_STROBE
	ao_enable_output(AO_STROBE_PORT, AO_STROBE_PIN, 0);
#endif
#ifdef AO_SIREN
	ao_enable_output(AO_SIREN_PORT, AO_SIREN_PIN, 0);
#endif
	ao_cmd_register(&ao_pad_cmds[0]);
	ao_add_task(&ao_pad_task, ao_pad, "pad listener");
	ao_add_task(&ao_pad_ignite_task, ao_pad_run, "pad igniter");
	ao_add_task(&ao_pad_monitor_task, ao_pad_monitor, "pad monitor");
}
