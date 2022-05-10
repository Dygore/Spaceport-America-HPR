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

#ifndef _AO_RN4678_H_
#define _AO_RN4678_H_

/* From the rn4678 pictail board

    1	SW_BTN	0-off/1-on	 2	P2_0	1-app/0-test (boot time)
    3	P2_4	1-app/0-WF	 4	EAN	0-app/1-WF (reboots)
    5				 6	RTS	0
    7				 8	P3_2	1 (NC)
    9	TXD			10	P3_3	1 (NC)
   11	RXD			12	P3_4	1 (NC)
   13				14
   15				16	P3_7	1 (NC)
   17	P0_5	1 (NC)		18	RST_N	1 run/0 reset
   19	WAKE_UP	1 run/0 btn	20
   21	P0_4	0 (NC)		22
   23	P1_5	1 (NC)		24	P3_1	1 (NC)
   25	CTS	0		26	3.3V
   27				28	GND


   Interesting pins:

   Not connected to microcontroller:

   SW_BTN	0-off		1-on
   P2_4		0 write-flash	1-app
   WAKE_UP	0-stop		1-run
   P2_0		0 test		1-run
   EAN		1-WF (reboots)	0-run
   RST_N	0 reset		1 run

   Connected to microcontroller:

   CTS		mostly 0
   RTS		mostly 1

   TXD
   RXD

   Other connections -- LDO33_O to VDD_IO

*/

#define AO_RN_REBOOT_MSG	"REBOOT"

#define AO_RN_CMD_TIMEOUT	AO_MS_TO_TICKS(200)

#define AO_RN_REBOOT_TIMEOUT	AO_MS_TO_TICKS(2000)

#define AO_RN_MAX_REPLY_LEN	10

#define AO_RN_SET_NAME_CMD		"SN,"
#define AO_RN_GET_NAME_CMD		"GN"

#define AO_RN_SET_STATUS_STRING	"so,"
#define AO_RN_STATUS_STRING_DISABLE	" "
#define AO_RN_STATUS_STRING_ENABLE	"%,%"

#define AO_RN_REBOOT_CMD		"R,1"

#define AO_RN_VERSION_CMD		"V"

#define AO_RN_TIMEOUT	-1
#define AO_RN_ERROR	0
#define AO_RN_OK	1

#define AO_RN_SET_COMMAND_PIN	"SX,07,0B"

#define AO_RN_SET_AUTH_JUST_WORKS	"SA,2"
#define AO_RN_SET_FAST_MODE		"SQ,9000"

/* This pin is configured to control cmd/data mode */
#define AO_RN_CMD_PORT	AO_RN_P3_7_PORT
#define AO_RN_CMD_PIN	AO_RN_P3_7_PIN

#define AO_RN_CMD_CMD	0
#define AO_RN_CMD_DATA	1

/* This pin indicates BT connection status */
#define AO_RN_CONNECTED_PORT	AO_RN_P1_5_PORT
#define AO_RN_CONNECTED_PIN	AO_RN_P1_5_PIN

void
ao_rn4678_init(void);

#endif /* _AO_RN_H_ */
