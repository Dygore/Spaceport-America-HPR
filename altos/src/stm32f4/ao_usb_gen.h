/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_USB_GEN_H_
#define _AO_USB_GEN_H_

#include "ao.h"
#include "ao_usb.h"
#include "ao_product.h"
#include <stdint.h>

#define USB_ECHO	0

#ifndef USE_USB_STDIO
#define USE_USB_STDIO	1
#endif

#if USE_USB_STDIO
#define AO_USB_OUT_SLEEP_ADDR	(&ao_stdin_ready)
#else
#define AO_USB_OUT_SLEEP_ADDR	(&ao_usb_out_avail)
#endif

struct ao_usb_setup {
	uint8_t		dir_type_recip;
	uint8_t		request;
	uint16_t	value;
	uint16_t	index;
	uint16_t	length;
} ao_usb_setup;

#define AO_USB_EP0_GOT_RESET	1
#define AO_USB_EP0_GOT_SETUP	2
#define AO_USB_EP0_GOT_RX_DATA	4
#define AO_USB_EP0_GOT_TX_ACK	8

/*
 * End point register indices
 */

#define AO_USB_CONTROL_EPR	0
#define AO_USB_INT_EPR		1
#define AO_USB_OUT_EPR		2
#define AO_USB_IN_EPR		3

/* Device interfaces required */

/* Queue IN bytes to EP0 */
void
ao_usb_dev_ep0_init(void);

void
ao_usb_dev_ep0_in(const void *data, uint16_t len);

bool
ao_usb_dev_ep0_in_busy(void);

/* Receive OUT bytes from EP0 */
uint16_t
ao_usb_dev_ep0_out(void *data, uint16_t len);

/* Set device address */
void
ao_usb_dev_set_address(uint8_t address);

void
ao_usb_dev_enable(void);

void
ao_usb_dev_disable(void);

void
ao_usb_dev_init(void);

/* Queue IN bytes to EPn */
void
ao_usb_dev_ep_in(uint8_t ep, const void *data, uint16_t len);

bool
ao_usb_dev_ep_in_busy(uint8_t ep);

/* Receive OUT bytes from EPn */
uint16_t
ao_usb_dev_ep_out(uint8_t ep, void *data, uint16_t len);


/* General interfaces provided */

void
ao_usb_ep0_interrupt(uint8_t mask);

void
ao_usb_in_interrupt(uint32_t mask);

void
ao_usb_out_interrupt(uint32_t mask);

void
ao_usb_int_interrupt(uint32_t mask);

#endif /* _AO_USB_GEN_H_ */

