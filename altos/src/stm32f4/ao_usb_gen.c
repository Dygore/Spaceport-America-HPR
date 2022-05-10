/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

#include "ao_usb_gen.h"

static uint8_t 	ao_usb_ep0_state;

/* Pending EP0 IN data */
static const uint8_t	*ao_usb_ep0_in_data;	/* Remaining data */
static uint8_t 		ao_usb_ep0_in_len;	/* Remaining amount */

/* Temp buffer for smaller EP0 in data */
static uint8_t	ao_usb_ep0_in_buf[2];

/* Pending EP0 OUT data */
static uint8_t *ao_usb_ep0_out_data;
static uint8_t 	ao_usb_ep0_out_len;

/* System ram shadow of USB buffer; writing individual bytes is
 * too much of a pain (sigh) */
static uint8_t	ao_usb_tx_buffer[AO_USB_IN_SIZE];
static uint8_t	ao_usb_tx_count;

static uint8_t	ao_usb_rx_buffer[AO_USB_OUT_SIZE];
static uint8_t	ao_usb_rx_count, ao_usb_rx_pos;

/*
 * End point register indices
 */

#define AO_USB_CONTROL_EPR	0
#define AO_USB_INT_EPR		1
#define AO_USB_OUT_EPR		2
#define AO_USB_IN_EPR		3

/* Marks when we don't need to send an IN packet.
 * This happens only when the last IN packet is not full,
 * otherwise the host will expect to keep seeing packets.
 * Send a zero-length packet as required
 */
static uint8_t	ao_usb_in_flushed;

/* Marks when we have delivered an IN packet to the hardware
 * and it has not been received yet. ao_sleep on this address
 * to wait for it to be delivered.
 */
static uint8_t	ao_usb_in_pending;

/* Marks when an OUT packet has been received by the hardware
 * but not pulled to the shadow buffer.
 */
static uint8_t	ao_usb_out_avail;
uint8_t		ao_usb_running;
static uint8_t	ao_usb_configuration;

static uint8_t	ao_usb_address;
static uint8_t	ao_usb_address_pending;

/*
 * Set current device address and mark the
 * interface as active
 */
static void
ao_usb_set_address(uint8_t address)
{
	ao_usb_dev_set_address(address);
	ao_usb_address_pending = 0;
}

#define TX_DBG 0
#define RX_DBG 0

#if TX_DBG
#define _tx_dbg0(msg) _dbg(__LINE__,msg,0)
#define _tx_dbg1(msg,value) _dbg(__LINE__,msg,value)
#else
#define _tx_dbg0(msg)
#define _tx_dbg1(msg,value)
#endif

#if RX_DBG
#define _rx_dbg0(msg) _dbg(__LINE__,msg,0)
#define _rx_dbg1(msg,value) _dbg(__LINE__,msg,value)
#else
#define _rx_dbg0(msg)
#define _rx_dbg1(msg,value)
#endif

#if TX_DBG || RX_DBG
static void _dbg(int line, char *msg, uint32_t value);
#endif

/*
 * Set just endpoint 0, for use during startup
 */

static void
ao_usb_set_ep0(void)
{
	ao_usb_dev_ep0_init();

	ao_usb_set_address(0);

	ao_usb_running = 0;

	/* Reset our internal state
	 */

	ao_usb_ep0_state = AO_USB_EP0_IDLE;

	ao_usb_ep0_in_data = NULL;
	ao_usb_ep0_in_len = 0;

	ao_usb_ep0_out_data = 0;
	ao_usb_ep0_out_len = 0;
}

static void
ao_usb_set_configuration(void)
{
#if 0
	/* Set up the INT end point */
	ao_usb_bdt[AO_USB_INT_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_INT_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_INT_SIZE;

	ao_usb_init_ep(AO_USB_INT_EPR,
		       AO_USB_INT_EP);

	/* Set up the OUT end point */
	ao_usb_bdt[AO_USB_OUT_EPR].single.addr_rx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_OUT_EPR].single.count_rx = ((1 << STM_USB_BDT_COUNT_RX_BL_SIZE) |
						      (((AO_USB_OUT_SIZE / 32) - 1) << STM_USB_BDT_COUNT_RX_NUM_BLOCK));
	ao_usb_out_rx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_OUT_SIZE;

	ao_usb_init_ep(AO_USB_OUT_EPR,
		       AO_USB_OUT_EP,
		       STM_USB_EPR_EP_TYPE_BULK,
		       STM_USB_EPR_STAT_RX_VALID,
		       STM_USB_EPR_STAT_TX_DISABLED);

	/* Set up the IN end point */
	ao_usb_bdt[AO_USB_IN_EPR].single.addr_tx = ao_usb_sram_addr;
	ao_usb_bdt[AO_USB_IN_EPR].single.count_tx = 0;
	ao_usb_in_tx_buffer = ao_usb_packet_buffer_addr(ao_usb_sram_addr);
	ao_usb_sram_addr += AO_USB_IN_SIZE;

	ao_usb_init_ep(AO_USB_IN_EPR,
		       AO_USB_IN_EP,
		       STM_USB_EPR_EP_TYPE_BULK,
		       STM_USB_EPR_STAT_RX_DISABLED,
		       STM_USB_EPR_STAT_TX_NAK);
#endif

	ao_usb_in_flushed = 0;
	ao_usb_in_pending = 0;
	ao_wakeup(&ao_usb_in_pending);

	ao_usb_out_avail = 0;
	ao_usb_configuration = 0;

	ao_usb_running = 1;
	ao_wakeup(&ao_usb_running);
}

static uint16_t	control_count;
static uint16_t	in_count;
static uint16_t	out_count;
#if USB_DEBUG
static uint16_t int_count;
static uint16_t	reset_count;
#endif

/* Send an IN data packet */
static void
ao_usb_ep0_flush(void)
{
	uint8_t this_len;

	/* Check to see if the endpoint is still busy */
	if (ao_usb_dev_ep0_in_busy()) {
		return;
	}

	this_len = ao_usb_ep0_in_len;
	if (this_len > AO_USB_CONTROL_SIZE)
		this_len = AO_USB_CONTROL_SIZE;

	if (this_len < AO_USB_CONTROL_SIZE)
		ao_usb_ep0_state = AO_USB_EP0_IDLE;

	ao_usb_ep0_in_len -= this_len;

	ao_usb_dev_ep0_in(ao_usb_ep0_in_data, this_len);
	ao_usb_ep0_in_data += this_len;
}

/* Read data from the ep0 OUT fifo */
static void
ao_usb_ep0_fill(void)
{
	uint16_t	len;

	len = ao_usb_dev_ep0_out(ao_usb_ep0_out_data, ao_usb_ep0_out_len);
	ao_usb_ep0_out_len -= len;
	ao_usb_ep0_out_data += len;
}

static void
ao_usb_ep0_in_reset(void)
{
	ao_usb_ep0_in_data = ao_usb_ep0_in_buf;
	ao_usb_ep0_in_len = 0;
}

static void
ao_usb_ep0_in_queue_byte(uint8_t a)
{
	if (ao_usb_ep0_in_len < sizeof (ao_usb_ep0_in_buf))
		ao_usb_ep0_in_buf[ao_usb_ep0_in_len++] = a;
}

static void
ao_usb_ep0_in_set(const uint8_t *data, uint8_t len)
{
	ao_usb_ep0_in_data = data;
	ao_usb_ep0_in_len = len;
}

static void
ao_usb_ep0_out_set(uint8_t *data, uint8_t len)
{
	ao_usb_ep0_out_data = data;
	ao_usb_ep0_out_len = len;
}

static void
ao_usb_ep0_in_start(uint16_t max)
{
	/* Don't send more than asked for */
	if (ao_usb_ep0_in_len > max)
		ao_usb_ep0_in_len = max;

	ao_usb_dev_ep0_in(ao_usb_ep0_in_data, ao_usb_ep0_in_len);
}

struct ao_usb_line_coding ao_usb_line_coding = {115200, 0, 0, 8};

/* Walk through the list of descriptors and find a match
 */
static void
ao_usb_get_descriptor(uint16_t value, uint16_t length)
{
	const uint8_t		*descriptor;
	uint8_t		type = value >> 8;
	uint8_t		index = value;

	descriptor = ao_usb_descriptors;
	while (descriptor[0] != 0) {
		if (descriptor[1] == type && index-- == 0) {
			uint8_t	len;
			if (type == AO_USB_DESC_CONFIGURATION)
				len = descriptor[2];
			else
				len = descriptor[0];
			if (len > length)
				len = length;
			ao_usb_ep0_in_set(descriptor, len);
			break;
		}
		descriptor += descriptor[0];
	}
}

static void
ao_usb_ep0_setup(void)
{
	uint16_t	setup_len;

	/* Pull the setup packet out of the fifo */
	setup_len = ao_usb_dev_ep0_out(&ao_usb_setup, 8);
	if (setup_len != 8) {
		return;
	}

	if ((ao_usb_setup.dir_type_recip & AO_USB_DIR_IN) || ao_usb_setup.length == 0)
		ao_usb_ep0_state = AO_USB_EP0_DATA_IN;
	else
		ao_usb_ep0_state = AO_USB_EP0_DATA_OUT;

	ao_usb_ep0_in_reset();

	switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_TYPE_MASK) {
	case AO_USB_TYPE_STANDARD:
		switch(ao_usb_setup.dir_type_recip & AO_USB_SETUP_RECIP_MASK) {
		case AO_USB_RECIP_DEVICE:
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_SET_ADDRESS:
				ao_usb_address = ao_usb_setup.value;
				ao_usb_address_pending = 1;
				break;
			case AO_USB_REQ_GET_DESCRIPTOR:
				ao_usb_get_descriptor(ao_usb_setup.value, ao_usb_setup.length);
				break;
			case AO_USB_REQ_GET_CONFIGURATION:
				ao_usb_ep0_in_queue_byte(ao_usb_configuration);
				break;
			case AO_USB_REQ_SET_CONFIGURATION:
				ao_usb_configuration = ao_usb_setup.value;
				ao_usb_set_configuration();
				break;
			}
			break;
		case AO_USB_RECIP_INTERFACE:
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_GET_INTERFACE:
				ao_usb_ep0_in_queue_byte(0);
				break;
			case AO_USB_REQ_SET_INTERFACE:
				break;
			}
			break;
		case AO_USB_RECIP_ENDPOINT:
			switch(ao_usb_setup.request) {
			case AO_USB_REQ_GET_STATUS:
				ao_usb_ep0_in_queue_byte(0);
				ao_usb_ep0_in_queue_byte(0);
				break;
			}
			break;
		}
		break;
	case AO_USB_TYPE_CLASS:
		switch (ao_usb_setup.request) {
		case AO_USB_SET_LINE_CODING:
			ao_usb_ep0_out_set((uint8_t *) &ao_usb_line_coding, 7);
			break;
		case AO_USB_GET_LINE_CODING:
			ao_usb_ep0_in_set((const uint8_t *) &ao_usb_line_coding, 7);
			break;
		case AO_USB_SET_CONTROL_LINE_STATE:
			break;
		}
		break;
	}

	/* If we're not waiting to receive data from the host,
	 * queue an IN response
	 */
	if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN)
		ao_usb_ep0_in_start(ao_usb_setup.length);
}

static void
ao_usb_ep0_handle(uint8_t receive)
{
	if (receive & AO_USB_EP0_GOT_RESET) {
		ao_usb_set_ep0();
		return;
	}
	if (receive & AO_USB_EP0_GOT_SETUP) {
		ao_usb_ep0_setup();
	}
	if (receive & AO_USB_EP0_GOT_RX_DATA) {
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_OUT) {
			ao_usb_ep0_fill();
			if (ao_usb_ep0_out_len == 0) {
				ao_usb_ep0_state = AO_USB_EP0_DATA_IN;
				ao_usb_ep0_in_start(0);
			}
		}
	}
	if (receive & AO_USB_EP0_GOT_TX_ACK) {
#if HAS_FLIGHT && AO_USB_FORCE_IDLE
		ao_flight_force_idle = 1;
#endif
		/* Wait until the IN packet is received from addr 0
		 * before assigning our local address
		 */
		if (ao_usb_address_pending)
			ao_usb_set_address(ao_usb_address);
		if (ao_usb_ep0_state == AO_USB_EP0_DATA_IN)
			ao_usb_ep0_flush();
	}
}

void
ao_usb_ep0_interrupt(uint8_t mask)
{
	if (mask) {
		++control_count;
		ao_usb_ep0_handle(mask);
	}
}

void
ao_usb_in_interrupt(uint32_t mask)
{
	if (mask & (1 << AO_USB_IN_EPR)) {
		++in_count;
		_tx_dbg1("TX ISR", epr);
		ao_usb_in_pending = 0;
		ao_wakeup(&ao_usb_in_pending);
	}
}

void
ao_usb_out_interrupt(uint32_t mask)
{
	if (mask & (1 << AO_USB_OUT_EPR)) {
		++out_count;
		_rx_dbg1("RX ISR", epr);
		ao_usb_out_avail = 1;
		_rx_dbg0("out avail set");
		ao_wakeup(AO_USB_OUT_SLEEP_ADDR);
		_rx_dbg0("stdin awoken");
	}
}

void
ao_usb_int_interrupt(uint32_t mask)
{
	(void) mask;
}

void
stm_otg_fs_wkup_isr(void)
{
	/* USB wakeup, just clear the bit for now */
//	stm_usb.istr &= ~(1 << STM_USB_ISTR_WKUP);
}

/* Queue the current IN buffer for transmission */
static void
_ao_usb_in_send(void)
{
	_tx_dbg0("in_send start");

	while (ao_usb_in_pending)
		ao_sleep(&ao_usb_in_pending);

	ao_usb_in_pending = 1;
	if (ao_usb_tx_count != AO_USB_IN_SIZE)
		ao_usb_in_flushed = 1;

	ao_usb_dev_ep_in(AO_USB_IN_EPR, ao_usb_tx_buffer, ao_usb_tx_count);
	ao_usb_tx_count = 0;

	_tx_dbg0("in_send end");
}

/* Wait for a free IN buffer. Interrupts are blocked */
static void
_ao_usb_in_wait(void)
{
	for (;;) {
		/* Check if the current buffer is writable */
		if (ao_usb_tx_count < AO_USB_IN_SIZE)
			break;

		_tx_dbg0("in_wait top");
		/* Wait for an IN buffer to be ready */
		while (ao_usb_in_pending)
			ao_sleep(&ao_usb_in_pending);
		_tx_dbg0("in_wait bottom");
	}
}

void
ao_usb_flush(void)
{
	if (!ao_usb_running)
		return;

	/* Anytime we've sent a character since
	 * the last time we flushed, we'll need
	 * to send a packet -- the only other time
	 * we would send a packet is when that
	 * packet was full, in which case we now
	 * want to send an empty packet
	 */
	ao_arch_block_interrupts();
	while (!ao_usb_in_flushed) {
		_tx_dbg0("flush top");
		_ao_usb_in_send();
		_tx_dbg0("flush end");
	}
	ao_arch_release_interrupts();
}

void
ao_usb_putchar(char c)
{
	if (!ao_usb_running)
		return;

	ao_arch_block_interrupts();
	_ao_usb_in_wait();

	ao_usb_in_flushed = 0;
	ao_usb_tx_buffer[ao_usb_tx_count++] = (uint8_t) c;

	/* Send the packet when full */
	if (ao_usb_tx_count == AO_USB_IN_SIZE) {
		_tx_dbg0("putchar full");
		_ao_usb_in_send();
		_tx_dbg0("putchar flushed");
	}
	ao_arch_release_interrupts();
}

static void
_ao_usb_out_recv(void)
{
	_rx_dbg0("out_recv top");
	ao_usb_out_avail = 0;

	ao_usb_rx_count = ao_usb_dev_ep_out(AO_USB_OUT_EPR, ao_usb_rx_buffer, sizeof (ao_usb_rx_buffer));

	_rx_dbg1("out_recv count", ao_usb_rx_count);

	ao_usb_rx_pos = 0;
}

static int
_ao_usb_pollchar(void)
{
	uint8_t c;

	if (!ao_usb_running)
		return AO_READ_AGAIN;

	for (;;) {
		if (ao_usb_rx_pos != ao_usb_rx_count)
			break;

		_rx_dbg0("poll check");
		/* Check to see if a packet has arrived */
		if (!ao_usb_out_avail) {
			_rx_dbg0("poll none");
			return AO_READ_AGAIN;
		}
		_ao_usb_out_recv();
	}

	/* Pull a character out of the fifo */
	c = ao_usb_rx_buffer[ao_usb_rx_pos++];
	return c;
}

char
ao_usb_getchar(void)
{
	int	c;

	ao_arch_block_interrupts();
	while ((c = _ao_usb_pollchar()) == AO_READ_AGAIN)
		ao_sleep(AO_USB_OUT_SLEEP_ADDR);
	ao_arch_release_interrupts();
	return c;
}

#ifndef HAS_USB_DISABLE
#define HAS_USB_DISABLE 1
#endif

#if HAS_USB_DISABLE
void
ao_usb_disable(void)
{
	ao_usb_dev_disable();
}
#endif

void
ao_usb_enable(void)
{
	ao_usb_dev_enable();
	ao_usb_configuration = 0;
}

#if USB_ECHO
struct ao_task ao_usb_echo_task;

static void
ao_usb_echo(void)
{
	char	c;

	for (;;) {
		c = ao_usb_getchar();
		ao_usb_putchar(c);
		ao_usb_flush();
	}
}
#endif

#if USB_DEBUG
static void
ao_usb_irq(void)
{
	printf ("control: %d out: %d in: %d int: %d reset: %d\n",
		control_count, out_count, in_count, int_count, reset_count);
}

const struct ao_cmds ao_usb_cmds[] = {
	{ ao_usb_irq, "I\0Show USB interrupt counts" },
	{ 0, NULL }
};
#endif

void
ao_usb_init(void)
{
	ao_usb_enable();

	ao_usb_ep0_state = AO_USB_EP0_IDLE;
#if USB_ECHO
	ao_add_task(&ao_usb_echo_task, ao_usb_echo, "usb echo");
#endif
#if USB_DEBUG
	ao_cmd_register(&ao_usb_cmds[0]);
#endif
#if !USB_ECHO
#if USE_USB_STDIO
	ao_add_stdio(_ao_usb_pollchar, ao_usb_putchar, ao_usb_flush);
#endif
#endif
}

#if TX_DBG || RX_DBG

struct ao_usb_dbg {
	int		line;
	char		*msg;
	uint32_t	value;
	uint32_t	prival;
#if TX_DBG
	uint16_t	in_count;
	uint32_t	in_epr;
	uint32_t	in_pending;
	uint32_t	tx_count;
	uint32_t	in_flushed;
#endif
#if RX_DBG
	uint8_t		rx_count;
	uint8_t		rx_pos;
	uint8_t		out_avail;
	uint32_t	out_epr;
#endif
};

#define NUM_USB_DBG	16

static struct ao_usb_dbg dbg[NUM_USB_DBG];
static int dbg_i;

static void _dbg(int line, char *msg, uint32_t value)
{
	uint32_t	prival;
	dbg[dbg_i].line = line;
	dbg[dbg_i].msg = msg;
	dbg[dbg_i].value = value;
#if AO_NONMASK_INTERRUPT
	asm("mrs %0,basepri" : "=&r" (prival));
#else
	asm("mrs %0,primask" : "=&r" (prival));
#endif
	dbg[dbg_i].prival = prival;
#if TX_DBG
	dbg[dbg_i].in_count = in_count;
	dbg[dbg_i].in_epr = stm_usb.epr[AO_USB_IN_EPR];
	dbg[dbg_i].in_pending = ao_usb_in_pending;
	dbg[dbg_i].tx_count = ao_usb_tx_count;
	dbg[dbg_i].in_flushed = ao_usb_in_flushed;
#endif
#if RX_DBG
	dbg[dbg_i].rx_count = ao_usb_rx_count;
	dbg[dbg_i].rx_pos = ao_usb_rx_pos;
	dbg[dbg_i].out_avail = ao_usb_out_avail;
	dbg[dbg_i].out_epr = stm_usb.epr[AO_USB_OUT_EPR];
#endif
	if (++dbg_i == NUM_USB_DBG)
		dbg_i = 0;
}
#endif
