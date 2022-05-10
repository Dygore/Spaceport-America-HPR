/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
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
#ifdef AO_BTM_INT_PORT
#include <ao_exti.h>
#endif

#ifndef ao_serial_btm_getchar
#define ao_serial_btm_putchar	ao_serial1_putchar
#define _ao_serial_btm_pollchar	_ao_serial1_pollchar
#define _ao_serial_btm_sleep_for(timeout)	ao_sleep_for((void *) &ao_serial1_rx_fifo, timeout)
#define ao_serial_btm_set_speed ao_serial1_set_speed
#define ao_serial_btm_drain	ao_serial1_drain
#endif

uint8_t		ao_btm_stdio;
uint8_t		ao_btm_connected;

#define BT_DEBUG 0

#if BT_DEBUG
char		ao_btm_buffer[256];
uint16_t		ao_btm_ptr;
char			ao_btm_dir;

static void
ao_btm_add_char(char c)
{
	if (ao_btm_ptr < sizeof (ao_btm_buffer))
		ao_btm_buffer[ao_btm_ptr++] = c;
}

static void
ao_btm_log_char(char c, char dir)
{
	if (dir != ao_btm_dir) {
		ao_btm_add_char(dir);
		ao_btm_dir = dir;
	}
	ao_btm_add_char(c);
}

static void
ao_btm_log_out_char(char c)
{
	ao_btm_log_char(c, '>');
}

static void
ao_btm_log_in_char(char c)
{
	ao_btm_log_char(c, '<');
}

/*
 * Dump everything received from the bluetooth device during startup
 */
static void
ao_btm_dump(void)
{
	int i;
	char c;
	uint16_t r;

	for (i = 0; i < ao_btm_ptr; i++) {
		c = ao_btm_buffer[i];
		if (c < ' ' && c != '\n')
			printf("\\%03o", ((int) c) & 0xff);
		else
			putchar(ao_btm_buffer[i]);
	}
	putchar('\n');
	r = ao_cmd_decimal();
	if (ao_cmd_status == ao_cmd_success && r)
		ao_btm_ptr = 0;
	ao_cmd_status = ao_cmd_success;
}

static void
ao_btm_speed(void)
{
	switch (ao_cmd_decimal()) {
	case 57600:
		ao_serial_btm_set_speed(AO_SERIAL_SPEED_57600);
		break;
	case 19200:
		ao_serial_btm_set_speed(AO_SERIAL_SPEED_19200);
		break;
	default:
		ao_cmd_status = ao_cmd_syntax_error;
		break;
	}
}

static uint8_t	ao_btm_enable;

static void
ao_btm_do_echo(void)
{
	int	c;
	while (ao_btm_enable) {
		ao_arch_block_interrupts();
		while ((c = _ao_serial_btm_pollchar()) == AO_READ_AGAIN && ao_btm_enable)
			_ao_serial_btm_sleep_for(0);
		ao_arch_release_interrupts();
		if (c != AO_READ_AGAIN) {
			putchar(c);
			flush();
		}
	}
	ao_exit();
}

static struct ao_task ao_btm_echo_task;

static void
ao_btm_send(void)
{
	int c;
	ao_btm_enable = 1;
	ao_add_task(&ao_btm_echo_task, ao_btm_do_echo, "btm-echo");
	while ((c = getchar()) != '~') {
		ao_serial_btm_putchar(c);
	}
	ao_btm_enable = 0;
	ao_wakeup((void *) &ao_serial_btm_rx_fifo);
}

const struct ao_cmds ao_btm_cmds[] = {
	{ ao_btm_dump,		"d\0Dump btm buffer." },
	{ ao_btm_speed,		"s <19200,57600>\0Set btm serial speed." },
	{ ao_btm_send,		"S\0BTM interactive mode. ~ to exit." },
	{ 0, NULL },
};

#define ao_btm_log_init()	ao_cmd_register(&ao_btm_cmds[0])

#else
#define ao_btm_log_in_char(c)
#define ao_btm_log_out_char(c)
#define ao_btm_log_init()
#endif

#define AO_BTM_MAX_REPLY	16
char		ao_btm_reply[AO_BTM_MAX_REPLY];

/*
 * Read one bluetooth character.
 * Returns AO_READ_AGAIN if no character arrives within 10ms
 */

static int
ao_btm_getchar(void)
{
	int	c;

	ao_arch_block_interrupts();
	while ((c = _ao_serial_btm_pollchar()) == AO_READ_AGAIN) {
		c = _ao_serial_btm_sleep_for(AO_MS_TO_TICKS(10));
		if (c) {
			c = AO_READ_AGAIN;
			break;
		}
	}
	ao_arch_release_interrupts();
	return c;
}

/*
 * Read a line of data from the serial port, truncating
 * it after a few characters.
 */

static uint8_t
ao_btm_get_line(void)
{
	uint8_t ao_btm_reply_len = 0;
	int c;
	uint8_t l;

	while ((c = ao_btm_getchar()) != AO_READ_AGAIN) {
		ao_btm_log_in_char(c);
		if (ao_btm_reply_len < sizeof (ao_btm_reply))
			ao_btm_reply[ao_btm_reply_len++] = (char) c;
		if (c == '\r' || c == '\n')
			break;
	}
	for (l = ao_btm_reply_len; l < sizeof (ao_btm_reply);)
		ao_btm_reply[l++] = '\0';
	return ao_btm_reply_len;
}

/*
 * Drain the serial port completely
 */
static void
ao_btm_drain(void)
{
	while (ao_btm_get_line())
		;
}

/*
 * Set the stdio echo for the bluetooth link
 */
static void
ao_btm_echo(uint8_t echo)
{
	ao_stdios[ao_btm_stdio].echo = echo;
}

/*
 * Delay between command charaters; the BT module
 * can't keep up with 57600 baud
 */

static void
ao_btm_putchar(char c)
{
	ao_btm_log_out_char(c);
	ao_serial_btm_putchar(c);
	ao_delay(1);
}

/*
 * Wait for the bluetooth device to return
 * status from the previously executed command
 */
static int
ao_btm_wait_reply(void)
{
	for (;;) {
		ao_btm_get_line();
		if (!strncmp(ao_btm_reply, "OK", 2))
			return 1;
		if (!strncmp(ao_btm_reply, "ERROR", 5))
			return -1;
		if (ao_btm_reply[0] == '\0')
			return 0;
	}
}

static void
ao_btm_string(const char *cmd)
{
	char	c;

	while ((c = *cmd++) != '\0')
		ao_btm_putchar(c);
}

static int
ao_btm_cmd(const char *cmd)
{
	ao_btm_drain();

#ifdef AO_BTM_INT_PORT
	/* Trust that AltosDroid will eventually disconnect and let us
	 * get things set up. The BTM module doesn't appear to listen
	 * for +++, so we have no way to force a disconnect.
	 */
	while (ao_btm_connected)
		ao_sleep(&ao_btm_connected);
#endif
	ao_btm_string(cmd);
	return ao_btm_wait_reply();
}

static int
ao_btm_set_name(void)
{
	char	sn[8];
	char	*s = sn + 8;
	char	c;
	int	n;
	ao_btm_string("ATN=TeleBT-");
	*--s = '\0';
	*--s = '\r';
	n = ao_serial_number;
	do {
		*--s = (uint8_t) ('0' + n % 10);
	} while (n /= 10);
	while ((c = *s++))
		ao_btm_putchar(c);
	return ao_btm_wait_reply();
}

static uint8_t
ao_btm_try_speed(uint8_t speed)
{
	ao_serial_btm_set_speed(speed);
	ao_serial_btm_drain();
	(void) ao_btm_cmd("\rATE0\rATQ0\r");
	if (ao_btm_cmd("AT\r") == 1)
		return 1;
	return 0;
}

#if BT_LINK_ON_P2
#define BT_PICTL_ICON	PICTL_P2ICON
#define BT_PIFG		P2IFG
#define BT_PDIR		P2DIR
#define BT_PINP		P2INP
#define BT_IEN2_PIE	IEN2_P2IE
#define BT_CC1111	1
#endif
#if BT_LINK_ON_P1
#define BT_PICTL_ICON	PICTL_P1ICON
#define BT_PIFG		P1IFG
#define BT_PDIR		P1DIR
#define BT_PINP		P1INP
#define BT_IEN2_PIE	IEN2_P1IE
#define BT_CC1111	1
#endif

static void
ao_btm_check_link(void)
{
#if BT_CC1111
	ao_arch_critical(
		/* Check the pin and configure the interrupt detector to wait for the
		 * pin to flip the other way
		 */
		if (BT_LINK_PIN) {
			ao_btm_connected = 0;
			PICTL |= BT_PICTL_ICON;
		} else {
			ao_btm_connected = 1;
			PICTL &= ~BT_PICTL_ICON;
		}
		);
#else
	ao_arch_block_interrupts();
	if (ao_gpio_get(AO_BTM_INT_PORT, AO_BTM_INT_PIN) == 0) {
		ao_btm_connected = 1;
	} else {
		ao_btm_connected = 0;
	}
	ao_arch_release_interrupts();
#endif
}

struct ao_task ao_btm_task;

/*
 * A thread to initialize the bluetooth device and
 * hang around to blink the LED when connected
 */
static void
ao_btm(void)
{
#ifdef AO_BTM_INT_PORT
	ao_exti_enable(AO_BTM_INT_PORT, AO_BTM_INT_PIN);
#endif

	/*
	 * Wait for the bluetooth device to boot
	 */
	ao_delay(AO_SEC_TO_TICKS(3));

	/*
	 * The first time we connect, the BTM-180 comes up at 19200 baud.
	 * After that, it will remember and come up at 57600 baud. So, see
	 * if it is already running at 57600 baud, and if that doesn't work
	 * then tell it to switch to 57600 from 19200 baud.
	 */
	while (!ao_btm_try_speed(AO_SERIAL_SPEED_57600)) {
		ao_delay(AO_SEC_TO_TICKS(1));
		if (ao_btm_try_speed(AO_SERIAL_SPEED_19200))
			ao_btm_cmd("ATL4\r");
		ao_delay(AO_SEC_TO_TICKS(1));
	}

	/* Disable echo */
	ao_btm_cmd("ATE0\r");

	/* Enable flow control */
	ao_btm_cmd("ATC1\r");

	/* Set the reported name to something we can find on the host */
	ao_btm_set_name();

	/* Turn off status reporting */
	ao_btm_cmd("ATQ1\r");

	ao_btm_drain();

	ao_btm_stdio = ao_add_stdio(_ao_serial_btm_pollchar,
				    ao_serial_btm_putchar,
				    NULL);
	ao_btm_echo(0);

	/* Check current pin state */
	ao_btm_check_link();

	for (;;) {
		while (!ao_btm_connected)
			ao_sleep(&ao_btm_connected);
		while (ao_btm_connected) {
			ao_led_for(AO_BT_LED, AO_MS_TO_TICKS(20));
			ao_delay(AO_SEC_TO_TICKS(3));
		}
	}
}


#if BT_CC1111
void
ao_btm_isr(void)
#if BT_LINK_ON_P1
	__interrupt 15
#endif
{
#if BT_LINK_ON_P1
	P1IF = 0;
#endif
	if (BT_PIFG & (1 << BT_LINK_PIN_INDEX)) {
		ao_btm_check_link();
		ao_wakeup(&ao_btm_connected);
	}
	BT_PIFG = 0;
}
#endif

#ifdef AO_BTM_INT_PORT
void
ao_btm_isr(void)
{
	ao_btm_check_link();
	ao_wakeup(&ao_btm_connected);
}
#endif

void
ao_btm_init (void)
{
	ao_serial_init();

	ao_serial_btm_set_speed(AO_SERIAL_SPEED_19200);

#ifdef AO_BTM_RESET_PORT
	ao_enable_output(AO_BTM_RESET_PORT,AO_BTM_RESET_PIN,0);
#endif

#ifdef AO_BTM_INT_PORT
	ao_enable_port(AO_BTM_INT_PORT);
	ao_exti_setup(AO_BTM_INT_PORT, AO_BTM_INT_PIN,
		      AO_EXTI_MODE_FALLING|AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_LOW,
		      ao_btm_isr);
#endif

#if BT_CC1111
#if BT_LINK_ON_P1
	/*
	 * Configure ser reset line
	 */

	P1_6 = 0;
	P1DIR |= (1 << 6);
#endif

	/*
	 * Configure link status line
	 */

	/* Set pin to input */
	BT_PDIR &= ~(1 << BT_LINK_PIN_INDEX);

	/* Set pin to tri-state */
	BT_PINP |= (1 << BT_LINK_PIN_INDEX);

	/* Enable interrupts */
	IEN2 |= BT_IEN2_PIE;
#endif

#if BT_LINK_ON_P2
	/* Eable the pin interrupt */
	PICTL |= PICTL_P2IEN;
#endif
#if BT_LINK_ON_P1
	/* Enable pin interrupt */
	P1IEN |= (1 << BT_LINK_PIN_INDEX);
#endif

	ao_add_task(&ao_btm_task, ao_btm, "bt");
	ao_btm_log_init();
}
