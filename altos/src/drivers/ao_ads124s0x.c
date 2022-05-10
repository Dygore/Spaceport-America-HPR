/*
 * Copyright © 2019 Bdale Garbee <bdale@gag.com>
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
#include <ao_exti.h>
#include "ao_ads124s0x.h"

#define DEBUG_LOW	1
#define DEBUG_HIGH	2

#define DEBUG		0

#if DEBUG
#define PRINTD(l, ...) do { if (DEBUG & (l)) { printf ("\r%5u %s: ", ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } } while(0)
#else
#define PRINTD(l,...)
#endif

struct ao_ads124s0x_sample	ao_ads124s0x_current;
uint8_t		nextchan = 0;
uint8_t		ao_ads124s0x_drdy;

static void
ao_ads124s0x_start(void) {
	ao_spi_get_bit(AO_ADS124S0X_SPI_CS_PORT,
		       AO_ADS124S0X_SPI_CS_PIN,
		       AO_ADS124S0X_SPI_BUS,
		       AO_ADS124S0X_SPI_SPEED);
}

static void
ao_ads124s0x_stop(void) {
	ao_spi_put_bit(AO_ADS124S0X_SPI_CS_PORT,
		       AO_ADS124S0X_SPI_CS_PIN,
		       AO_ADS124S0X_SPI_BUS);
}

static uint8_t
ao_ads124s0x_reg_read(uint8_t addr)
{
	uint8_t	d[3];

	d[0] = addr | AO_ADS124S0X_RREG;
	d[1] = 0;			
	d[2] = 0;			
	ao_ads124s0x_start();
	ao_spi_duplex(d, d, 3, AO_ADS124S0X_SPI_BUS);
	ao_ads124s0x_stop();

	PRINTD(DEBUG_LOW, "read %x = %x\n", addr, d[0]);

	return d[2];
}

/*
static void
ao_ads124s0x_reg_write(uint8_t addr, uint8_t value)
{
	uint8_t	d[3];

	PRINTD(DEBUG_LOW, "write %x %x\n", addr, value);
	d[0] = addr | AO_ADS124S0X_WREG;
	d[1] = 0;			
	d[2] = value;
	ao_ads124s0x_start();
	ao_spi_send(d, 3, AO_ADS124S0X_SPI_BUS);
	ao_ads124s0x_stop();

}
*/

static void 
ao_ads124s0x_isr(void)
{
	ao_ads124s0x_drdy = 1;
	ao_wakeup(&ao_ads124s0x_drdy);
}

static void
ao_ads124s0x_setup(void)
{
	uint8_t	d[20];

	ao_delay(1);

	ao_gpio_set(AO_ADS124S0X_RESET_PORT, AO_ADS124S0X_RESET_PIN, 1);

	ao_delay(1);

	uint8_t	devid = ao_ads124s0x_reg_read(AO_ADS124S0X_ID);
	if ((devid & 7) != AO_ADS124S0X_ID_ADS124S06)
		ao_panic(AO_PANIC_SELF_TEST_ADS124S0X);

	ao_exti_setup(AO_ADS124S0X_DRDY_PORT, AO_ADS124S0X_DRDY_PIN,
		AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_HIGH,
		ao_ads124s0x_isr);

	/* run converter at 4ksps so we can scan 4 channels at 1ksps using
	   full duplex ala datasheet section 9.5.4.3 */

	d[0] = AO_ADS124S0X_INPMUX | AO_ADS124S0X_WREG;
	d[1] = 8;	/* write 8 registers starting with INPMUX */
	d[2] = 0x0c;	/* input mux AIN0 relative to AINCOM */
	d[3] = 0x00;	/* default first conversion delay, pga disabled */
	d[4] = 0x1e;	/* gchop disabled, internal clock, continuous 
			   conversion, low-latency filter, 4000 SPS */
	d[5] = 0x00;	/* ref monitor disabled, ref buffers bypassed, ref 
			   set to REFP0/REFN0, internal reference off */
	d[6] = 0x00;	/* pga otuput rail, low side power switch, excitation
			   current source all off */
	d[7] = 0xff;	/* idac1 and idac2 disconnected */
	d[8] = 0x00;	/* all vbias disconnected */
	d[9] = 0x10;	/* sys monitor off, spi timeout disabled, crc disabled,
			   prepending status byte disabled */
	ao_ads124s0x_start();
	ao_spi_send(d, 10, AO_ADS124S0X_SPI_BUS);
	ao_ads124s0x_stop();

	/* start conversions */
	
	d[0] = AO_ADS124S0X_START;
	ao_ads124s0x_start();
	ao_spi_send(d, 1, AO_ADS124S0X_SPI_BUS);
	ao_ads124s0x_stop();
}

static void
ao_ads124s0x(void)
{
	uint8_t	d[3], curchan;

	ao_ads124s0x_setup();

	ao_exti_enable(AO_ADS124S0X_DRDY_PORT, AO_ADS124S0X_DRDY_PIN);

	for (;;) {
		ao_arch_block_interrupts();
		ao_ads124s0x_drdy = 0;
		while (ao_ads124s0x_drdy == 0)
			ao_sleep(&ao_ads124s0x_drdy);
		ao_arch_release_interrupts();

		curchan = nextchan;
		nextchan = (nextchan + 1) % AO_ADS124S0X_CHANNELS;

		d[0] = AO_ADS124S0X_INPMUX | AO_ADS124S0X_WREG;
		d[1] = 1;			
		d[2] = nextchan << 4 | 0x0c; ;	/* relative to AINCOM */
		ao_ads124s0x_start();
		ao_spi_duplex(d, d, 3, AO_ADS124S0X_SPI_BUS);
		ao_ads124s0x_stop();

		ao_ads124s0x_current.ain[curchan] = 
			d[0] << 16 | d[1] << 8 | d[2];

		// FIXME
		//	If nextchan == 0, we have a complete set of inputs
		//	and we need to log them somewhere

		ao_ads124s0x_drdy = 0;
	}
}

static struct ao_task ao_ads124s0x_task;

static void
ao_ads124s0x_dump(void)	
{
	static int done;

	if (!done) {
		done = 1;
		ao_add_task(&ao_ads124s0x_task, ao_ads124s0x, "ads124s0x");
	}
		
	printf ("ADS124S0X value %d %d %d %d\n",
		ao_ads124s0x_current.ain[0],
		ao_ads124s0x_current.ain[1],
		ao_ads124s0x_current.ain[2],
		ao_ads124s0x_current.ain[3]);
}

const struct ao_cmds ao_ads124s0x_cmds[] = {
	{ ao_ads124s0x_dump,	"I\0Display ADS124S0X data" },
	{ 0, NULL },
};

void
ao_ads124s0x_init(void)
{
	ao_cmd_register(ao_ads124s0x_cmds);

	ao_enable_output(AO_ADS124S0X_RESET_PORT, AO_ADS124S0X_RESET_PIN, 0);

	ao_spi_init_cs(AO_ADS124S0X_SPI_CS_PORT, 
		(1 << AO_ADS124S0X_SPI_CS_PIN));

//	ao_add_task(&ao_ads124s0x_task, ao_ads124s0x, "ads124s0x");
}
