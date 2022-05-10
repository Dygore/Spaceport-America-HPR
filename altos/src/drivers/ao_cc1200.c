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
#include <ao_cc1200.h>
#include <ao_exti.h>
#include <ao_fec.h>
#include <ao_packet.h>
#if HAS_PAD
#include <ao_pad.h>
#endif

static uint8_t cc1201;

static uint8_t ao_radio_mutex;

static uint8_t ao_radio_wake;		/* radio ready. Also used as sleep address */
static uint8_t ao_radio_abort;		/* radio operation should abort */

int8_t	ao_radio_rssi;			/* Last received RSSI value */

#ifndef CC1200_DEBUG
#define CC1200_DEBUG		0
#endif

#ifndef CC1200_LOW_LEVEL_DEBUG
#define CC1200_LOW_LEVEL_DEBUG	0
#endif

#define CC1200_TRACE		0
#define CC1200_APRS_TRACE	0

extern const uint32_t	ao_radio_cal;

#ifdef AO_CC1200_FOSC
#define FOSC	AO_CC1200_FOSC
#else
#define FOSC	40000000
#endif

#define AO_CC1200_SPI_SPEED	ao_spi_speed(7700000)	/* 7.7MHz max for extended memory reads */

#define ao_radio_select()	ao_spi_get_mask(AO_CC1200_SPI_CS_PORT,(1 << AO_CC1200_SPI_CS_PIN),AO_CC1200_SPI_BUS,AO_CC1200_SPI_SPEED)
#define ao_radio_deselect()	ao_spi_put_mask(AO_CC1200_SPI_CS_PORT,(1 << AO_CC1200_SPI_CS_PIN),AO_CC1200_SPI_BUS)
#define ao_radio_spi_send(d,l)	ao_spi_send((d), (l), AO_CC1200_SPI_BUS)
#define ao_radio_spi_send_fixed(d,l) ao_spi_send_fixed((d), (l), AO_CC1200_SPI_BUS)
#define ao_radio_spi_recv(d,l)	ao_spi_recv((d), (l), AO_CC1200_SPI_BUS)
#define ao_radio_duplex(o,i,l)	ao_spi_duplex((o), (i), (l), AO_CC1200_SPI_BUS)

static uint8_t
ao_radio_reg_read(uint16_t addr)
{
	uint8_t	data[2];
	uint8_t	d;

#if CC1200_TRACE
	printf("\t\tao_radio_reg_read (%04x): ", addr); flush();
#endif
	if (CC1200_IS_EXTENDED(addr)) {
		data[0] = ((1 << CC1200_READ)  |
			   (0 << CC1200_BURST) |
			   CC1200_EXTENDED);
		data[1] = (uint8_t) addr;
		d = 2;
	} else {
		data[0] = ((1 << CC1200_READ)  |
			   (0 << CC1200_BURST) |
			   (uint8_t) addr);
		d = 1;
	}
	ao_radio_select();
	ao_radio_spi_send(data, d);
	ao_radio_spi_recv(data, 1);
	ao_radio_deselect();
#if CC1200_TRACE
	printf (" %02x\n", data[0]);
#endif
	return data[0];
}

static void
ao_radio_reg_write(uint16_t addr, uint8_t value)
{
	uint8_t	data[3];
	uint8_t	d;

#if CC1200_TRACE
	printf("\t\tao_radio_reg_write (%04x): %02x\n", addr, value);
#endif
	if (CC1200_IS_EXTENDED(addr)) {
		data[0] = ((0 << CC1200_READ)  |
			   (0 << CC1200_BURST) |
			   CC1200_EXTENDED);
		data[1] = (uint8_t) addr;
		d = 2;
	} else {
		data[0] = ((0 << CC1200_READ)  |
			   (0 << CC1200_BURST) |
			   (uint8_t) addr);
		d = 1;
	}
	data[d] = value;
	ao_radio_select();
	ao_radio_spi_send(data, d+1);
	ao_radio_deselect();
#if CC1200_TRACE
	(void) ao_radio_reg_read(addr);
#endif
}

static uint8_t
ao_radio_strobe(uint8_t addr)
{
	uint8_t	in;

#if CC1200_TRACE
	printf("\t\tao_radio_strobe (%02x): ", addr); flush();
#endif
	ao_radio_select();
	ao_radio_duplex(&addr, &in, 1);
	ao_radio_deselect();
#if CC1200_TRACE
	printf("%02x\n", in); flush();
#endif
	return in;
}

static uint8_t
ao_radio_fifo_read(uint8_t *data, uint8_t len)
{
	uint8_t	addr = ((1 << CC1200_READ)  |
			(1 << CC1200_BURST) |
			CC1200_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	ao_radio_spi_recv(data, len);
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_fifo_write_start(void)
{
	uint8_t	addr = ((0 << CC1200_READ)  |
			(1 << CC1200_BURST) |
			CC1200_FIFO);
	uint8_t status;

	ao_radio_select();
	ao_radio_duplex(&addr, &status, 1);
	return status;
}

static inline uint8_t ao_radio_fifo_write_stop(uint8_t status) {
	ao_radio_deselect();
	return status;
}

static uint8_t
ao_radio_fifo_write(const uint8_t *data, uint8_t len)
{
	uint8_t	status = ao_radio_fifo_write_start();
	ao_radio_spi_send(data, len);
	return ao_radio_fifo_write_stop(status);
}

static uint8_t
ao_radio_fifo_write_fixed(uint8_t data, uint8_t len)
{
	uint8_t status = ao_radio_fifo_write_start();
	ao_radio_spi_send_fixed(data, len);
	return ao_radio_fifo_write_stop(status);
}

static uint8_t
ao_radio_int_pin(void)
{
	return ao_gpio_get(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
}

static uint8_t
ao_radio_status(void)
{
	return ao_radio_strobe (CC1200_SNOP);
}

void
ao_radio_recv_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_wake);
}

#define ao_radio_rdf_value 0x55

static void
ao_radio_isr(void)
{
	ao_exti_disable(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
	ao_radio_wake = 1;
	ao_wakeup(&ao_radio_wake);
}

static void
ao_radio_start_tx(void)
{
	ao_exti_enable(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
	ao_radio_strobe(CC1200_STX);
}

static void
ao_radio_start_rx(void)
{
	ao_exti_enable(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
	ao_radio_strobe(CC1200_SRX);
}

static void
ao_radio_idle(void)
{
	for (;;) {
		uint8_t	state = (ao_radio_strobe(CC1200_SIDLE) >> CC1200_STATUS_STATE) & CC1200_STATUS_STATE_MASK;
		if (state == CC1200_STATUS_STATE_IDLE)
			break;
		if (state == CC1200_STATUS_STATE_TX_FIFO_ERROR)
			ao_radio_strobe(CC1200_SFTX);
		if (state == CC1200_STATUS_STATE_RX_FIFO_ERROR)
			ao_radio_strobe(CC1200_SFRX);
	}
	/* Flush any pending data in the fifos */
	ao_radio_strobe(CC1200_SFTX);
	ao_radio_strobe(CC1200_SFRX);
	/* Make sure the RF calibration is current */
	ao_radio_strobe(CC1200_SCAL);
}

/*
 * Packet deviation
 *
 *	fdev = fosc >> 22 * (256 + dev_m) << dev_e
 *
 * Deviation for 38400 baud should be 20.5kHz:
 *
 *     	40e6 / (2 ** 22) * (256 + 13) * (2 ** 3) = 20523Hz
 *
 * Deviation for 9600 baud should be 5.125kHz:
 *
 *     	40e6 / (2 ** 22) * (256 + 13) * (2 ** 1) = 5131Hz
 *
 * Deviation for 2400 baud should be 1.28125kHz, but cc1111 and
 * cc115l can't do that, so we'll use 1.5kHz instead:
 *
 *     	40e6 / (2 ** 21) * (79) = 1506Hz
 */

#define PACKET_DEV_M_384	13
#define PACKET_DEV_E_384	3

#define PACKET_DEV_M_96		13
#define PACKET_DEV_E_96		1

#define PACKET_DEV_M_24		79
#define PACKET_DEV_E_24		0

/*
 * For our packet data
 *
 *              (2**20 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 39
 *
 * Given the bit period of the baseband, T, the bandwidth of the
 * baseband signal is B = 1/(2T).  The overall bandwidth of the
 * modulated signal is then Channel bandwidth = 2Δf + 2B.
 *
 * 38400 -- 2 * 20500 + 38400 = 79.4 kHz
 *  9600 -- 2 * 5.125 +  9600 = 19.9 kHz
 *  2400 -- 2 * 1.5   +  2400 =  5.4 khz
 *
 * Symbol rate 38400 Baud:
 *
 *	DATARATE_M = 1013008
 *	DATARATE_E = 8
 *	CHANBW = 104.16667
 *
 * Symbol rate 9600 Baud:
 *
 *	DATARATE_M = 1013008
 *	DATARATE_E = 6
 *	CHANBW = 26.042 (round to 19.8)
 *
 * Symbol rate 2400 Baud:
 *
 *	DATARATE_M = 1013008
 *	DATARATE_E = 4
 *	CHANBW = 5.0 (round to 9.5)
 */

#if FOSC == 40000000
#define PACKET_SYMBOL_RATE_M		1013008
#define PACKET_SYMBOL_RATE_E_384	8
#define PACKET_SYMBOL_RATE_E_96		6
#define PACKET_SYMBOL_RATE_E_24		4
#endif

#if FOSC == 32000000
#define PACKET_SYMBOL_RATE_M		239914
#define PACKET_SYMBOL_RATE_E_384	9
#define PACKET_SYMBOL_RATE_E_96		7
#define PACKET_SYMBOL_RATE_E_24		5
#endif

/* 200 / 2 = 100 */
#define PACKET_CHAN_BW_384	((CC1200_CHAN_BW_ADC_CIC_DECFACT_12 << CC1200_CHAN_BW_ADC_CIC_DECFACT) | \
				 (16 << CC1200_CHAN_BW_BB_CIC_DECFACT))

/*
 * CC1201 doesn't support our low bandwidth receive setups, so we use
 * larger values for that part, leaving the bandwidth at over 50kHz
 */

/* 200 / 10 = 20 */
#define PACKET_CHAN_BW_96_CC1200	((CC1200_CHAN_BW_ADC_CIC_DECFACT_48 << CC1200_CHAN_BW_ADC_CIC_DECFACT) | \
					 (16 << CC1200_CHAN_BW_BB_CIC_DECFACT))

#define PACKET_CHAN_BW_96_CC1201	((CC1200_CHAN_BW_ADC_CIC_DECFACT_48 << CC1200_CHAN_BW_ADC_CIC_DECFACT) | \
					 (8 << CC1200_CHAN_BW_BB_CIC_DECFACT))

/* 200 / 25 = 8 */
#define PACKET_CHAN_BW_24_CC1200	((CC1200_CHAN_BW_ADC_CIC_DECFACT_48 << CC1200_CHAN_BW_ADC_CIC_DECFACT) | \
					 (44 << CC1200_CHAN_BW_BB_CIC_DECFACT))

#define PACKET_CHAN_BW_24_CC1201	((CC1200_CHAN_BW_ADC_CIC_DECFACT_48 << CC1200_CHAN_BW_ADC_CIC_DECFACT) | \
					 (8 << CC1200_CHAN_BW_BB_CIC_DECFACT))

static const uint16_t packet_setup[] = {
	CC1200_SYMBOL_RATE1,		((PACKET_SYMBOL_RATE_M >> 8) & 0xff),
	CC1200_SYMBOL_RATE0,		((PACKET_SYMBOL_RATE_M >> 0) & 0xff),
        CC1200_PKT_CFG2,                            	 /* Packet Configuration Reg. 2 */
		((0 << CC1200_PKT_CFG2_FG_MODE_EN) |
		 (CC1200_PKT_CFG2_CCA_MODE_ALWAYS_CLEAR << CC1200_PKT_CFG2_CCA_MODE) |
		 (CC1200_PKT_CFG2_PKT_FORMAT_NORMAL << CC1200_PKT_CFG2_PKT_FORMAT)),
        CC1200_PKT_CFG1,                                 /* Packet Configuration Reg. 1 */
		((1 << CC1200_PKT_CFG1_FEC_EN) |
		 (1 << CC1200_PKT_CFG1_WHITE_DATA) |
		 (0 << CC1200_PKT_CFG1_PN9_SWAP_EN) |
		 (CC1200_PKT_CFG1_ADDR_CHECK_CFG_NONE << CC1200_PKT_CFG1_ADDR_CHECK_CFG) |
		 (CC1200_PKT_CFG1_CRC_CFG_CRC16_INIT_ONES << CC1200_PKT_CFG1_CRC_CFG) |
		 (1 << CC1200_PKT_CFG1_APPEND_STATUS)),
        CC1200_PREAMBLE_CFG1,	((CC1200_PREAMBLE_CFG1_NUM_PREAMBLE_4_BYTES << CC1200_PREAMBLE_CFG1_NUM_PREAMBLE) |
				 (CC1200_PREAMBLE_CFG1_PREAMBLE_WORD_AA << CC1200_PREAMBLE_CFG1_PREAMBLE_WORD)),
};

static const uint16_t packet_setup_384[] = {
	CC1200_DEVIATION_M,	PACKET_DEV_M_384,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (PACKET_DEV_E_384 << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,	((PACKET_SYMBOL_RATE_E_384 << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((PACKET_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
	CC1200_CHAN_BW,		PACKET_CHAN_BW_384,
        CC1200_MDMCFG2,                                  /* General Modem Parameter Configuration Reg. 2 */
		((CC1200_MDMCFG2_ASK_SHAPE_8 << CC1200_MDMCFG2_ASK_SHAPE) |
		 (CC1200_MDMCFG2_SYMBOL_MAP_CFG_MODE_0 << CC1200_MDMCFG2_SYMBOL_MAP_CFG) |
		 (CC1200_MDMCFG2_UPSAMPLER_P_8 << CC1200_MDMCFG2_UPSAMPLER_P) |
		 (0 << CC1200_MDMCFG2_CFM_DATA_EN)),
};

static const uint16_t packet_setup_96[] = {
	CC1200_DEVIATION_M,	PACKET_DEV_M_96,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (PACKET_DEV_E_96 << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,	((PACKET_SYMBOL_RATE_E_96 << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((PACKET_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
        CC1200_MDMCFG2,                                  /* General Modem Parameter Configuration Reg. 2 */
		((CC1200_MDMCFG2_ASK_SHAPE_8 << CC1200_MDMCFG2_ASK_SHAPE) |
		 (CC1200_MDMCFG2_SYMBOL_MAP_CFG_MODE_0 << CC1200_MDMCFG2_SYMBOL_MAP_CFG) |
		 (CC1200_MDMCFG2_UPSAMPLER_P_32 << CC1200_MDMCFG2_UPSAMPLER_P) |
		 (0 << CC1200_MDMCFG2_CFM_DATA_EN)),
};

static const uint16_t packet_setup_24[] = {
	CC1200_DEVIATION_M,	PACKET_DEV_M_24,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (PACKET_DEV_E_24 << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,	((PACKET_SYMBOL_RATE_E_24 << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((PACKET_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
        CC1200_MDMCFG2,                                  /* General Modem Parameter Configuration Reg. 2 */
		((CC1200_MDMCFG2_ASK_SHAPE_8 << CC1200_MDMCFG2_ASK_SHAPE) |
		 (CC1200_MDMCFG2_SYMBOL_MAP_CFG_MODE_0 << CC1200_MDMCFG2_SYMBOL_MAP_CFG) |
		 (CC1200_MDMCFG2_UPSAMPLER_P_64 << CC1200_MDMCFG2_UPSAMPLER_P) |
		 (0 << CC1200_MDMCFG2_CFM_DATA_EN)),
};

/*
 * RDF deviation is 3kHz
 *
 *	fdev = fosc >> 22 * (256 + dev_m) << dev_e	dev_e != 0
 *	fdev = fosc >> 21 * dev_m			dev_e == 0
 *
 *     	40e6 / (2 ** 21) * 157 = 2995Hz
 */

#define RDF_DEV_E	0
#define RDF_DEV_M	157

/*
 * For our RDF beacon, set the symbol rate to 2kBaud (for a 1kHz tone)
 *
 *              (2**20 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 39
 *
 *	DATARATE_M = 669411
 *	DATARATE_E = 4
 *
 * To make the tone last for 200ms, we need 2000 * .2 = 400 bits or 50 bytes
 */
#define RDF_SYMBOL_RATE_E	4
#define RDF_SYMBOL_RATE_M	669411
#define RDF_PACKET_LEN	50

static const uint16_t rdf_setup[] = {
	CC1200_DEVIATION_M,	RDF_DEV_M,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (RDF_DEV_E << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,	((RDF_SYMBOL_RATE_E << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((RDF_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
	CC1200_SYMBOL_RATE1,	((RDF_SYMBOL_RATE_M >> 8) & 0xff),
	CC1200_SYMBOL_RATE0,	((RDF_SYMBOL_RATE_M >> 0) & 0xff),
        CC1200_PKT_CFG2,                            	 /* Packet Configuration Reg. 2 */
		((0 << CC1200_PKT_CFG2_FG_MODE_EN) |
		 (CC1200_PKT_CFG2_CCA_MODE_ALWAYS_CLEAR << CC1200_PKT_CFG2_CCA_MODE) |
		 (CC1200_PKT_CFG2_PKT_FORMAT_NORMAL << CC1200_PKT_CFG2_PKT_FORMAT)),
        CC1200_PKT_CFG1,                                 /* Packet Configuration Reg. 1 */
		((0 << CC1200_PKT_CFG1_FEC_EN) |
		 (0 << CC1200_PKT_CFG1_WHITE_DATA) |
		 (0 << CC1200_PKT_CFG1_PN9_SWAP_EN) |
		 (CC1200_PKT_CFG1_ADDR_CHECK_CFG_NONE << CC1200_PKT_CFG1_ADDR_CHECK_CFG) |
		 (CC1200_PKT_CFG1_CRC_CFG_DISABLED << CC1200_PKT_CFG1_CRC_CFG) |
		 (0 << CC1200_PKT_CFG1_APPEND_STATUS)),
        CC1200_PREAMBLE_CFG1,
		((CC1200_PREAMBLE_CFG1_NUM_PREAMBLE_NONE << CC1200_PREAMBLE_CFG1_NUM_PREAMBLE) |
		 (CC1200_PREAMBLE_CFG1_PREAMBLE_WORD_AA << CC1200_PREAMBLE_CFG1_PREAMBLE_WORD)),
        CC1200_MDMCFG2,                                  /* General Modem Parameter Configuration Reg. 2 */
		((0 << CC1200_MDMCFG2_ASK_SHAPE) |
		 (0 << CC1200_MDMCFG2_SYMBOL_MAP_CFG) |
		 (12 << CC1200_MDMCFG2_UPSAMPLER_P) |
		 (0 << CC1200_MDMCFG2_CFM_DATA_EN)),
};

/*
 * APRS deviation is 3kHz
 *
 *	fdev = fosc >> 22 * (256 + dev_m) << dev_e	dev_e != 0
 *	fdev = fosc >> 21 * dev_m			dev_e == 0
 *
 *     	40e6 / (2 ** 21) * 157 = 2995Hz
 */

#define APRS_DEV_E	0
#define APRS_DEV_M	157

/*
 * For our APRS beacon, set the symbol rate to 9.6kBaud (8x oversampling for 1200 baud data rate)
 *
 *              (2**20 + DATARATE_M) * 2 ** DATARATE_E
 *	Rdata = -------------------------------------- * fosc
 *		             2 ** 39
 *
 *	DATARATE_M = 1013008
 *	DATARATE_E = 6
 *
 *	Rdata = 9599.998593330383301
 *
 */
#define APRS_SYMBOL_RATE_E	6
#define APRS_SYMBOL_RATE_M	1013008
#define APRS_BUFFER_SIZE	64

static const uint16_t aprs_setup[] = {
	CC1200_DEVIATION_M,	APRS_DEV_M,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (APRS_DEV_E << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,	((APRS_SYMBOL_RATE_E << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((APRS_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
	CC1200_SYMBOL_RATE1,	((APRS_SYMBOL_RATE_M >> 8) & 0xff),
	CC1200_SYMBOL_RATE0,	((APRS_SYMBOL_RATE_M >> 0) & 0xff),
        CC1200_PKT_CFG2,                            	 /* Packet Configuration Reg. 2 */
		((0 << CC1200_PKT_CFG2_FG_MODE_EN) |
		 (CC1200_PKT_CFG2_CCA_MODE_ALWAYS_CLEAR << CC1200_PKT_CFG2_CCA_MODE) |
		 (CC1200_PKT_CFG2_PKT_FORMAT_NORMAL << CC1200_PKT_CFG2_PKT_FORMAT)),
        CC1200_PKT_CFG1,                                 /* Packet Configuration Reg. 1 */
		((0 << CC1200_PKT_CFG1_FEC_EN) |
		 (0 << CC1200_PKT_CFG1_WHITE_DATA) |
		 (0 << CC1200_PKT_CFG1_PN9_SWAP_EN) |
		 (CC1200_PKT_CFG1_ADDR_CHECK_CFG_NONE << CC1200_PKT_CFG1_ADDR_CHECK_CFG) |
		 (CC1200_PKT_CFG1_CRC_CFG_DISABLED << CC1200_PKT_CFG1_CRC_CFG) |
		 (0 << CC1200_PKT_CFG1_APPEND_STATUS)),
        CC1200_PKT_CFG0,                                 /* Packet Configuration Reg. 0 */
		((CC1200_PKT_CFG0_LENGTH_CONFIG_FIXED << CC1200_PKT_CFG0_LENGTH_CONFIG) |
		 (0 << CC1200_PKT_CFG0_PKG_BIT_LEN) |
		 (0 << CC1200_PKT_CFG0_UART_MODE_EN) |
		 (0 << CC1200_PKT_CFG0_UART_SWAP_EN)),
        CC1200_PREAMBLE_CFG1,
		((CC1200_PREAMBLE_CFG1_NUM_PREAMBLE_NONE << CC1200_PREAMBLE_CFG1_NUM_PREAMBLE) |
		 (CC1200_PREAMBLE_CFG1_PREAMBLE_WORD_AA << CC1200_PREAMBLE_CFG1_PREAMBLE_WORD)),
        CC1200_MDMCFG2,                                  /* General Modem Parameter Configuration Reg. 2 */
		((CC1200_MDMCFG2_ASK_SHAPE_8 << CC1200_MDMCFG2_ASK_SHAPE) |
		 (CC1200_MDMCFG2_SYMBOL_MAP_CFG_MODE_0 << CC1200_MDMCFG2_SYMBOL_MAP_CFG) |
		 (CC1200_MDMCFG2_UPSAMPLER_P_8 << CC1200_MDMCFG2_UPSAMPLER_P) |
		 (0 << CC1200_MDMCFG2_CFM_DATA_EN)),
	CC1200_FIFO_CFG,
		((0 << CC1200_FIFO_CFG_CRC_AUTOFLUSH) |
		 (APRS_BUFFER_SIZE << CC1200_FIFO_CFG_FIFO_THR)),
};

/*
 * For Test mode, we want an unmodulated carrier. To do that, we
 * set the deviation to zero and enable a preamble so that the radio
 * turns on before we send any data
 */

static const uint16_t test_setup[] = {
	CC1200_DEVIATION_M,	0,
	CC1200_MODCFG_DEV_E,	((CC1200_MODCFG_DEV_E_MODEM_MODE_NORMAL << CC1200_MODCFG_DEV_E_MODEM_MODE) |
				 (CC1200_MODCFG_DEV_E_MOD_FORMAT_2_GFSK << CC1200_MODCFG_DEV_E_MOD_FORMAT) |
				 (0 << CC1200_MODCFG_DEV_E_DEV_E)),
	CC1200_SYMBOL_RATE2,		((APRS_SYMBOL_RATE_E << CC1200_SYMBOL_RATE2_DATARATE_E) |
				 (((APRS_SYMBOL_RATE_M >> 16) & CC1200_SYMBOL_RATE2_DATARATE_M_19_16_MASK) << CC1200_SYMBOL_RATE2_DATARATE_M_19_16)),
	CC1200_SYMBOL_RATE1,		((APRS_SYMBOL_RATE_M >> 8) & 0xff),
	CC1200_SYMBOL_RATE0,		((APRS_SYMBOL_RATE_M >> 0) & 0xff),
	CC1200_PKT_CFG2,	((CC1200_PKT_CFG2_CCA_MODE_ALWAYS_CLEAR << CC1200_PKT_CFG2_CCA_MODE) |
				 (CC1200_PKT_CFG2_PKT_FORMAT_NORMAL << CC1200_PKT_CFG2_PKT_FORMAT)),
	CC1200_PKT_CFG1,	((0 << CC1200_PKT_CFG1_WHITE_DATA) |
				 (CC1200_PKT_CFG1_ADDR_CHECK_CFG_NONE << CC1200_PKT_CFG1_ADDR_CHECK_CFG) |
				 (CC1200_PKT_CFG1_CRC_CFG_DISABLED << CC1200_PKT_CFG1_CRC_CFG) |
				 (0 << CC1200_PKT_CFG1_APPEND_STATUS)),
        CC1200_PREAMBLE_CFG1,	((CC1200_PREAMBLE_CFG1_NUM_PREAMBLE_4_BYTES << CC1200_PREAMBLE_CFG1_NUM_PREAMBLE) |
				 (CC1200_PREAMBLE_CFG1_PREAMBLE_WORD_AA << CC1200_PREAMBLE_CFG1_PREAMBLE_WORD)),
};

#define AO_PKT_CFG0_INFINITE ((CC1200_PKT_CFG0_LENGTH_CONFIG_INFINITE << CC1200_PKT_CFG0_LENGTH_CONFIG) | \
			      (0 << CC1200_PKT_CFG0_PKG_BIT_LEN) |	\
			      (0 << CC1200_PKT_CFG0_UART_MODE_EN) |	\
			      (0 << CC1200_PKT_CFG0_UART_SWAP_EN))

#define AO_PKT_CFG0_FIXED ((CC1200_PKT_CFG0_LENGTH_CONFIG_FIXED << CC1200_PKT_CFG0_LENGTH_CONFIG) | \
			   (0 << CC1200_PKT_CFG0_PKG_BIT_LEN) |		\
			   (0 << CC1200_PKT_CFG0_UART_MODE_EN) |	\
			   (0 << CC1200_PKT_CFG0_UART_SWAP_EN))

static uint16_t ao_radio_mode;

#define AO_RADIO_MODE_BITS_PACKET	1
#define AO_RADIO_MODE_BITS_TX_BUF	4
#define AO_RADIO_MODE_BITS_TX_FINISH	8
#define AO_RADIO_MODE_BITS_RX		16
#define AO_RADIO_MODE_BITS_RDF		32
#define AO_RADIO_MODE_BITS_APRS		64
#define AO_RADIO_MODE_BITS_TEST		128
#define AO_RADIO_MODE_BITS_INFINITE	256
#define AO_RADIO_MODE_BITS_FIXED	512

#define AO_RADIO_MODE_NONE		0
#define AO_RADIO_MODE_PACKET_TX		(AO_RADIO_MODE_BITS_PACKET | AO_RADIO_MODE_BITS_FIXED    | AO_RADIO_MODE_BITS_TX_FINISH)
#define AO_RADIO_MODE_PACKET_RX		(AO_RADIO_MODE_BITS_PACKET | AO_RADIO_MODE_BITS_FIXED    | AO_RADIO_MODE_BITS_RX)
#define AO_RADIO_MODE_RDF		(AO_RADIO_MODE_BITS_RDF    | AO_RADIO_MODE_BITS_FIXED    | AO_RADIO_MODE_BITS_TX_FINISH)
#define AO_RADIO_MODE_APRS_BUF		(AO_RADIO_MODE_BITS_APRS   | AO_RADIO_MODE_BITS_INFINITE | AO_RADIO_MODE_BITS_TX_BUF)
#define AO_RADIO_MODE_APRS_LAST_BUF	(AO_RADIO_MODE_BITS_APRS   | AO_RADIO_MODE_BITS_FIXED    | AO_RADIO_MODE_BITS_TX_BUF)
#define AO_RADIO_MODE_APRS_FINISH	(AO_RADIO_MODE_BITS_APRS   | AO_RADIO_MODE_BITS_FIXED    | AO_RADIO_MODE_BITS_TX_FINISH)
#define AO_RADIO_MODE_TEST		(AO_RADIO_MODE_BITS_TEST   | AO_RADIO_MODE_BITS_INFINITE | AO_RADIO_MODE_BITS_TX_BUF)

static void
_ao_radio_set_regs(const uint16_t *regs, int nreg)
{
	int i;

	for (i = 0; i < nreg; i++) {
		ao_radio_reg_write(regs[0], (uint8_t) regs[1]);
		regs += 2;
	}
}

#define ao_radio_set_regs(setup) _ao_radio_set_regs(setup, (sizeof (setup) / sizeof(setup[0])) >> 1)

static void
ao_radio_set_mode(uint16_t new_mode)
{
	uint16_t changes;

	if (new_mode == ao_radio_mode)
		return;

	changes = (uint16_t) (new_mode & (~ao_radio_mode));

	if (changes & AO_RADIO_MODE_BITS_PACKET) {
		ao_radio_set_regs(packet_setup);

		switch (ao_config.radio_rate) {
		default:
		case AO_RADIO_RATE_38400:
			ao_radio_set_regs(packet_setup_384);
			break;
		case AO_RADIO_RATE_9600:
			ao_radio_set_regs(packet_setup_96);
			if (cc1201)
				ao_radio_reg_write(CC1200_CHAN_BW, PACKET_CHAN_BW_96_CC1201);
			else
				ao_radio_reg_write(CC1200_CHAN_BW, PACKET_CHAN_BW_96_CC1200);
			break;
		case AO_RADIO_RATE_2400:
			ao_radio_set_regs(packet_setup_24);
			if (cc1201)
				ao_radio_reg_write(CC1200_CHAN_BW, PACKET_CHAN_BW_24_CC1201);
			else
				ao_radio_reg_write(CC1200_CHAN_BW, PACKET_CHAN_BW_24_CC1200);
			break;
		}
	}

	if (changes & AO_RADIO_MODE_BITS_TX_BUF) {
		ao_radio_reg_write(AO_CC1200_INT_GPIO_IOCFG, CC1200_IOCFG_GPIO_CFG_TXFIFO_THR);
		ao_exti_set_mode(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN, AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_HIGH);
	}

	if (changes & AO_RADIO_MODE_BITS_TX_FINISH) {
		ao_radio_reg_write(AO_CC1200_INT_GPIO_IOCFG, CC1200_IOCFG_GPIO_CFG_PKT_SYNC_RXTX);
		ao_exti_set_mode(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN, AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_HIGH);
	}

	if (changes & AO_RADIO_MODE_BITS_RX) {
		ao_radio_reg_write(AO_CC1200_INT_GPIO_IOCFG, CC1200_IOCFG_GPIO_CFG_MARC_MCU_WAKEUP);
		ao_exti_set_mode(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN, AO_EXTI_MODE_RISING|AO_EXTI_PRIORITY_HIGH);
	}

	if (changes & AO_RADIO_MODE_BITS_RDF)
		ao_radio_set_regs(rdf_setup);

	if (changes & AO_RADIO_MODE_BITS_APRS)
		ao_radio_set_regs(aprs_setup);

	if (changes & AO_RADIO_MODE_BITS_TEST)
		ao_radio_set_regs(test_setup);

	if (changes & AO_RADIO_MODE_BITS_INFINITE)
		ao_radio_reg_write(CC1200_PKT_CFG0, AO_PKT_CFG0_INFINITE);

	if (changes & AO_RADIO_MODE_BITS_FIXED)
		ao_radio_reg_write(CC1200_PKT_CFG0, AO_PKT_CFG0_FIXED);

	ao_radio_mode = new_mode;
}

static const uint16_t radio_setup[] = {
#include "ao_cc1200_CC1200.h"
};

static uint8_t	ao_radio_configured = 0;

static void
ao_radio_setup(void)
{
	uint8_t partnumber = ao_radio_reg_read(CC1200_PARTNUMBER);

	if (partnumber == CC1200_PARTNUMBER_CC1201)
		cc1201 = 1;

	ao_radio_strobe(CC1200_SRES);

	ao_radio_set_regs(radio_setup);

	ao_radio_mode = 0;

	ao_radio_idle();

	ao_config_get();

	ao_radio_configured = 1;
}

static void
ao_radio_set_len(uint8_t len)
{
	static uint8_t	last_len;

	if (len != last_len) {
		ao_radio_reg_write(CC1200_PKT_LEN, len);
		last_len = len;
	}
}

static void
ao_radio_get(uint8_t len)
{
	static uint32_t	last_radio_setting;
	static uint8_t	last_radio_rate;
	static uint8_t	last_radio_10mw;

	ao_mutex_get(&ao_radio_mutex);

	if (!ao_radio_configured)
		ao_radio_setup();
	if (ao_config.radio_setting != last_radio_setting) {
		ao_radio_reg_write(CC1200_FREQ2, (uint8_t) (ao_config.radio_setting >> 16));
		ao_radio_reg_write(CC1200_FREQ1, (uint8_t) (ao_config.radio_setting >> 8));
		ao_radio_reg_write(CC1200_FREQ0, (uint8_t) ao_config.radio_setting);
		last_radio_setting = ao_config.radio_setting;
		ao_radio_strobe(CC1200_SCAL);
	}
	if (ao_config.radio_rate != last_radio_rate) {
		ao_radio_mode &= (uint16_t) ~AO_RADIO_MODE_BITS_PACKET;
		last_radio_rate = ao_config.radio_rate;
	}
	if(ao_config.radio_10mw != last_radio_10mw) {
		last_radio_10mw = ao_config.radio_10mw;
		/*
		 * 0x37 "should" be 10dBm, but measurements on TBT
		 * v4.0 show that too hot, so use * 0x32 to make sure 
		 * we're in spec.
		 */
		if (ao_config.radio_10mw)
			ao_radio_reg_write(CC1200_PA_CFG1, 0x32);
		else
			ao_radio_reg_write(CC1200_PA_CFG1, 0x3f);
	}
	ao_radio_set_len(len);
}

#define ao_radio_put()	ao_mutex_put(&ao_radio_mutex)

static inline uint8_t
ao_radio_state(void)
{
	return (ao_radio_status() >> CC1200_STATUS_STATE) & CC1200_STATUS_STATE_MASK;
}

#if CC1200_DEBUG_
static void
ao_radio_show_state(char *where)
{
	printf("%s: state %d len %d rxbytes %d\n",
	       where, ao_radio_state(),
	       ao_radio_reg_read(CC1200_PKT_LEN),
	       ao_radio_reg_read(CC1200_NUM_RXBYTES));
}
#else
#define ao_radio_show_state(where)
#endif

/* Wait for the radio to signal an interrupt
 */
static void
ao_radio_wait_isr(AO_TICK_TYPE timeout)
{
	ao_arch_block_interrupts();
	while (!ao_radio_wake && !ao_radio_abort)
		if (ao_sleep_for(&ao_radio_wake, timeout))
			ao_radio_abort = 1;
	ao_arch_release_interrupts();
}

static void
ao_rdf_start(uint8_t len)
{
	ao_radio_abort = 0;
	ao_radio_get(len);

	ao_radio_set_mode(AO_RADIO_MODE_RDF);
	ao_radio_wake = 0;
}

static void
ao_radio_run(void)
{
	ao_radio_wake = 0;
	ao_radio_abort = 0;
	ao_radio_start_tx();
	ao_radio_wait_isr(0);
	if (!ao_radio_wake)
		ao_radio_idle();
	ao_radio_put();
}

void
ao_radio_rdf(void)
{
	ao_rdf_start(AO_RADIO_RDF_LEN);

	ao_radio_fifo_write_fixed(ao_radio_rdf_value, AO_RADIO_RDF_LEN);

	ao_radio_run();
}

void
ao_radio_continuity(uint8_t c)
{
	uint8_t	i;
	uint8_t status;

	ao_rdf_start(AO_RADIO_CONT_TOTAL_LEN);

	status = ao_radio_fifo_write_start();
	for (i = 0; i < 3; i++) {
		ao_radio_spi_send_fixed(0x00, AO_RADIO_CONT_PAUSE_LEN);
		if (i < c)
			ao_radio_spi_send_fixed(ao_radio_rdf_value, AO_RADIO_CONT_TONE_LEN);
		else
			ao_radio_spi_send_fixed(0x00, AO_RADIO_CONT_TONE_LEN);
	}
	ao_radio_spi_send_fixed(0x00, AO_RADIO_CONT_PAUSE_LEN);
	status = ao_radio_fifo_write_stop(status);
	(void) status;
	ao_radio_run();
}

void
ao_radio_rdf_abort(void)
{
	ao_radio_abort = 1;
	ao_wakeup(&ao_radio_wake);
}

static uint8_t radio_on;

void
ao_radio_test_on(void)
{
	if (!radio_on) {
#if HAS_MONITOR
		ao_monitor_disable();
#endif
#if PACKET_HAS_SLAVE
		ao_packet_slave_stop();
#endif
#if HAS_PAD
		ao_pad_disable();
#endif
		ao_radio_get(0xff);
		ao_radio_set_mode(AO_RADIO_MODE_TEST);
		ao_radio_strobe(CC1200_STX);
#if CC1200_TRACE
		{ int t;
			for (t = 0; t < 10; t++) {
				printf ("status: %02x\n", ao_radio_status());
				ao_delay(AO_MS_TO_TICKS(100));
			}
		}
#endif
		radio_on = 1;
	}
}

void
ao_radio_test_off(void)
{
	if (radio_on) {
		ao_radio_idle();
		ao_radio_put();
		radio_on = 0;
#if HAS_MONITOR
		ao_monitor_enable();
#endif
#if HAS_PAD
		ao_pad_enable();
#endif
	}
}

static void
ao_radio_test_cmd(void)
{
	uint8_t	mode = 2;
	ao_cmd_white();
	if (ao_cmd_lex_c != '\n')
		mode = (uint8_t) ao_cmd_decimal();
	mode++;
	if ((mode & 2))
		ao_radio_test_on();
	if (mode == 3) {
		printf ("Hit a character to stop..."); flush();
		getchar();
		putchar('\n');
	}
	if ((mode & 1))
		ao_radio_test_off();
}

void
ao_radio_send(const void *d, uint8_t size)
{
	ao_radio_get(size);
	ao_radio_set_mode(AO_RADIO_MODE_PACKET_TX);

	ao_radio_fifo_write(d, size);

	ao_radio_run();
}

void
ao_radio_send_aprs(ao_radio_fill_func fill)
{
	uint8_t	buf[APRS_BUFFER_SIZE];
	int	cnt;
	int	total = 0;
	uint8_t	done = 0;
	uint8_t	started = 0;

	ao_radio_abort = 0;
	ao_radio_get(0xff);
	ao_radio_wake = 0;
	while (!done && !ao_radio_abort) {
		cnt = (*fill)(buf, sizeof(buf));
		if (cnt < 0) {
			done = 1;
			cnt = -cnt;
		}
		total += cnt;

		/* At the last buffer, set the total length */
		if (done)
			ao_radio_set_len((uint8_t) (total & 0xff));

		/* Wait for some space in the fifo */
		while (started && ao_radio_int_pin() != 0 && !ao_radio_abort) {
			ao_radio_wake = 0;
			ao_exti_enable(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
			ao_radio_wait_isr(AO_MS_TO_TICKS(1000));
		}
		if (ao_radio_abort)
			break;

		if (done)
			ao_radio_set_mode(AO_RADIO_MODE_APRS_FINISH);
		else
			ao_radio_set_mode(AO_RADIO_MODE_APRS_BUF);

		ao_radio_fifo_write(buf, (uint8_t) cnt);
		if (!started) {
			ao_radio_strobe(CC1200_STX);
			started = 1;
		}
	}
	/* Wait for the transmitter to go idle */
	while (started && ao_radio_int_pin() != 0 && !ao_radio_abort) {
		ao_radio_wake = 0;
		ao_exti_enable(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN);
		ao_radio_wait_isr(AO_MS_TO_TICKS(1000));
	}
	if (ao_radio_abort)
		ao_radio_idle();
	ao_radio_put();
}

#if 0
static uint8_t
ao_radio_marc_state(void)
{
	return ao_radio_reg_read(CC1200_MARCSTATE);
}

static uint8_t
ao_radio_modem_status1(void)
{
	return ao_radio_reg_read(CC1200_MODEM_STATUS1);
}

static uint8_t
ao_radio_modem_status0(void)
{
	return ao_radio_reg_read(CC1200_MODEM_STATUS0);
}

struct ao_radio_state {
	char	where[4];
	uint8_t	marc_state;
	uint8_t marc_status1;
	uint8_t marc_status0;
	uint8_t	modem_status1;
	uint8_t	modem_status0;
};

static void
ao_radio_fill_state(char *where, struct ao_radio_state *s)
{
	strcpy(s->where, where);
	s->marc_state = ao_radio_marc_state();
	s->marc_status1 = ao_radio_reg_read(CC1200_MARC_STATUS1);
	s->marc_status0 = ao_radio_reg_read(CC1200_MARC_STATUS0);
	s->modem_status1 = ao_radio_modem_status1();
	s->modem_status0 = ao_radio_modem_status0();
}

static void
ao_radio_dump_state(struct ao_radio_state *s)
{
	printf ("%s: marc %2x marc1 %2x marc0 %2x modem1 %2x modem0 %2x\n",
		s->where, s->marc_state, s->marc_status1, s->marc_status0, s->modem_status1, s->modem_status0);
}
#endif

uint8_t
ao_radio_recv(void *d, uint8_t size, AO_TICK_TYPE timeout)
{
	uint8_t	success = 0;

	ao_radio_abort = 0;
	ao_radio_get(size - 2);
	ao_radio_set_mode(AO_RADIO_MODE_PACKET_RX);
	ao_radio_wake = 0;
	ao_radio_start_rx();

	while (!ao_radio_abort) {
		ao_radio_wait_isr(timeout);
		if (ao_radio_wake) {
			uint8_t		marc_status1 = ao_radio_reg_read(CC1200_MARC_STATUS1);

			/* Check the receiver status to see what happened
			 */
			switch (marc_status1) {
			case CC1200_MARC_STATUS1_RX_FINISHED:
			case CC1200_MARC_STATUS1_ADDRESS:
			case CC1200_MARC_STATUS1_CRC:
				/* Normal return, go fetch the bytes from the FIFO
				 * and give them back to the caller
				 */
				success = 1;
				break;
			case CC1200_MARC_STATUS1_RX_TIMEOUT:
			case CC1200_MARC_STATUS1_RX_TERMINATION:
			case CC1200_MARC_STATUS1_EWOR_SYNC_LOST:
			case CC1200_MARC_STATUS1_MAXIMUM_LENGTH:
			case CC1200_MARC_STATUS1_RX_FIFO_OVERFLOW:
			case CC1200_MARC_STATUS1_RX_FIFO_UNDERFLOW:
				/* Something weird happened; reset the radio and
				 * return failure
				 */
				success = 0;
				break;
			default:
				/* some other status; go wait for the radio to do something useful
				 */
				continue;
			}
			break;
		} else {
			uint8_t modem_status1 = ao_radio_reg_read(CC1200_MODEM_STATUS1);

			/* Check to see if the packet header has been seen, in which case we'll
			 * want to keep waiting for the rest of the packet to appear
			 */
			if (modem_status1 & (1 << CC1200_MODEM_STATUS1_SYNC_FOUND))
			{
				ao_radio_abort = 0;

				/* Set a timeout based on the packet length so that we make sure to
				 * wait long enough to receive the whole thing.
				 *
				 * timeout = bits * FEC expansion / rate
				 */
				switch (ao_config.radio_rate) {
				default:
				case AO_RADIO_RATE_38400:
					timeout = AO_MS_TO_TICKS((AO_TICK_TYPE) size * (8 * 2 * 10) / 384) + 1;
					break;
				case AO_RADIO_RATE_9600:
					timeout = AO_MS_TO_TICKS((AO_TICK_TYPE) size * (8 * 2 * 10) / 96) + 1;
					break;
				case AO_RADIO_RATE_2400:
					timeout = AO_MS_TO_TICKS((AO_TICK_TYPE) size * (8 * 2 * 10) / 24) + 1;
					break;
				}
			}
		}
	}

	if (success) {
		int8_t	rssi;
		uint8_t	status;

		status = ao_radio_fifo_read(d, size);
		(void) status;
		rssi = ((int8_t *) d)[size - 2];
		ao_radio_rssi = rssi;

		/* Bound it to the representable range */
		if (rssi > -11)
			rssi = -11;

		/* Write it back to the packet */
		((uint8_t *) d)[size-2] = AO_RADIO_FROM_RSSI(rssi);
	} else {
		ao_radio_idle();
		ao_radio_rssi = 0;
	}

	ao_radio_put();
	return success;
}


#if CC1200_DEBUG
static char *cc1200_state_name[] = {
	[CC1200_STATUS_STATE_IDLE] = "IDLE",
	[CC1200_STATUS_STATE_RX] = "RX",
	[CC1200_STATUS_STATE_TX] = "TX",
	[CC1200_STATUS_STATE_FSTXON] = "FSTXON",
	[CC1200_STATUS_STATE_CALIBRATE] = "CALIBRATE",
	[CC1200_STATUS_STATE_SETTLING] = "SETTLING",
	[CC1200_STATUS_STATE_RX_FIFO_ERROR] = "RX_FIFO_ERROR",
	[CC1200_STATUS_STATE_TX_FIFO_ERROR] = "TX_FIFO_ERROR",
};

struct ao_cc1200_reg {
	uint16_t	addr;
	char		*name;
};

static const struct ao_cc1200_reg ao_cc1200_reg[] = {
	{ .addr = CC1200_IOCFG3,	.name = "IOCFG3" },
	{ .addr = CC1200_IOCFG2,	.name = "IOCFG2" },
	{ .addr = CC1200_IOCFG1,	.name = "IOCFG1" },
	{ .addr = CC1200_IOCFG0,	.name = "IOCFG0" },
	{ .addr = CC1200_SYNC3,	.name = "SYNC3" },
	{ .addr = CC1200_SYNC2,	.name = "SYNC2" },
	{ .addr = CC1200_SYNC1,	.name = "SYNC1" },
	{ .addr = CC1200_SYNC0,	.name = "SYNC0" },
	{ .addr = CC1200_SYNC_CFG1,	.name = "SYNC_CFG1" },
	{ .addr = CC1200_SYNC_CFG0,	.name = "SYNC_CFG0" },
	{ .addr = CC1200_DEVIATION_M,	.name = "DEVIATION_M" },
	{ .addr = CC1200_MODCFG_DEV_E,	.name = "MODCFG_DEV_E" },
	{ .addr = CC1200_DCFILT_CFG,	.name = "DCFILT_CFG" },
	{ .addr = CC1200_PREAMBLE_CFG1,	.name = "PREAMBLE_CFG1" },
	{ .addr = CC1200_PREAMBLE_CFG0,	.name = "PREAMBLE_CFG0" },
	{ .addr = CC1200_IQIC,	.name = "IQIC" },
	{ .addr = CC1200_CHAN_BW,	.name = "CHAN_BW" },
	{ .addr = CC1200_MDMCFG2,	.name = "MDMCFG2" },
	{ .addr = CC1200_MDMCFG1,	.name = "MDMCFG1" },
	{ .addr = CC1200_MDMCFG0,	.name = "MDMCFG0" },
	{ .addr = CC1200_SYMBOL_RATE2,	.name = "SYMBOL_RATE2" },
	{ .addr = CC1200_SYMBOL_RATE1,	.name = "SYMBOL_RATE1" },
	{ .addr = CC1200_SYMBOL_RATE0,	.name = "SYMBOL_RATE0" },
	{ .addr = CC1200_AGC_REF,	.name = "AGC_REF" },
	{ .addr = CC1200_AGC_CS_THR,	.name = "AGC_CS_THR" },
	{ .addr = CC1200_AGC_GAIN_ADJUST,	.name = "AGC_GAIN_ADJUST" },
	{ .addr = CC1200_AGC_CFG3,	.name = "AGC_CFG3" },
	{ .addr = CC1200_AGC_CFG2,	.name = "AGC_CFG2" },
	{ .addr = CC1200_AGC_CFG1,	.name = "AGC_CFG1" },
	{ .addr = CC1200_AGC_CFG0,	.name = "AGC_CFG0" },
	{ .addr = CC1200_FIFO_CFG,	.name = "FIFO_CFG" },
	{ .addr = CC1200_DEV_ADDR,	.name = "DEV_ADDR" },
	{ .addr = CC1200_SETTLING_CFG,	.name = "SETTLING_CFG" },
	{ .addr = CC1200_FS_CFG,	.name = "FS_CFG" },
	{ .addr = CC1200_WOR_CFG1,	.name = "WOR_CFG1" },
	{ .addr = CC1200_WOR_CFG0,	.name = "WOR_CFG0" },
	{ .addr = CC1200_WOR_EVENT0_MSB,	.name = "WOR_EVENT0_MSB" },
	{ .addr = CC1200_WOR_EVENT0_LSB,	.name = "WOR_EVENT0_LSB" },
	{ .addr = CC1200_RXDCM_TIME,		.name = "RXDCM_TIME" },
	{ .addr = CC1200_PKT_CFG2,	.name = "PKT_CFG2" },
	{ .addr = CC1200_PKT_CFG1,	.name = "PKT_CFG1" },
	{ .addr = CC1200_PKT_CFG0,	.name = "PKT_CFG0" },
	{ .addr = CC1200_RFEND_CFG1,	.name = "RFEND_CFG1" },
	{ .addr = CC1200_RFEND_CFG0,	.name = "RFEND_CFG0" },
	{ .addr = CC1200_PA_CFG1,	.name = "PA_CFG1" },
	{ .addr = CC1200_PA_CFG0,	.name = "PA_CFG0" },
	{ .addr = CC1200_PKT_LEN,	.name = "PKT_LEN" },
	{ .addr = CC1200_IF_MIX_CFG,	.name = "IF_MIX_CFG" },
	{ .addr = CC1200_FREQOFF_CFG,	.name = "FREQOFF_CFG" },
	{ .addr = CC1200_TOC_CFG,	.name = "TOC_CFG" },
	{ .addr = CC1200_MARC_SPARE,	.name = "MARC_SPARE" },
	{ .addr = CC1200_ECG_CFG,	.name = "ECG_CFG" },
	{ .addr = CC1200_EXT_CTRL,	.name = "EXT_CTRL" },
	{ .addr = CC1200_RCCAL_FINE,	.name = "RCCAL_FINE" },
	{ .addr = CC1200_RCCAL_COARSE,	.name = "RCCAL_COARSE" },
	{ .addr = CC1200_RCCAL_OFFSET,	.name = "RCCAL_OFFSET" },
	{ .addr = CC1200_FREQOFF1,	.name = "FREQOFF1" },
	{ .addr = CC1200_FREQOFF0,	.name = "FREQOFF0" },
	{ .addr = CC1200_FREQ2,	.name = "FREQ2" },
	{ .addr = CC1200_FREQ1,	.name = "FREQ1" },
	{ .addr = CC1200_FREQ0,	.name = "FREQ0" },
	{ .addr = CC1200_IF_ADC2,	.name = "IF_ADC2" },
	{ .addr = CC1200_IF_ADC1,	.name = "IF_ADC1" },
	{ .addr = CC1200_IF_ADC0,	.name = "IF_ADC0" },
	{ .addr = CC1200_FS_DIG1,	.name = "FS_DIG1" },
	{ .addr = CC1200_FS_DIG0,	.name = "FS_DIG0" },
	{ .addr = CC1200_FS_CAL3,	.name = "FS_CAL3" },
	{ .addr = CC1200_FS_CAL2,	.name = "FS_CAL2" },
	{ .addr = CC1200_FS_CAL1,	.name = "FS_CAL1" },
	{ .addr = CC1200_FS_CAL0,	.name = "FS_CAL0" },
	{ .addr = CC1200_FS_CHP,	.name = "FS_CHP" },
	{ .addr = CC1200_FS_DIVTWO,	.name = "FS_DIVTWO" },
	{ .addr = CC1200_FS_DSM1,	.name = "FS_DSM1" },
	{ .addr = CC1200_FS_DSM0,	.name = "FS_DSM0" },
	{ .addr = CC1200_FS_DVC1,	.name = "FS_DVC1" },
	{ .addr = CC1200_FS_DVC0,	.name = "FS_DVC0" },
	{ .addr = CC1200_FS_LBI,	.name = "FS_LBI" },
	{ .addr = CC1200_FS_PFD,	.name = "FS_PFD" },
	{ .addr = CC1200_FS_PRE,	.name = "FS_PRE" },
	{ .addr = CC1200_FS_REG_DIV_CML,	.name = "FS_REG_DIV_CML" },
	{ .addr = CC1200_FS_SPARE,	.name = "FS_SPARE" },
	{ .addr = CC1200_FS_VCO4,	.name = "FS_VCO4" },
	{ .addr = CC1200_FS_VCO3,	.name = "FS_VCO3" },
	{ .addr = CC1200_FS_VCO2,	.name = "FS_VCO2" },
	{ .addr = CC1200_FS_VCO1,	.name = "FS_VCO1" },
	{ .addr = CC1200_FS_VCO0,	.name = "FS_VCO0" },
	{ .addr = CC1200_GBIAS6,	.name = "GBIAS6" },
	{ .addr = CC1200_GBIAS5,	.name = "GBIAS5" },
	{ .addr = CC1200_GBIAS4,	.name = "GBIAS4" },
	{ .addr = CC1200_GBIAS3,	.name = "GBIAS3" },
	{ .addr = CC1200_GBIAS2,	.name = "GBIAS2" },
	{ .addr = CC1200_GBIAS1,	.name = "GBIAS1" },
	{ .addr = CC1200_GBIAS0,	.name = "GBIAS0" },
	{ .addr = CC1200_IFAMP,	.name = "IFAMP" },
	{ .addr = CC1200_LNA,	.name = "LNA" },
	{ .addr = CC1200_RXMIX,	.name = "RXMIX" },
	{ .addr = CC1200_XOSC5,	.name = "XOSC5" },
	{ .addr = CC1200_XOSC4,	.name = "XOSC4" },
	{ .addr = CC1200_XOSC3,	.name = "XOSC3" },
	{ .addr = CC1200_XOSC2,	.name = "XOSC2" },
	{ .addr = CC1200_XOSC1,	.name = "XOSC1" },
	{ .addr = CC1200_XOSC0,	.name = "XOSC0" },
	{ .addr = CC1200_ANALOG_SPARE,	.name = "ANALOG_SPARE" },
	{ .addr = CC1200_PA_CFG3,	.name = "PA_CFG3" },
	{ .addr = CC1200_WOR_TIME1,	.name = "WOR_TIME1" },
	{ .addr = CC1200_WOR_TIME0,	.name = "WOR_TIME0" },
	{ .addr = CC1200_WOR_CAPTURE1,	.name = "WOR_CAPTURE1" },
	{ .addr = CC1200_WOR_CAPTURE0,	.name = "WOR_CAPTURE0" },
	{ .addr = CC1200_BIST,	.name = "BIST" },
	{ .addr = CC1200_DCFILTOFFSET_I1,	.name = "DCFILTOFFSET_I1" },
	{ .addr = CC1200_DCFILTOFFSET_I0,	.name = "DCFILTOFFSET_I0" },
	{ .addr = CC1200_DCFILTOFFSET_Q1,	.name = "DCFILTOFFSET_Q1" },
	{ .addr = CC1200_DCFILTOFFSET_Q0,	.name = "DCFILTOFFSET_Q0" },
	{ .addr = CC1200_IQIE_I1,	.name = "IQIE_I1" },
	{ .addr = CC1200_IQIE_I0,	.name = "IQIE_I0" },
	{ .addr = CC1200_IQIE_Q1,	.name = "IQIE_Q1" },
	{ .addr = CC1200_IQIE_Q0,	.name = "IQIE_Q0" },
	{ .addr = CC1200_RSSI1,	.name = "RSSI1" },
	{ .addr = CC1200_RSSI0,	.name = "RSSI0" },
	{ .addr = CC1200_MARCSTATE,	.name = "MARCSTATE" },
	{ .addr = CC1200_LQI_VAL,	.name = "LQI_VAL" },
	{ .addr = CC1200_PQT_SYNC_ERR,	.name = "PQT_SYNC_ERR" },
	{ .addr = CC1200_DEM_STATUS,	.name = "DEM_STATUS" },
	{ .addr = CC1200_FREQOFF_EST1,	.name = "FREQOFF_EST1" },
	{ .addr = CC1200_FREQOFF_EST0,	.name = "FREQOFF_EST0" },
	{ .addr = CC1200_AGC_GAIN3,	.name = "AGC_GAIN3" },
	{ .addr = CC1200_AGC_GAIN2,	.name = "AGC_GAIN2" },
	{ .addr = CC1200_AGC_GAIN1,	.name = "AGC_GAIN1" },
	{ .addr = CC1200_AGC_GAIN0,	.name = "AGC_GAIN0" },
	{ .addr = CC1200_SOFT_RX_DATA_OUT,	.name = "SOFT_RX_DATA_OUT" },
	{ .addr = CC1200_SOFT_TX_DATA_IN,	.name = "SOFT_TX_DATA_IN" },
	{ .addr = CC1200_ASK_SOFT_RX_DATA,	.name = "ASK_SOFT_RX_DATA" },
	{ .addr = CC1200_RNDGEN,	.name = "RNDGEN" },
	{ .addr = CC1200_MAGN2,	.name = "MAGN2" },
	{ .addr = CC1200_MAGN1,	.name = "MAGN1" },
	{ .addr = CC1200_MAGN0,	.name = "MAGN0" },
	{ .addr = CC1200_ANG1,	.name = "ANG1" },
	{ .addr = CC1200_ANG0,	.name = "ANG0" },
	{ .addr = CC1200_CHFILT_I2,	.name = "CHFILT_I2" },
	{ .addr = CC1200_CHFILT_I1,	.name = "CHFILT_I1" },
	{ .addr = CC1200_CHFILT_I0,	.name = "CHFILT_I0" },
	{ .addr = CC1200_CHFILT_Q2,	.name = "CHFILT_Q2" },
	{ .addr = CC1200_CHFILT_Q1,	.name = "CHFILT_Q1" },
	{ .addr = CC1200_CHFILT_Q0,	.name = "CHFILT_Q0" },
	{ .addr = CC1200_GPIO_STATUS,	.name = "GPIO_STATUS" },
	{ .addr = CC1200_FSCAL_CTRL,	.name = "FSCAL_CTRL" },
	{ .addr = CC1200_PHASE_ADJUST,	.name = "PHASE_ADJUST" },
	{ .addr = CC1200_PARTNUMBER,	.name = "PARTNUMBER" },
	{ .addr = CC1200_PARTVERSION,	.name = "PARTVERSION" },
	{ .addr = CC1200_SERIAL_STATUS,	.name = "SERIAL_STATUS" },
	{ .addr = CC1200_MODEM_STATUS1,	.name = "MODEM_STATUS1" },
	{ .addr = CC1200_MODEM_STATUS0,	.name = "MODEM_STATUS0" },
	{ .addr = CC1200_MARC_STATUS1,	.name = "MARC_STATUS1" },
	{ .addr = CC1200_MARC_STATUS0,	.name = "MARC_STATUS0" },
	{ .addr = CC1200_PA_IFAMP_TEST,	.name = "PA_IFAMP_TEST" },
	{ .addr = CC1200_FSRF_TEST,	.name = "FSRF_TEST" },
	{ .addr = CC1200_PRE_TEST,	.name = "PRE_TEST" },
	{ .addr = CC1200_PRE_OVR,	.name = "PRE_OVR" },
	{ .addr = CC1200_ADC_TEST,	.name = "ADC_TEST" },
	{ .addr = CC1200_DVC_TEST,	.name = "DVC_TEST" },
	{ .addr = CC1200_ATEST,	.name = "ATEST" },
	{ .addr = CC1200_ATEST_LVDS,	.name = "ATEST_LVDS" },
	{ .addr = CC1200_ATEST_MODE,	.name = "ATEST_MODE" },
	{ .addr = CC1200_XOSC_TEST1,	.name = "XOSC_TEST1" },
	{ .addr = CC1200_XOSC_TEST0,	.name = "XOSC_TEST0" },
	{ .addr = CC1200_RXFIRST,	.name = "RXFIRST" },
	{ .addr = CC1200_TXFIRST,	.name = "TXFIRST" },
	{ .addr = CC1200_RXLAST,	.name = "RXLAST" },
	{ .addr = CC1200_TXLAST,	.name = "TXLAST" },
	{ .addr = CC1200_NUM_TXBYTES,	.name = "NUM_TXBYTES" },
	{ .addr = CC1200_NUM_RXBYTES,	.name = "NUM_RXBYTES" },
	{ .addr = CC1200_FIFO_NUM_TXBYTES,	.name = "FIFO_NUM_TXBYTES" },
	{ .addr = CC1200_FIFO_NUM_RXBYTES,	.name = "FIFO_NUM_RXBYTES" },
};

#define AO_NUM_CC1200_REG	(sizeof ao_cc1200_reg / sizeof ao_cc1200_reg[0])

static uint8_t
ao_radio_get_marc_status(void)
{
	return ao_radio_reg_read(CC1200_MARC_STATUS1);
}

static void ao_radio_show(void) {
	uint8_t	status;
	unsigned int	i;

	ao_mutex_get(&ao_radio_mutex);
	status = ao_radio_status();
	printf ("Status:   %02x\n", status);
	printf ("CHIP_RDY: %d\n", (status >> CC1200_STATUS_CHIP_RDY) & 1);
	printf ("STATE:    %s\n", cc1200_state_name[(status >> CC1200_STATUS_STATE) & CC1200_STATUS_STATE_MASK]);
	printf ("MARC:     %02x\n", ao_radio_get_marc_status());

	for (i = 0; i < AO_NUM_CC1200_REG; i++)
		printf ("\t%02x %-20.20s\n", ao_radio_reg_read(ao_cc1200_reg[i].addr), ao_cc1200_reg[i].name);

	ao_radio_put();
}

static void ao_radio_beep(void) {
	ao_radio_rdf();
}

static void ao_radio_packet(void) {
	static const uint8_t packet[] = {
#if 1
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
#else
		3, 1, 2, 3
#endif
	};

	ao_radio_send(packet, sizeof (packet));
}

static void
ao_radio_test_recv(void)
{
	static uint8_t	bytes[34];
	uint8_t	b;

	if (ao_radio_recv(bytes, 34, 0)) {
		if (bytes[33] & 0x80)
			printf ("CRC OK");
		else
			printf ("CRC BAD");
		printf (" RSSI %d", AO_RSSI_FROM_RADIO(bytes[32]));
		for (b = 0; b < 32; b++)
			printf (" %02x", bytes[b]);

		printf (" RSSI %02x LQI %02x", bytes[32], bytes[33]);
		printf ("\n");
	}
}

#if HAS_APRS
#include <ao_aprs.h>

static void
ao_radio_aprs(void)
{
#if PACKET_HAS_SLAVE
	ao_packet_slave_stop();
#endif
	ao_aprs_send();
}
#endif
#endif

#if CC1200_LOW_LEVEL_DEBUG
static void
ao_radio_strobe_test(void)
{
	uint8_t addr;
	uint8_t	r;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	r = ao_radio_strobe(addr);
	printf ("Strobe %02x -> %02x (rdy %d state %d)\n",
		addr,
		r,
		r >> 7,
		(r >> 4) & 0x7);
}

static void
ao_radio_write_test(void)
{
	uint16_t	addr;
	uint8_t		data;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	data = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	printf ("Write %04x = %02x\n", addr, data);
	ao_radio_reg_write(addr, data);
}

static void
ao_radio_read_test(void)
{
	uint16_t	addr;
	uint8_t		data;

	addr = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	data = ao_radio_reg_read(addr);
	printf ("Read %04x = %02x\n", addr, data);
}
#endif

static const struct ao_cmds ao_radio_cmds[] = {
	{ ao_radio_test_cmd,	"C <1 start, 0 stop, none both>\0Radio carrier test" },
#if CC1200_DEBUG
#if HAS_APRS
	{ ao_radio_aprs,	"G\0Send APRS packet" },
#endif
	{ ao_radio_show,	"R\0Show CC1200 status" },
	{ ao_radio_beep,	"b\0Emit an RDF beacon" },
	{ ao_radio_packet,	"p\0Send a test packet" },
	{ ao_radio_test_recv,	"q\0Recv a test packet" },
#endif
#if CC1200_LOW_LEVEL_DEBUG
	{ ao_radio_strobe_test,	"A <value>\0Strobe radio" },
	{ ao_radio_write_test,	"W <addr> <value>\0Write radio reg" },
	{ ao_radio_read_test,	"B <addr>\0Read radio reg" },
#endif
	{ 0, NULL }
};

void
ao_radio_init(void)
{
	ao_radio_configured = 0;
	ao_spi_init_cs (AO_CC1200_SPI_CS_PORT, (1 << AO_CC1200_SPI_CS_PIN));

#if 0
	AO_CC1200_SPI_CS_PORT->bsrr = ((uint32_t) (1 << AO_CC1200_SPI_CS_PIN));
	for (i = 0; i < 10000; i++) {
		if ((SPI_2_PORT->idr & (1 << SPI_2_MISO_PIN)) == 0)
			break;
	}
	AO_CC1200_SPI_CS_PORT->bsrr = (1 << AO_CC1200_SPI_CS_PIN);
	if (i == 10000)
		ao_panic(AO_PANIC_SELF_TEST_CC1200);
#endif

	/* Enable the EXTI interrupt for the appropriate pin */
	ao_enable_port(AO_CC1200_INT_PORT);
	ao_exti_setup(AO_CC1200_INT_PORT, AO_CC1200_INT_PIN,
		      AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_HIGH,
		      ao_radio_isr);

	ao_cmd_register(&ao_radio_cmds[0]);
}
