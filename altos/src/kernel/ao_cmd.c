/*
 * Copyright © 2009 Keith Packard <keithp@keithp.com>
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
#include "ao_task.h"

char	ao_cmd_lex_c;
enum ao_cmd_status ao_cmd_status;

#ifndef AO_CMD_LEN
#if AO_PYRO_NUM
#define AO_CMD_LEN 128
#else
#define AO_CMD_LEN	48
#endif
#endif

static char	cmd_line[AO_CMD_LEN];
static uint8_t	cmd_len;
static uint8_t	cmd_i;

static const char backspace[] = "\010 \010";

void
ao_put_string(const char *s)
{
	char	c;
	while ((c = *s++))
		putchar(c);
}

void
ao_cmd_readline(const char *prompt)
{
	char c;
	if (ao_echo())
		ao_put_string(prompt);
	cmd_len = 0;
	for (;;) {
		flush();
		c = getchar();
		/* backspace/delete */
		if (c == '\010' || c == '\177') {
			if (cmd_len != 0) {
				if (ao_echo())
					ao_put_string(backspace);
				--cmd_len;
			}
			continue;
		}

		/* ^U */
		if (c == '\025') {
			while (cmd_len != 0) {
				if (ao_echo())
					ao_put_string(backspace);
				--cmd_len;
			}
			continue;
		}

		/* map CR to NL */
		if (c == '\r')
			c = '\n';

		if (c == '\n') {
			if (ao_echo())
				putchar('\n');
			break;
		}

		if (cmd_len >= AO_CMD_LEN - 2)
			continue;
		cmd_line[cmd_len++] = c;
		if (ao_echo())
			putchar(c);
	}
	cmd_line[cmd_len++] = '\n';
	cmd_line[cmd_len++] = '\0';
	cmd_i = 0;
}

char
ao_cmd_lex(void)
{
	ao_cmd_lex_c = '\n';
	if (cmd_i < cmd_len)
		ao_cmd_lex_c = cmd_line[cmd_i++];
	return ao_cmd_lex_c;
}

static void
putnibble(uint8_t v)
{
	if (v < 10)
		putchar(v + '0');
	else
		putchar(v + ('a' - 10));
}

uint8_t
ao_getnibble(void)
{
	char	c;

	c = getchar();
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'f')
		return c - ('a' - 10);
	if ('A' <= c && c <= 'F')
		return c - ('A' - 10);
	ao_cmd_status = ao_cmd_lex_error;
	return 0;
}

void
ao_cmd_put16(uint16_t v)
{
	ao_cmd_put8((uint8_t) (v >> 8));
	ao_cmd_put8((uint8_t) v);
}

void
ao_cmd_put8(uint8_t v)
{
	putnibble((v >> 4) & 0xf);
	putnibble(v & 0xf);
}

uint8_t
ao_cmd_is_white(void)
{
	return ao_cmd_lex_c == ' ' || ao_cmd_lex_c == '\t';
}

void
ao_cmd_white(void)
{
	while (ao_cmd_is_white())
		ao_cmd_lex();
}

int8_t
ao_cmd_hexchar(char c)
{
	if ('0' <= c && c <= '9')
		return (int8_t) (c - '0');
	if ('a' <= c && c <= 'f')
		return (int8_t) (c - 'a' + 10);
	if ('A' <= c && c <= 'F')
		return (int8_t) (c - 'A' + 10);
	return -1;
}

static uint32_t
get_hex(uint8_t lim)
{
	uint32_t result = 0;
	uint8_t i;

	ao_cmd_white();
	for (i = 0; i < lim; i++) {
		int8_t n = ao_cmd_hexchar(ao_cmd_lex_c);
		if (n < 0) {
			if (i == 0 || lim != 0xff)
				ao_cmd_status = ao_cmd_lex_error;
			break;
		}
		result = (uint32_t) ((result << 4) | (uint32_t) n);
		ao_cmd_lex();
	}
	return result;
}

uint8_t
ao_cmd_hexbyte(void)
{
	return (uint8_t) get_hex(2);
}

uint32_t
ao_cmd_hex(void)
{
	return get_hex(0xff);
}

uint32_t
ao_cmd_decimal(void) 
{
	uint32_t result = 0;
	uint8_t	r = ao_cmd_lex_error;
	bool negative = false;

	ao_cmd_white();
	if (ao_cmd_lex_c == '-') {
		negative = true;
		ao_cmd_lex();
	}
	for(;;) {
		if ('0' <= ao_cmd_lex_c && ao_cmd_lex_c <= '9')
			result = result * 10 + (ao_cmd_lex_c - '0');
		else
			break;
		r = ao_cmd_success;
		ao_cmd_lex();
	}
	if (r != ao_cmd_success)
		ao_cmd_status = r;
	if (negative)
		result = -result;
	return result;
}

uint8_t
ao_match_word(const char *word)
{
	while (*word) {
		if (ao_cmd_lex_c != *word) {
			ao_cmd_status = ao_cmd_syntax_error;
			return 0;
		}
		word++;
		ao_cmd_lex();
	}
	return 1;
}

static void
echo(void)
{
	uint32_t v = ao_cmd_hex();
	if (ao_cmd_status == ao_cmd_success)
		ao_stdios[ao_cur_stdio].echo = v != 0;
}

static void
ao_reboot(void)
{
	ao_cmd_white();
	if (!ao_match_word("eboot"))
		return;
	/* Delay waiting for the packet master to be turned off
	 * so that we don't end up back in idle mode because we
	 * received a packet after boot.
	 */
	flush();
	ao_delay(AO_SEC_TO_TICKS(1));
	ao_arch_reboot();
	ao_panic(AO_PANIC_REBOOT);
}

#ifndef HAS_VERSION
#define HAS_VERSION 1
#endif

#if HAS_VERSION
#define _stringify(x) #x
#define stringify(x) _stringify(x)
static void
version(void)
{
	printf("manufacturer     %s\n"
	       "product          %s\n"
	       "serial-number    %u\n"
#if HAS_LOG && (HAS_FLIGHT || HAS_TRACKER)
	       "current-flight   %u\n"
#endif
#if HAS_LOG
	       "log-format       %u\n"
#if !DISABLE_LOG_SPACE
	       "log-space	 %lu\n"
#endif
#endif
#if defined(AO_BOOT_APPLICATION_BASE) && defined(AO_BOOT_APPLICATION_BOUND)
	       "program-space    %u\n"
#endif
#if AO_VALUE_32
	       "altitude-32      1\n"
#endif
	       , ao_manufacturer
	       , ao_product
	       , ao_serial_number
#if HAS_LOG && (HAS_FLIGHT || HAS_TRACKER)
	       , ao_flight_number
#endif
#if HAS_LOG
	       , AO_LOG_FORMAT
#if !DISABLE_LOG_SPACE
	       , (unsigned long) ao_storage_log_max
#endif
#endif
#if defined(AO_BOOT_APPLICATION_BASE) && defined(AO_BOOT_APPLICATION_BOUND)
	       , (unsigned) ((uint32_t) AO_BOOT_APPLICATION_BOUND - (uint32_t) AO_BOOT_APPLICATION_BASE)
#endif
		);
	printf("software-version %." stringify(AO_MAX_VERSION) "s\n", ao_version);
}
#endif

#ifndef NUM_CMDS
#define NUM_CMDS	11
#endif

static const struct ao_cmds	*(ao_cmds[NUM_CMDS]);
static uint8_t		ao_ncmds;

static void
help(void)
{
	uint8_t cmds;
	uint8_t cmd;
	const struct ao_cmds * cs;
	const char *h;
	size_t e;

	for (cmds = 0; cmds < ao_ncmds; cmds++) {
		cs = ao_cmds[cmds];
		for (cmd = 0; cs[cmd].func; cmd++) {
			h = cs[cmd].help;
			ao_put_string(h);
			e = strlen(h);
			h += e + 1;
			e = 45 - e;
			while (e--)
				putchar(' ');
			ao_put_string(h);
			putchar('\n');
		}
	}
}

static void
report(void)
{
	switch(ao_cmd_status) {
	case ao_cmd_lex_error:
	case ao_cmd_syntax_error:
		ao_put_string("Syntax error\n");
		ao_cmd_status = 0;
	default:
		break;
	}
}

void
ao_cmd_register(const struct ao_cmds *cmds)
{
	if (ao_ncmds >= NUM_CMDS)
		ao_panic(AO_PANIC_CMD);
	ao_cmds[ao_ncmds++] = cmds;
}

void
ao_cmd(void)
{
	char	c;
	uint8_t cmd, cmds;
	const struct ao_cmds * cs;
	void (*func)(void);

	for (;;) {
		ao_cmd_readline("> ");
		ao_cmd_lex();
		ao_cmd_white();
		c = ao_cmd_lex_c;
		ao_cmd_lex();
		if (c == '\r' || c == '\n')
			continue;
		func = (void (*)(void)) NULL;
		for (cmds = 0; cmds < ao_ncmds; cmds++) {
			cs = ao_cmds[cmds];
			for (cmd = 0; cs[cmd].func; cmd++)
				if (cs[cmd].help[0] == c) {
					func = cs[cmd].func;
					break;
				}
			if (func)
				break;
		}
#if HAS_MONITOR
		ao_mutex_get(&ao_monitoring_mutex);
#endif
		if (func)
			(*func)();
		else
			ao_cmd_status = ao_cmd_syntax_error;
		report();
#if HAS_MONITOR
		ao_mutex_put(&ao_monitoring_mutex);
#endif
	}
}

#if HAS_BOOT_LOADER

#include <ao_boot.h>

static void
ao_loader(void)
{
	flush();
	ao_boot_loader();
}
#endif

#if HAS_TASK
struct ao_task ao_cmd_task;
#endif

const struct ao_cmds	ao_base_cmds[] = {
	{ help,		"?\0Help" },
#if HAS_TASK_INFO && HAS_TASK
	{ ao_task_info,	"T\0Tasks" },
#endif
	{ echo,		"E <0 off, 1 on>\0Echo" },
	{ ao_reboot,	"r eboot\0Reboot" },
#if HAS_VERSION
	{ version,	"v\0Version" },
#endif
#if HAS_BOOT_LOADER
	{ ao_loader,	"X\0Switch to boot loader" },
#endif
	{ 0,	NULL },
};

void
ao_cmd_init(void)
{
	ao_cmd_register(&ao_base_cmds[0]);
#if HAS_TASK
	ao_add_task(&ao_cmd_task, ao_cmd, "cmd");
#endif
}
