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
#include <ao_rn4678.h>
#include <ao_exti.h>
#include <stdarg.h>

static uint8_t	ao_rn_connected;

#define AO_RN_DEBUG 0

#if AO_RN_DEBUG
static void ao_rn_dbg(char *format, ...) {
	va_list a;
	uint32_t	irq = ao_arch_irqsave();
	ao_arch_release_interrupts();
	va_start(a, format);
	vprintf(format, a);
	va_end(a);
	flush();
	ao_arch_irqrestore(irq);
}

static char	ao_rn_dir;

static void
ao_rn_log_char(char c, char dir)
{
	if (dir != ao_rn_dir) {
		putchar(dir); putchar('\n');
		ao_rn_dir = dir;
	}
	switch (c) {
	case '\r':
		putchar('\\'); putchar('r');
		break;
	case '\n':
		putchar('\\'); putchar('n');
		break;
	default:
		putchar(c);
	}
	flush();
}

static void
ao_rn_log_out_char(char c)
{
	ao_rn_log_char(c, '}');
}

static void
ao_rn_log_in_char(char c)
{
	ao_rn_log_char(c, '{');
}

static inline void
ao_rn_putchar(char c)
{
	ao_rn_log_out_char(c);
	ao_serial_rn_putchar(c);
}

static inline int
_ao_rn_pollchar(void)
{
	int	c = _ao_serial_rn_pollchar();

	if (c != AO_READ_AGAIN) {
		ao_arch_release_interrupts();
		ao_rn_log_in_char((char) c);
		ao_arch_block_interrupts();
	}
	return c;
}
#else
#define ao_rn_dbg(fmt, ...)
#define ao_rn_putchar(c)	ao_serial_rn_putchar(c)
#define _ao_rn_pollchar()	_ao_serial_rn_pollchar()
#endif

/* For stdio, this skips all status messages *sigh* */

#define STATUS_CHAR	'%'

static const char *status_strings[] = {
	"RFCOMM_CLOSE",
	"RFCOMM_OPEN",
	"CONNECT",
	"LCONNECT",
	"DISCONN",
	"BONDED",
};

#define NUM_STATUS_STRING	(sizeof status_strings/sizeof status_strings[0])

static char		ao_rn_buffer[64];
static size_t		ao_rn_buf_cnt, ao_rn_buf_ptr;
static int		ao_rn_draining;
static AO_TICK_TYPE	ao_rn_buf_time;

/* Well, this is annoying. The status strings from the RN4678 can't be
 * disabled due to a firmware bug. So, this code finds those in the
 * input and strips them out.
 */
static int
_ao_wrap_rn_pollchar(void)
{
	int		c = AO_READ_AGAIN;
	unsigned	i;
	int		done = 0;

	while (!done && !ao_rn_draining) {
		c = _ao_serial_rn_pollchar();

		if (c == AO_READ_AGAIN) {
			if (ao_rn_buf_cnt && (ao_time() - ao_rn_buf_time) > AO_MS_TO_TICKS(1000)) {
				ao_rn_draining = 1;
				continue;
			}
			return AO_READ_AGAIN;
		}

		if (ao_rn_buf_cnt) {
			/* buffering chars */

			if (c == STATUS_CHAR) {
				/* End of status string, drop it and carry on */
				ao_rn_buffer[ao_rn_buf_cnt] = '\0';
//				ao_rn_dbg("discard %s\n", ao_rn_buffer);
				ao_rn_buf_cnt = 0;
			} else if (ao_rn_buf_cnt == sizeof(ao_rn_buffer)) {
				/* If we filled the buffer, just give up */
				ao_rn_draining = 1;
			} else {
				ao_rn_buffer[ao_rn_buf_cnt++] = (char) c;
				for (i = 0; i < NUM_STATUS_STRING; i++) {
					size_t cmp = strlen(status_strings[i]);
					if (cmp >= ao_rn_buf_cnt)
						cmp = ao_rn_buf_cnt-1;
					if (memcmp(ao_rn_buffer+1, status_strings[i], cmp) == 0)
						break;
				}
				if (i == NUM_STATUS_STRING)
					ao_rn_draining = 1;
			}
		} else if (c == STATUS_CHAR) {
			ao_rn_buffer[0] = (char) c;
			ao_rn_buf_cnt = 1;
			ao_rn_buf_ptr = 0;
			ao_rn_buf_time = ao_time();
		} else
			done = 1;
	}
	if (ao_rn_draining) {
		c = ao_rn_buffer[ao_rn_buf_ptr++] & 0xff;
		if (ao_rn_buf_ptr == ao_rn_buf_cnt) {
			ao_rn_buf_ptr = ao_rn_buf_cnt = 0;
			ao_rn_draining = 0;
		}
	}
	return c;
}

static void
ao_rn_puts(char *s)
{
	char c;

	while ((c = *s++))
		ao_rn_putchar(c);
}

static void
ao_rn_drain(void)
{
	int	timeout = 0;

//	ao_rn_dbg("drain...\n");
	ao_serial_rn_drain();
	while (!timeout) {
		ao_arch_block_interrupts();
		while (_ao_rn_pollchar() == AO_READ_AGAIN) {
			if (_ao_serial_rn_sleep_for(AO_MS_TO_TICKS(10))) {
				timeout = 1;
				break;
			}
		}
		ao_arch_release_interrupts();
	}
//	ao_rn_dbg("drain done\n");
}

static void
ao_rn_send_cmd(char *cmd, char *param)
{
//	ao_rn_dbg("send_cmd %s%s\n", cmd, param ? param : "");
	ao_rn_drain();
	ao_rn_puts(cmd);
	if (param)
		ao_rn_puts(param);
	ao_rn_putchar('\r');
}

static int
ao_rn_wait_char(AO_TICK_TYPE giveup_time)
{
	int c;

	ao_arch_block_interrupts();
	while ((c = _ao_rn_pollchar()) == AO_READ_AGAIN) {
		AO_TICK_SIGNED	delay = (AO_TICK_SIGNED) (giveup_time - ao_time());
		if (delay < 0) {
			ao_arch_release_interrupts();
			return AO_READ_AGAIN;
		}
		_ao_serial_rn_sleep_for((AO_TICK_TYPE) delay);
	}
	ao_arch_release_interrupts();
	return c;
}

static int
ao_rn_wait_for(AO_TICK_TYPE timeout, char *match)
{
	char		reply[AO_RN_MAX_REPLY_LEN + 1];
	size_t		match_len = strlen(match);
	AO_TICK_TYPE	giveup_time = ao_time() + timeout;
	int		c;

//	ao_rn_dbg("wait for %d, \"%s\"\n", timeout, match);
	memset(reply, ' ', sizeof(reply));
	while (memcmp(reply, match, match_len) != 0) {
		c = ao_rn_wait_char(giveup_time);
		if (c == AO_READ_AGAIN) {
//			ao_rn_dbg("\twait for timeout\n");
			return AO_RN_TIMEOUT;
		}
		reply[match_len] = (char) c;
		memmove(reply, reply+1, match_len);
		reply[match_len] = '\0';
//		ao_rn_dbg("\tmatch now \"%s\"\n", reply);
	}
//	ao_rn_dbg("\twait for ok\n");
	return AO_RN_OK;
}

static int
ao_rn_wait_line(AO_TICK_TYPE giveup_time, char *line, int len)
{
	char *l = line;

//	ao_rn_dbg("wait line\n");
	for (;;) {
		int c = ao_rn_wait_char(giveup_time);

		/* timeout */
		if (c == AO_READ_AGAIN) {
//			ao_rn_dbg("\twait line timeout\n");
			return AO_RN_TIMEOUT;
		}

		/* done */
		if (c == '\r') {
			*l = '\0';
//			ao_rn_dbg("\twait line \"%s\"\n", line);
			return AO_RN_OK;
		}

		if (c == '\n')
			continue;

		/* buffer overrun */
		if (len <= 1)
			return AO_RN_ERROR;

		*l++ = (char) c;
		len--;
	}
}

static int
ao_rn_wait_status(void)
{
	char		message[AO_RN_MAX_REPLY_LEN];
	AO_TICK_TYPE	giveup_time = ao_time() + AO_RN_CMD_TIMEOUT;
	int		status;

//	ao_rn_dbg("wait status\n");
	status = ao_rn_wait_line(giveup_time, message, sizeof (message));
	if (status == AO_RN_OK)
		if (strncmp(message, "AOK", 3) != 0)
			status = AO_RN_ERROR;
	return status;
}

static int
ao_rn_set_name(void)
{
	char	sn[8];
	char	*s = sn + 8;
	int	n;

//	ao_rn_dbg("set name...\n");
	*--s = '\0';
	n = ao_serial_number;
	do {
		*--s = (uint8_t) ('0' + n % 10);
	} while (n /= 10);
	ao_rn_send_cmd(AO_RN_SET_NAME_CMD "TeleBT-", s);
	return ao_rn_wait_status();
}

static int
ao_rn_get_name(char *name, int len)
{
//	ao_rn_dbg("get name...\n");
	ao_rn_send_cmd(AO_RN_GET_NAME_CMD, NULL);
	return ao_rn_wait_line(ao_time() + AO_RN_CMD_TIMEOUT, name, len);
}

static void
ao_rn_check_link(void)
{
	ao_rn_connected = 1 - ao_gpio_get(AO_RN_CONNECTED_PORT, AO_RN_CONNECTED_PIN);
}

static void
ao_rn_isr(void)
{
	ao_rn_check_link();
	ao_wakeup(&ao_rn_connected);
}

static void
ao_bt_panic(int where)
{
	int i;
	for (;;) {
		for (i = 0; i < 25; i++) {
			ao_led_for(AO_BT_LED, AO_MS_TO_TICKS(10));
			ao_delay(AO_MS_TO_TICKS(10));
		}
		ao_delay(AO_MS_TO_TICKS(500));
		for (i = 0; i < where; i++) {
			ao_led_for(AO_BT_LED, AO_MS_TO_TICKS(200));
			ao_delay(AO_MS_TO_TICKS(200));
		}
	}
}

static uint8_t	ao_rn_stdio;

/*
 * Set the stdio echo for the bluetooth link
 */
static void
ao_rn_echo(uint8_t echo)
{
	ao_stdios[ao_rn_stdio].echo = echo;
}

static void
ao_rn(void)
{
	int	status = AO_RN_ERROR;
	char	name[17];
	int	i;

	ao_rn_dbg("ao_rn top\n");

	/* Select CMD mode after the device gets out of reset */
	ao_gpio_set(AO_RN_CMD_PORT, AO_RN_CMD_PIN, AO_RN_CMD_CMD);

	for (i = 0; i < 3; i++) {
		ao_rn_dbg("reset device\n");

		ao_gpio_set(AO_RN_RST_N_PORT, AO_RN_RST_N_PIN, 0);
		ao_delay(AO_MS_TO_TICKS(100));

		/* Reboot the RN4678 and wait for it to start talking */
		ao_rn_drain();
		ao_gpio_set(AO_RN_RST_N_PORT, AO_RN_RST_N_PIN, 1);
		status = ao_rn_wait_for(AO_RN_REBOOT_TIMEOUT, AO_RN_REBOOT_MSG);
		if (status != AO_RN_OK) {
			ao_rn_dbg("reboot failed\n");
			continue;
		}

		/* After it reboots, it can take a moment before it responds
		 * to commands
		 */
		status = ao_rn_wait_for(AO_RN_REBOOT_TIMEOUT, "CMD> ");

		if (status == AO_RN_TIMEOUT) {
			ao_rn_puts("$$$");
			(void) ao_rn_wait_for(AO_RN_REBOOT_TIMEOUT, "CMD> ");
		}

		ao_rn_send_cmd(AO_RN_VERSION_CMD, NULL);
		(void) ao_rn_wait_status();

		/* Check to see if the name is already set and assume
		 * that the device is ready to go
		 */
		status = ao_rn_get_name(name, sizeof (name));
		if (status != AO_RN_OK) {
			ao_rn_dbg("get name failed\n");
			status = ao_rn_get_name(name, sizeof (name));
			if (status != AO_RN_OK)
				continue;
		}

		if (strncmp(name, "TeleBT-", 7) == 0) {
			ao_rn_dbg("name is set\n");
			status = AO_RN_OK;
			break;
		}

		/* Make the command pin control command/data mode */
		ao_rn_send_cmd(AO_RN_SET_COMMAND_PIN, NULL);
		if (ao_rn_wait_status() != AO_RN_OK) {
			ao_rn_dbg("set command pin failed\n");
			continue;
		}

		ao_rn_send_cmd(AO_RN_SET_STATUS_STRING, AO_RN_STATUS_STRING_ENABLE);
		if (ao_rn_wait_status() != AO_RN_OK) {
			ao_rn_dbg("set status string\n");
			continue;
		}

		/* Select 'fast' mode to ignore command sequence (more or less) */
		ao_rn_send_cmd(AO_RN_SET_FAST_MODE, NULL);
		if (ao_rn_wait_status() != AO_RN_OK) {
			ao_rn_dbg("set fast mode failed\n");
			continue;
		}

		/* Finally, set the name. Doing this last makes it possible to check
		 * if the whole sequence has been done
		 */
		if (ao_rn_set_name() != AO_RN_OK) {
			ao_rn_dbg("set name failed\n");
			continue;
		}

		/* After we've configured the device, go back around and reboot it
		 * as that's how we get the new configuration to take effect
		 */
	}
	ao_rn_dbg("ao_rn status %d\n", status);

	if (status != AO_RN_OK)
		ao_bt_panic(4);

	ao_gpio_set(AO_RN_CMD_PORT, AO_RN_CMD_PIN, AO_RN_CMD_DATA);

	/* Wait for the hardware to finish sending messages, then clear the queue */
	ao_delay(AO_MS_TO_TICKS(200));
	ao_rn_drain();

	ao_exti_enable(AO_RN_CONNECTED_PORT, AO_RN_CONNECTED_PIN);

#if AO_RN_DEBUG

	/*
	 * Separate debug code when things aren't working. Just dump
	 * inbound bluetooth characters to stdout
	 */
	for (;;) {
		int	c;

		ao_arch_block_interrupts();
		while ((c = _ao_rn_pollchar()) == AO_READ_AGAIN)
			ao_sleep(&ao_serial_rn_rx_fifo);
		ao_arch_release_interrupts();
	}
#else
	ao_rn_stdio = ao_add_stdio(_ao_wrap_rn_pollchar,
				   ao_serial_rn_putchar,
				   NULL);

	ao_rn_echo(0);
	ao_rn_check_link();
	/*
	 * Now just hang around and flash the blue LED when we've got
	 * a connection
	 */
	for (;;) {
		ao_arch_block_interrupts();
		while (!ao_rn_connected)
			ao_sleep(&ao_rn_connected);
		ao_arch_release_interrupts();
		while (ao_rn_connected) {
			ao_led_for(AO_BT_LED, AO_MS_TO_TICKS(20));
			if (ao_rn_buf_cnt != 0)
				ao_wakeup(&ao_stdin_ready);
			ao_delay(AO_SEC_TO_TICKS(3));
		}
	}
#endif
}

static struct ao_task ao_rn_task;

static void
ao_rn_factory(void)
{
	int	i;
	uint8_t	v = 0;

	/*
	 * Factory reset. Flip pin P3_1 5 times within the first five
	 * seconds of power-on
	 */

	/* Select our target output pin */
	ao_enable_output(AO_RN_P3_1_PORT, AO_RN_P3_1_PIN, v);

	/* Turn off the BT device using the SW_BTN pin */
	printf("Power down BT\n"); flush();
	ao_gpio_set(AO_RN_SW_BTN_PORT, AO_RN_SW_BTN_PIN, 0);
	ao_delay(AO_MS_TO_TICKS(1000));

	/* And turn it back on */
	printf("Power up BT\n"); flush();
	ao_gpio_set(AO_RN_SW_BTN_PORT, AO_RN_SW_BTN_PIN, 1);

	/* Right after power on, poke P3_1 five times to force a
	 * factory reset
	 */
	for (i = 0; i < 20; i++) {
		v = 1-v;
		ao_delay(AO_MS_TO_TICKS(50));
		ao_gpio_set(AO_RN_P3_1_PORT, AO_RN_P3_1_PIN, v);
		if (v)
			ao_led_on(AO_BT_LED);
		else
			ao_led_off(AO_BT_LED);
	}

	/* And let P3_1 float again */
	ao_enable_input(AO_RN_P3_1_PORT, AO_RN_P3_1_PIN, AO_EXTI_MODE_PULL_NONE);

	printf("Reboot BT\n"); flush();
	ao_delay(AO_MS_TO_TICKS(100));
	ao_gpio_set(AO_RN_RST_N_PORT, AO_RN_RST_N_PIN, 0);
	ao_delay(AO_MS_TO_TICKS(100));
	ao_gpio_set(AO_RN_RST_N_PORT, AO_RN_RST_N_PIN, 1);
}

#if AO_RN_DEBUG
static void
ao_rn_send(void)
{
	int	c;

	while ((c = getchar()) != '~')
		ao_rn_putchar(c);
}
#endif

static const struct ao_cmds rn_cmds[] = {
	{ ao_rn_factory, "F\0Factory reset rn4678" },
#if AO_RN_DEBUG
	{ ao_rn_send, "B\0Send data to rn4678. End with ~" },
#endif
	{ 0 },
};

void
ao_rn4678_init(void)
{
	(void) ao_rn_set_name;

	ao_serial_rn_set_speed(AO_SERIAL_SPEED_115200);

	/* Reset line */
	ao_enable_output(AO_RN_RST_N_PORT, AO_RN_RST_N_PIN, 0);

	/* SW_BTN */
	ao_enable_output(AO_RN_SW_BTN_PORT, AO_RN_SW_BTN_PIN, 1);

	/* P3_7 command/data selector */
	ao_enable_output(AO_RN_CMD_PORT, AO_RN_CMD_PIN, AO_RN_CMD_CMD);

	ao_enable_input(AO_RN_CONNECTED_PORT, AO_RN_CONNECTED_PIN, AO_EXTI_MODE_PULL_NONE);
	ao_exti_setup(AO_RN_CONNECTED_PORT, AO_RN_CONNECTED_PIN,
		      AO_EXTI_MODE_FALLING|AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_LOW,
		      ao_rn_isr);

	ao_cmd_register(rn_cmds);
	ao_add_task(&ao_rn_task, ao_rn, "bluetooth");
}
