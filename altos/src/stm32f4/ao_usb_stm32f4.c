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

static uint32_t	grxstsp;

static inline uint8_t
grxstsp_enum(void)
{
	return (grxstsp >> STM_USB_GRXSTSP_EPNUM) & STM_USB_GRXSTSP_EPNUM_MASK;
}

static inline uint8_t
grxstsp_pktsts(void)
{
	return (grxstsp >> STM_USB_GRXSTSP_PKTSTS) & STM_USB_GRXSTSP_PKTSTS_MASK;
}

static inline uint16_t
grxstsp_bcnt(void)
{
	return (grxstsp >> STM_USB_GRXSTSP_BCNT) & STM_USB_GRXSTSP_BCNT_MASK;
}

static void
ao_usb_dev_ep_out_start(uint8_t ep)
{
	stm_usb.doep[ep].doeptsiz = ((1 << STM_USB_DOEPTSIZ_PKTCNT) |
				     (3 << STM_USB_DOEPTSIZ_STUPCNT) |
				     (24 << STM_USB_DOEPTSIZ_XFRSIZ));

//	stm_usb.doep[ep].doepctl |= (1 << STM_USB_DOEPCTL_EPENA);
}

static void
ao_usb_mask_in_bits(vuint32_t *addr, uint32_t shift, uint32_t mask, uint32_t bits)
{
	uint32_t	value;

	value = *addr;
	value &= ~(mask << shift);
	value |= (bits << shift);
	*addr = value;
}

static void
ao_usb_activate_ep0(void)
{
	stm_usb.diep[0].diepctl = ((0 << STM_USB_DIEPCTL_TXFNUM) |
				   (0 << STM_USB_DIEPCTL_STALL) |
				   (STM_USB_DIEPCTL_EPTYP_CONTROL << STM_USB_DIEPCTL_EPTYP) |
				   (1 << STM_USB_DIEPCTL_USBAEP) |
				   (STM_USB_DIEPCTL_MPSIZ0_64 << STM_USB_DIEPCTL_MPSIZ));
	stm_usb.doep[0].doepctl = ((0 << STM_USB_DOEPCTL_SNPM) |
				   (STM_USB_DOEPCTL_EPTYP_CONTROL << STM_USB_DOEPCTL_EPTYP) |
				   (1 << STM_USB_DOEPCTL_USBAEP) |
				   (STM_USB_DOEPCTL_MPSIZ0_64 << STM_USB_DOEPCTL_MPSIZ));
}

#if 0
static void
ao_usb_activate_in(int epnum)
{
	stm_usb.daintmsk |= (1 << (epnum + STM_USB_DAINTMSK_IEPM));
	stm_usb.diep[epnum].diepctl = ((epnum << STM_USB_DIEPCTL_TXFNUM) |
				       (0 << STM_USB_DIEPCTL_STALL) |
				       (STM_USB_DIEPCTL_EPTYP_BULK << STM_USB_DIEPCTL_EPTYP) |
				       (1 << STM_USB_DIEPCTL_USBAEP) |
				       (64 << STM_USB_DIEPCTL_MPSIZ));
}

static void
ao_usb_activate_out(int epnum)
{
	stm_usb.daintmsk |= (1 << (epnum + STM_USB_DAINTMSK_OEPM));
	stm_usb.doep[epnum].doepctl = ((0 << STM_USB_DOEPCTL_SNPM) |
				       (STM_USB_DOEPCTL_EPTYP_BULK << STM_USB_DOEPCTL_EPTYP) |
				       (1 << STM_USB_DOEPCTL_USBAEP) |
				       (64 << STM_USB_DOEPCTL_MPSIZ));
}
#endif

static void
ao_usb_enum_done(void)
{
	/* Set turn-around delay. 6 is for high hclk (> 32MHz) */
	ao_usb_mask_in_bits(&stm_usb.gusbcfg, STM_USB_GUSBCFG_TRDT, STM_USB_GUSBCFG_TRDT_MASK, 6);

	ao_usb_activate_ep0();
}

static void
ao_usb_flush_tx_fifo(uint32_t fifo)
{
	stm_usb.grstctl = ((1 << STM_USB_GRSTCTL_TXFFLSH) |
			   (fifo << STM_USB_GRSTCTL_TXFNUM));
	while ((stm_usb.grstctl & (1 << STM_USB_GRSTCTL_TXFFLSH)) != 0)
		ao_arch_nop();
}

static void
ao_usb_flush_rx_fifo(void)
{
	stm_usb.grstctl = (1 << STM_USB_GRSTCTL_RXFFLSH);
	while ((stm_usb.grstctl & (1 << STM_USB_GRSTCTL_RXFFLSH)) != 0)
		ao_arch_nop();
}

/* reset and enable EP0 */
void
ao_usb_dev_ep0_init(void)
{
	uint32_t	diepctl;

	/* Flush TX fifo */
	ao_usb_flush_tx_fifo(STM_USB_GRSTCTL_TXFNUM_ALL);

	/* Clear interrupts */
	for (int i = 0; i < 6; i++) {
		stm_usb.diep[i].diepint = 0xfffffffful;
		stm_usb.doep[i].doepint = 0xfffffffful;
	}
	stm_usb.daint = 0xfffffffful;

	/* Enable EP0 in/out interrupts */
	/* 2. Unmask interrupt bits */
	stm_usb.daintmsk |= ((1 << (STM_USB_DAINTMSK_IEPM + 0)) |
			     (1 << (STM_USB_DAINTMSK_OEPM + 0)));

	stm_usb.doepmsk |= ((1 << STM_USB_DOEPMSK_STUPM) |
			    (1 << STM_USB_DOEPMSK_EPDM) |
			    (1 << STM_USB_DOEPMSK_XFRCM));
	stm_usb.diepmsk |= ((1 << STM_USB_DIEPMSK_TOM) |
			    (1 << STM_USB_DIEPMSK_XFRCM) |
			    (1 << STM_USB_DIEPMSK_EPDM));

	/* 1. Set NAK bit for all OUT endpoints */
	stm_usb.doep[0].doepctl |= (1 << STM_USB_DOEPCTL_CNAK);
	for (int i = 1; i < 6; i++)
		stm_usb.doep[i].doepctl |= (1 << STM_USB_DOEPCTL_SNAK);

	/* 3. Setup FIFO ram allocation */

	/* XXX make principled decisions here */
	stm_usb.grxfsiz = 0x80;

	stm_usb.dieptxf0 = ((0x40 << STM_USB_DIEPTXF0_TX0FD) |		/* size = 256 bytes */
			    (0x80 << STM_USB_DIEPTXF0_TX0FSA));		/* start address = 0x80 */

	/* 4. Program OUT endpoint 0 to receive a SETUP packet */

	uint32_t	doeptsiz;

	doeptsiz = ((1 << STM_USB_DOEPTSIZ_PKTCNT) |
		    (0x40 << STM_USB_DOEPTSIZ_XFRSIZ) |
		    (1 << STM_USB_DOEPTSIZ_STUPCNT));

	stm_usb.doep[0].doeptsiz = doeptsiz;

	/* Program MPSIZ field to set maximum packet size */

	diepctl = ((0 << STM_USB_DIEPCTL_EPENA ) |
		   (0 << STM_USB_DIEPCTL_EPDIS ) |
		   (0 << STM_USB_DIEPCTL_SNAK ) |
		   (0 << STM_USB_DIEPCTL_CNAK ) |
		   (0 << STM_USB_DIEPCTL_TXFNUM) |
		   (0 << STM_USB_DIEPCTL_STALL ) |
		   (STM_USB_DIEPCTL_EPTYP_CONTROL << STM_USB_DIEPCTL_EPTYP ) |
		   (0 << STM_USB_DIEPCTL_NAKSTS ) |
		   (0 << STM_USB_DIEPCTL_EONUM ) |
		   (1 << STM_USB_DIEPCTL_USBAEP ) |
		   (STM_USB_DIEPCTL_MPSIZ0_64 << STM_USB_DIEPCTL_MPSIZ));

	stm_usb.diep[0].diepctl = diepctl;

	uint32_t	doepctl;

	doepctl = ((0 << STM_USB_DOEPCTL_EPENA ) |
		   (0 << STM_USB_DOEPCTL_EPDIS ) |
		   (0 << STM_USB_DOEPCTL_SNAK ) |
		   (0 << STM_USB_DOEPCTL_CNAK ) |
		   (0 << STM_USB_DOEPCTL_STALL ) |
		   (0 << STM_USB_DOEPCTL_SNPM ) |
		   (STM_USB_DOEPCTL_EPTYP_CONTROL << STM_USB_DOEPCTL_EPTYP ) |
		   (0 << STM_USB_DOEPCTL_NAKSTS ) |
		   (1 << STM_USB_DOEPCTL_USBAEP ) |
		   (STM_USB_DOEPCTL_MPSIZ0_64 << STM_USB_DOEPCTL_MPSIZ));

	stm_usb.doep[0].doepctl = doepctl;

	/* Clear interrupts */
	stm_usb.diep[0].diepint = 0xffffffff;
	stm_usb.doep[0].doepint = 0xffffffff;

	ao_usb_dev_ep_out_start(0);
}

void
ao_usb_dev_ep0_in(const void *data, uint16_t len)
{
	return ao_usb_dev_ep_in(0, data, len);
}

bool
ao_usb_dev_ep0_in_busy(void)
{
	return false;
}

uint16_t
ao_usb_dev_ep0_out(void *data, uint16_t len)
{
	return ao_usb_dev_ep_out(0, data, len);
}

/* Queue IN bytes to EPn */
void
ao_usb_dev_ep_in(uint8_t ep, const void *_data, uint16_t len)
{
	const uint8_t *data = _data;
	int	l = len;

	while (l > 0) {
		uint32_t d;
		memcpy(&d, data, 4);
		stm_usb.dfifo[ep].fifo = d;
		l -= 4;
		data += 4;
	}

	/* Set the IN data size */
	stm_usb.diep[ep].dieptsiz = ((1 << STM_USB_DIEPTSIZ_PKTCNT) |
				    (len << STM_USB_DIEPTSIZ_XFRSIZ));

	/* Enable the TX empty interrupt */
	stm_usb.diepempmsk |= (1 << ep);

	/* Enable the endpoint to queue the packet for transmission */
	stm_usb.diep[ep].diepctl |= (1 << STM_USB_DIEPCTL_EPENA);
}

bool
ao_usb_dev_ep_in_busy(uint8_t ep)
{
	(void) ep;
	return false;
}

/* Receive OUT bytes from EPn */
uint16_t
ao_usb_dev_ep_out(uint8_t ep, void *_data, uint16_t len)
{
	uint8_t		*data = _data;
	uint16_t	received;
	int		l = len;
	uint32_t	t;

	if (grxstsp_enum() != ep)
		return 0;

	received = grxstsp_bcnt();
	if (received > len)
		received = len;

	while (l >= 4) {
		uint32_t d;
		d = stm_usb.dfifo[0].fifo;
		memcpy(data, &d, 4);
		l -= 4;
		data += 4;
	}

	if (l != 0) {
		t = stm_usb.dfifo[0].fifo;
		memcpy(data, &t, l);
	}

	ao_usb_dev_ep_out_start(ep);
	return received;
}

void
ao_usb_dev_set_address(uint8_t address)
{
	uint32_t	dcfg;

	dcfg = stm_usb.dcfg;

	dcfg &= ~(STM_USB_DCFG_DAD_MASK << STM_USB_DCFG_DAD);
	dcfg |= address & STM_USB_DCFG_DAD_MASK;
	stm_usb.dcfg = dcfg;
}

static void
ao_usb_core_reset(void)
{
	/* Wait for AHB master IDLE state. */
	while ((stm_usb.grstctl & (1 << STM_USB_GRSTCTL_AHBIDL)) == 0)
		ao_arch_nop();


	/* Core soft reset */
	stm_usb.grstctl |= (1 << STM_USB_GRSTCTL_CSRST);

	/* Wait for reset to complete */

	while ((stm_usb.grstctl & (1 << STM_USB_GRSTCTL_CSRST)) != 0)
		ao_arch_nop();
}

static void
ao_usb_core_init(void)
{
	/* Enable embedded PHY */
	stm_usb.gusbcfg |= (1 << STM_USB_GUSBCFG_PHYSEL);

	/* Core reset */
	ao_usb_core_reset();

	/* Deactivate power down */
	stm_usb.gccfg = (1 << STM_USB_GCCFG_PWRDWN);
}

static void
ao_usb_delay(uint32_t ms)
{
	AO_TICK_TYPE	now = ao_time();
	AO_TICK_TYPE	then = now + AO_MS_TO_TICKS(ms);

	while ((int16_t) (then - ao_time()) > 0)
		ao_arch_nop();
}

static void
ao_usb_set_device_mode(void)
{
	uint32_t	gusbcfg;

	gusbcfg = stm_usb.gusbcfg;
	gusbcfg &= ~((1 << STM_USB_GUSBCFG_FHMOD) |
		     (1 << STM_USB_GUSBCFG_FDMOD));
	gusbcfg |= (1 << STM_USB_GUSBCFG_FDMOD);
	stm_usb.gusbcfg = gusbcfg;
	ao_usb_delay(50);
}

static void
ao_usb_device_init(void)
{
	/* deactivate vbus sensing */
	stm_usb.gccfg |= (1 << STM_USB_GCCFG_VBDEN);

	stm_usb.gccfg &= ~(1 << STM_USB_GCCFG_VBDEN);

	/* Force device mode */
	stm_usb.gotgctl |= ((1 << STM_USB_GOTGCTL_BVALOEN) |
			    (1 << STM_USB_GOTGCTL_BVALOVAL));

	/* Restart the phy clock */
	stm_usb.pcgcctl = 0;

	/* Device mode configuration */
	stm_usb.dcfg |= (STM_USB_DCFG_PFIVL_80 << STM_USB_DCFG_PFIVL);

	/* Set full speed phy */
	stm_usb.dcfg |= (STM_USB_DCFG_DSPD_FULL_SPEED << STM_USB_DCFG_DSPD);

	/* Flush the fifos */
	ao_usb_flush_tx_fifo(STM_USB_GRSTCTL_TXFNUM_ALL);
	ao_usb_flush_rx_fifo();

	/* Clear all pending device interrupts */
	stm_usb.diepmsk = 0;
	stm_usb.doepmsk = 0;
	stm_usb.daint = 0xffffffffUL;
	stm_usb.daintmsk = 0;

	/* Reset all endpoints */
	for (int i = 0; i < 6; i++) {

		/* Reset IN endpoint */
		if (stm_usb.diep[i].diepctl & (1 << STM_USB_DIEPCTL_EPENA))
			stm_usb.diep[i].diepctl = ((1 << STM_USB_DIEPCTL_EPDIS) |
						   (1 << STM_USB_DIEPCTL_SNAK));
		else
			stm_usb.diep[i].diepctl = 0;
		stm_usb.diep[i].dieptsiz = 0;
		stm_usb.diep[i].diepint = 0xffu;
	}

	for (int i = 0; i < 6; i++) {
		/* Reset OUT endpoint */
		if (stm_usb.doep[i].doepctl & (1 << STM_USB_DOEPCTL_EPENA))
			stm_usb.doep[i].doepctl = ((1 << STM_USB_DOEPCTL_EPDIS) |
						   (1 << STM_USB_DOEPCTL_SNAK));
		else
			stm_usb.doep[i].doepctl = 0;

		stm_usb.doep[i].doeptsiz = 0;
		stm_usb.doep[i].doepint = 0xffu;
	}

	stm_usb.diepmsk &= ~(1 << STM_USB_DIEPMSK_TXFURM);

	/* Disable all interrupts */
	stm_usb.gintmsk = 0;

	/* Clear pending interrupts */
	stm_usb.gintsts = 0xfffffffful;

	/* Enable core interrupts */
	stm_usb.gintmsk = ((1 << STM_USB_GINTMSK_WUIM ) |
			   (0 << STM_USB_GINTMSK_SRQIM ) |
			   (0 << STM_USB_GINTMSK_DISCINT ) |
			   (0 << STM_USB_GINTMSK_CIDSCHGM ) |
			   (0 << STM_USB_GINTMSK_LPMINTM ) |
			   (0 << STM_USB_GINTMSK_PTXFEM ) |
			   (0 << STM_USB_GINTMSK_HCIM) |
			   (0 << STM_USB_GINTMSK_PRTIM ) |
			   (0 << STM_USB_GINTMSK_RSTDETM ) |
			   (1 << STM_USB_GINTMSK_IISOOXFRM ) |
			   (1 << STM_USB_GINTMSK_IISOIXFRM ) |
			   (1 << STM_USB_GINTMSK_OEPINT) |
			   (1 << STM_USB_GINTMSK_IEPINT) |
			   (0 << STM_USB_GINTMSK_EOPFM ) |
			   (0 << STM_USB_GINTMSK_ISOODRPM ) |
			   (1 << STM_USB_GINTMSK_ENUMDNEM) |
			   (1 << STM_USB_GINTMSK_USBRST) |
			   (1 << STM_USB_GINTMSK_USBSUSPM ) |
			   (0 << STM_USB_GINTMSK_ESUSPM ) |
			   (0 << STM_USB_GINTMSK_GONAKEFFM ) |
			   (0 << STM_USB_GINTMSK_GINAKEFFM ) |
			   (0 << STM_USB_GINTMSK_NPTXFEM ) |
			   (0 << STM_USB_GINTMSK_RXFLVLM) |
			   (0 << STM_USB_GINTMSK_SOFM ) |
			   (0 << STM_USB_GINTMSK_OTGINT ) |
			   (0 << STM_USB_GINTMSK_MMISM));
}

static void
ao_usb_device_connect(void)
{
	/* Enable pull-up/pull-down */
	stm_usb.dctl &= ~(1 << STM_USB_DCTL_SDIS);
	ao_usb_delay(20);
}

static void
ao_usb_device_disconnect(void)
{
	/* Disable pull-up/pull-down */
	stm_usb.dctl |= (1 << STM_USB_DCTL_SDIS);
	ao_usb_delay(20);
}

static void
ao_usb_dev_start(void)
{
	ao_usb_device_connect();
	stm_usb.gahbcfg |= (1 << STM_USB_GAHBCFG_GINTMSK);
}

void
ao_usb_dev_enable(void)
{
	ao_arch_block_interrupts();

	/* Configure GPIOs */
	ao_enable_port(&stm_gpioa);
#if 0
	stm_afr_set(&stm_gpioa,  8, STM_AFR_AF10);	/* USB_FS_SOF */
	stm_afr_set(&stm_gpioa,  9, STM_AFR_AF10);	/* USB_FS_VBUS */
	stm_afr_set(&stm_gpioa, 10, STM_AFR_AF10);	/* USB_FS_ID */
#endif
	stm_afr_set(&stm_gpioa, 11, STM_AFR_AF10);
	stm_ospeedr_set(&stm_gpioa, 11, STM_OSPEEDR_HIGH);
	stm_pupdr_set(&stm_gpioa, 11, STM_PUPDR_NONE);

	stm_afr_set(&stm_gpioa, 12, STM_AFR_AF10);
	stm_ospeedr_set(&stm_gpioa, 12, STM_OSPEEDR_HIGH);
	stm_pupdr_set(&stm_gpioa, 12, STM_PUPDR_NONE);

	/* Power on USB */
	stm_rcc_ahb2_clk_enable(1 << STM_RCC_AHB2ENR_OTGFSEN);

	/* Route interrupts */
	stm_nvic_set_priority(STM_ISR_OTG_FS_POS, AO_STM_NVIC_LOW_PRIORITY);
	stm_nvic_set_enable(STM_ISR_OTG_FS_POS);

	ao_arch_release_interrupts();

	/* Core init */
	ao_usb_core_init();

	/* Set device mode */
	ao_usb_set_device_mode();

	/* Reset FIFO allocations */
	for (int i = 1; i < 16; i++)
		stm_usb.dieptxf[i-1] = 0x0;

	ao_usb_device_init();

	/* Disconnect */
	ao_usb_device_disconnect();

	/* Start */
	ao_usb_dev_start();
}

void
ao_usb_dev_disable(void)
{
	ao_usb_device_disconnect();

	stm_usb.gusbcfg = ((1 << STM_USB_GUSBCFG_FDMOD) |
			   (0 << STM_USB_GUSBCFG_FHMOD) |
			   (6 << STM_USB_GUSBCFG_TRDT) |
			   (0 << STM_USB_GUSBCFG_HNPCAP) |
			   (0 << STM_USB_GUSBCFG_SRPCAP) |
			   (1 << STM_USB_GUSBCFG_PHYSEL) |
			   (0 << STM_USB_GUSBCFG_TOCAL));

	stm_usb.gahbcfg = ((0 << STM_USB_GAHBCFG_PTXFELVL) |
			   (1 << STM_USB_GAHBCFG_TXFELVL) |
			   (0 << STM_USB_GAHBCFG_GINTMSK));

	stm_usb.dctl = ((0 << STM_USB_DCTL_POPRGDNE) |
			(1 << STM_USB_DCTL_SDIS));

	stm_rcc_ahb2_clk_disable(1 << STM_RCC_AHB2ENR_OTGFSEN);
}

void
stm_otg_fs_isr(void)
{
	uint32_t	gintsts = stm_usb.gintsts;
	uint8_t		ep0_receive = 0;
	uint32_t	out_interrupt = 0;
	uint32_t	in_interrupt = 0;

	/* Clear all received interrupts */
	stm_usb.gintsts = gintsts;

	if (gintsts & (1 << STM_USB_GINTSTS_USBRST)) {
		ep0_receive |= AO_USB_EP0_GOT_RESET;
	}

	if (gintsts & (1 << STM_USB_GINTSTS_ENUMDNE)) {
		ao_usb_enum_done();
	}

	if (gintsts & ((1 << STM_USB_GINTSTS_OEPINT) |
		       (1 << STM_USB_GINTSTS_IEPINT)))
	{
		uint32_t	daint = stm_usb.daint;
		uint32_t	oepint = (daint >> STM_USB_DAINT_OEPINT) & STM_USB_DAINT_OEPINT_MASK;
		uint32_t	iepint = (daint >> STM_USB_DAINT_IEPINT) & STM_USB_DAINT_IEPINT_MASK;

		for (int ep = 0; ep < 6; ep++) {
			if (gintsts & (1 << STM_USB_GINTSTS_OEPINT)) {
				if (oepint & (1 << ep)) {
					uint32_t	doepint = stm_usb.doep[ep].doepint;

					stm_usb.doep[ep].doepint = doepint;
					if (doepint & (1 << STM_USB_DOEPINT_XFRC)) {
						if (ep == 0)
							ep0_receive |= AO_USB_EP0_GOT_SETUP;
						else
							out_interrupt |= (1 << ep);
					}
					grxstsp = stm_usb.grxstsp;
				}
			}

			if (gintsts & (1 << STM_USB_GINTSTS_IEPINT)) {
				if (iepint & (1 << ep)) {
					uint32_t	diepint = stm_usb.diep[ep].diepint;

					stm_usb.diep[ep].diepint = diepint;
					if (diepint & (1 << STM_USB_DIEPINT_XFRC)) {
						if (ep == 0)
							ep0_receive |= AO_USB_EP0_GOT_TX_ACK;
						else
							in_interrupt |= (1 << ep);
					}
				}
			}
		}
	} else {
		grxstsp = 0;
	}

	if (ep0_receive)
		ao_usb_ep0_interrupt(ep0_receive);

	if (out_interrupt)
		ao_usb_out_interrupt(out_interrupt);

	if (in_interrupt)
		ao_usb_in_interrupt(in_interrupt);
}

/*

  running				before plugging in		at first packet
  gotgctl = 0x04cd0000,			0x04c10000,			0x04cd0000		*************

      CURMOD = 0
      OTGVER = 0
      BSVLD = 1				BSVLD = 0
      ASVLD = 1				ASVLD = 0
      DBCT = 0
      CIDSTS = 1

  gotgint = 0x00100000, 		0x00100000,			0x00100000

      IDCHNG = 1

  gahbcfg = 0x1, 			0x1,				0x00000001

      TXFELVL = 0	trigger half empty
      GINTMSK = 1	interrupts enabled

  gusbcfg = 0x40001840, 		0x40001440			0x40001840		*************

      FDMOD = 1	force device mode
      FHMOD = 0
      TRDT = 6				5				6
      HNPCAP = 0
      SRPCAP = 0
      PHYSEL = 1
      TOCAL = 0

  grstctl = 0x80000040, 		0x80000000			0x80000400		***********

      AHBIDL = 1
      TXFNUM = 1				TXFNUM = 0		TXFNUM = 0x20 (flush all)
      TXFFLSH = 0
      RXFFLSH = 0
      FCRST = 0
      PSRST = 0
      CSRST = 0

  gintsts = 0x0480b43a, 		0x04008022			0x04888438		***********

      WKUPINT = 0				0
      SRQINT = 0				0
      DISCINT = 0				0
      CIDSCHG = 0				0
      LPMINT = 0				0
      PTXFE = 1					PTXFE = 1		PTXFE = 1
      HCINT = 0
      HPRTINT = 0
      RSTDET = 1				RSTDET = 0		RSTDET = 1
      IPXFER = 0
      IISOIXFR = 0
      OEPINT = 0							OEPINT = 1
      IEPINT = 0
      EOPF = 1					EOPF = 1		EOPF = 1
      ISOODRP = 0
      ENUMDNE = 1
      USBRST = 1
      USBSUSP = 0
      ESUSP = 1								ESUSP = 1
      GONAKEFF = 0
      GINAKEFF = 0
      NPTXFE = 1			       	NPTXFE = 1		NPTXFE = 1
      RXFLVL = 1							RXFLVL = 1
      SOF = 1								SOF = 1
      OTGINT = 0
      MMIS = 1			       		MMIS = 1		MMIS = 0
      CMOD = 0

  gintmsk = 0xc03c3814, 		0xc03c3814,

      WUIM = 1
      SRQIM = 1
      DISCINT = 0
      CIDSCHGM = 0
      LPMINTM = 0
      PTXFEM = 0
      HCIM = 0
      PRTIM = 0
      RSTDETM = 0
      IISOOXFRM = 1
      IISOIXFRM = 1
      OEPINT = 1
      IEPINT = 1
      EOPFM = 0
      ISOODRPM = 0
      ENUMDNEM = 1
      USBRST = 1
      USBSUSPM = 1
      ESUSPM =0
      GONAKEFFM = 0
      GINAKEFFM = 0
      NPTXFEM = 0
      RXFLVLM = 1
      SOFM = 0
      OTGINT = 1
      MMISM = 0

  grxstsr = 0xac0080, 			0x0				0x14c0080	***************

      STSPHST = 0							STSPHST = 0
      FRMNUM = 5							FRMNUM = 10
      PKTSTS = 6	-- SETUP data packet				PKTSTS = 6 -- SETUP data packet
      DPID = 0								DPID = 0
      BCNT = 8								BCNT = 8
      EPNUM = 0								EPNUM = 0

  grxstsp = 0xac0080,			0x0				0x14c0080

      (same)

  grxfsiz = 0x80,			0x80				0x80

      RXFD = 128	512 bytes

  dieptxf0 = 0x00400080,		0x00400080			0x00400080

      TX0FD = 64	256 bytes
      TX0FSA = 0x80

  gccfg = 0x21fff0, 			0x21fff0			0x21fff0

      VBDEN = 1
      SDEN = 0
      PDEN = 0
      DCDEN = 0
      BCDEN = 0
      PWRDN = 1
      PS2DET = 0
      SDET = 0
      PDET = 0
      DCDET = 0

  cid = 0x2000,				0x2000				0x2000

      PRODUCT_ID = 0x2000

  glpmcfg = 0x0,			0x0				0x0

      ENBESL = 0
      LPMRCNTTST = 0
      SNDLPM = 0
      LPMRCNT = 0
      LPMCHIDX = 0
      L1RSMOK = 0
      SLPSTS = 0
      LPMRSP = 0
      L1DSEN = 0
      BESLTHRS = 0
      L1SSEN = 0
      REMWAKE = 0
      BESL = 0
      LPMACK = 0
      LPMEN = 0

  dieptxf = {0x8000c0, 0x0, 0x0, 0x0, 0x0}, 	{0x8000c0, 0x0, 0x0, 0x0, 0x0},	{0x8000c0, 0x0, 0x0, 0x0, 0x0}, 

      INEXPTXFD 0 = 0x80	512 bytes
      INEXPTXSA 0 = 0xc0

  dcfg = 0x82000b3,			0x8200003,			0x8200003

      ERRATIM = 0
      PFIVL = 0
      DAD = 0xb				DAD = 0x0			DAD = 0
      NZLSOHSK = 0
      DSPD = 3	Full speed USB 1.1

  dctl = 0x0, 				0x0				0x0

      DSBESLRJCT = 0
      POPRGDNE = 0
      CGONAK = 0
      SGONAK = 0
      CGINAK = 0
      SGINAK = 0
      TCTL = 0
      GONSTS = 0
      GINSTS = 0
      SDIS = 0
      RWUSIG = 0

  dsts = 0x0043ff06, 			0x00000006			0x00400c06

      DEVLNSTS = 1	(D+ low, D- high)
      FNSOF = 0x3ff							FNSOF = 0xc
      EERR = 0
      ENUMSPD = 3	Full speed					ENUMSPD = 3
      SUSPSTS = 0							SUSPSTS = 0

  diepmsk = 0xb, 			0x0				0xb

      NAKM = 0
      TXFURM = 0
      INEPNEM = 0
      INEPNMM = 0
      ITTXFEMSK = 0
      TOM = 1
      EPDM = 1
      XFRCM = 1

  doepmsk = 0x2b, 			0x0				0x2b

      NYETMSK = 0
      NAKMSK = 0
      BERRM =0
      OUTPKTERRM = 0
      STSPHSRXM = 1
      OTEPDM = 0
      STUPM = 1
      EPDM = 1
      XFRCM = 1

  daint = 0x0, 				0x0			0x10000

  daintmsk = 0x30003,			0x0			0x10001

      OEPM = 0x3 	endpoints 0 and 1			OEPM = 0x1	endpoint 0
      IEPM = 0x3	endpoints 0 and 1			IEPM = 0x1	endpoint 0

  dvbusdis = 0x17d7, 			0x17d7			0x17d7

      VBUSDT = 0x17d7 	reset value

  dvbuspulse = 0x5b8, 			0x5b8			0x5b8

      DVBUSP = 0x5b8	reset value

  diepempmsk = 0x0, 			0x0			0x0

      INEPTXFEM = 0		no endpoints

  diep = {{
      diepctl = 0x28000,

	  EPENA = 0
	  EPDIS = 0
	  SNAK = 0
	  CNAK = 0
	  TXFNUM = 0
	  STALL = 0
	  EPTYP = 0
	  NAKSTS = 1
	  USBAEP = 1
	  MPSIZ = 0		64 bytes

      diepint = 0x20c0,

	  NAK = 1
	  PKTDRPSTS = 0
	  TXFIFOUDRN = 0
	  TXFE = 1
	  INEPNE = 1
	  ITTXFE = 0
	  TOC = 0
	  EPDISD = 0
	  XFRC = 0

      dieptsiz = 0x0,

	  PKTCNT = 0
	  XFRSIZ = 0

      dtxfsts = 0x40,

	  INEPTFSAV = 0x40	256 bytes available

    }, {
      diepctl = 0x00490040,

	  EPENA = 0
	  EPDIS = 0
	  SODDFRM = 0
	  SD0PID  = 0
	  SNAK = 0
	  CNAK = 0
	  TXFNUM = 1
	  STALL = 0
	  EPTYP = 2		bulk
	  NAKSTS = 0
	  EONUM = 1
	  USBAEP = 0
	  MPSIZ = 64	256 bytes

      diepint = 0x2090,

	  NAK = 1
	  PKTDRPSTS = 0
	  TXFIFOUDRN = 0
	  TXFE = 1
	  INEPNE = 0
	  INPENM = 0
	  ITTXFE = 1
	  TOC = 0
	  EPDISD = 0
	  XFRC = 0

      dieptsiz = 0x0,

	  MCNT = 0
	  PKTCNT = 0
	  XFRSIZ = 0

      dtxfsts = 0x80,

	  INEPTFSAV = 0x80		512 bytes available

    }, {
      diepctl = 0x0,
      pad_04 = 0x0,
      diepint = 0x80,
      pad_0c = 0x0,
      dieptsiz = 0x0,
      pad_14 = 0x43425355,
      dtxfsts = 0x40,
      pad_1c = 0x400000
    }, {
      diepctl = 0x0,
      pad_04 = 0x0,
      diepint = 0x80,
      pad_0c = 0x0,
      dieptsiz = 0x0,
      pad_14 = 0x43425355,
      dtxfsts = 0x40,
      pad_1c = 0x400000
    }, {
      diepctl = 0x0,
      pad_04 = 0x0,
      diepint = 0x80,
      pad_0c = 0x0,
      dieptsiz = 0x0,
      pad_14 = 0x43425355,
      dtxfsts = 0x40,
      pad_1c = 0x400000
    }, {
      diepctl = 0x0,
      pad_04 = 0x0,
      diepint = 0x80,
      pad_0c = 0x0,
      dieptsiz = 0x0,
      pad_14 = 0x43425355,
      dtxfsts = 0x40,
      pad_1c = 0x400000
    }},

  doep = {{
      doepctl = 0x80028000, 		0x00008000,			0x28000

	  EPENA = 1			    EPENA = 0			EPENA = 0
	  EPDIS = 0
	  SNAK =0
	  CNAK = 0
	  STALL = 0
	  SNPM = 0
	  EPTYP = 0
	  NAKSTS = 1			    NAKSTS = 0			NAKSTS = 1
	  USPAEP = 1							USPAEP = 1
	  MPSIZ = 0		64 bytes				MPSIZ = 0

      doepint = 0x8010, 		0x0				0x8008

	  NYET = 0
	  NAK = 0
	  BERR = 0
	  OUTPKTERR = 0
	  STSPHSRX = 0
	  OTEPDIS = 1
	  STUP = 0							STUP = 1
	  EPDISD = 0
	  XFRC = 0

      doeptsiz = 0x38, 			0x0				0x20080008

	  STPCNT = 0	1 packet					STPCNT = 1
	  PKTCNT = 0							PKTCNT = 1
	  XFRSIZ = 0x38	56 bytes (64 - 8)				XFRSIZ = 8

    }, {
      doepctl = 0x800b0040,

	  EPENA = 1
	  EPDIS = 0
	  SD1PID = 0
	  SD0PID = 0
	  SNAK = 0
	  CNAK = 0
	  STALL = 0
	  SNPM =0
	  EPTYP = 2		Bulk
	  NAKSTS = 1
	  EONUM = 1
	  USBAEP = 0
	  MPSIZ = 0x40	64 bytes

      doepint = 0x0,
      doeptsiz = 0x21,

	  RXDPID = 0
	  PKTCNT = 0
	  XFRSIZ = 0x21    33 bytes ?

    }, {
      doepctl = 0x0,
      pad_04 = 0x0,
      doepint = 0x0,
      pad_0c = 0x0,
      doeptsiz = 0x0,
      pad_14 = 0x43425355,
      pad_18 = 0x40,
      pad_1c = 0x400000
    }, {
      doepctl = 0x0,
      pad_04 = 0x0,
      doepint = 0x0,
      pad_0c = 0x0,
      doeptsiz = 0x0,
      pad_14 = 0x43425355,
      pad_18 = 0x40,
      pad_1c = 0x400000
    }, {
      doepctl = 0x0,
      pad_04 = 0x0,
      doepint = 0x0,
      pad_0c = 0x0,
      doeptsiz = 0x0,
      pad_14 = 0x43425355,
      pad_18 = 0x40,
      pad_1c = 0x400000
    }, {
      doepctl = 0x0,
      pad_04 = 0x0,
      doepint = 0x0,
      pad_0c = 0x0,
      doeptsiz = 0x0,
      pad_14 = 0x43425355,
      pad_18 = 0x40,
      pad_1c = 0x400000
    }},

  pcgcctl = 0x0,		0x0,			0x0

      SUSP = 0
      PHYSLEEP = 0
      ENL1GTG = 0
      PHYSUSP = 0
      GATEHCLK = 0
      STPPCLK = 0

  dfifo = {{
	fifo =						0x1000680,
								

      Clock configuration:

$5 = {
  cr = 0x0f077d83, 

	PLLI2SRDY = 1
	PLLI2SON = 1
	PLLRDY = 1
	PLLON = 1
	CSSON = 0
	HSEBYP = 1
	HSERDY = 1
	HSEON = 1
	HSICAL = 0x7d
	HSITRIM = 0x10
	HSIRDY = 1
	HSION = 1

  pllcfgr = 0x27403208,

	PLLR = 2
	PLLQ = 7
	PLLSRC = 1	HSE
	PLLP = 0	2		
	PLLN = 0xc8	200
	PLLM = 8

		clk_pllin = 8000000 / 8 = 1000000
		vco = 1000000 * 200 = 200000000
		clk_pll1p = 200000000 / 2 = 100000000 (100MHz)
		clk_pll1q = 200000000 / 7 = ???
		clk_pll1r = 200000000 / 2 = 100000000 (100MHz)

  cfgr = 0x0000100a, 
  cir = 0x00000000, 
  ahb1rstr = 0x0, 
  ahb2rstr = 0x0, 
  ahb3rstr = 0x0, 
  pad_1c = 0x0, 
  apb1rstr = 0x0, 
  apb2rstr = 0x0, 
  pad_28 = 0x0, 
  pad_2c = 0x0, 
  ahb1enr = 0x40107f, 
  ahb2enr = 0x80, 
  ahbdnr = 0x3, 
  pad_3c = 0x0, 
  apb1enr = 0x11000410, 
  apb2enr = 0xc800, 
  pad_48 = 0x0, 
  pad_4c = 0x0, 
  ahb1lpenr = 0x6390ff, 
  ahb2lpenr = 0xd0, 
  ahb3lpenr = 0x3, 
  pad_5c = 0x0, 
  apb1lpenr = 0xfffecfff, 
  apb2lpenr = 0x357f9f3, 
  pad_68 = 0x0, 
  pad_6c = 0x0, 
  bdcr = 0x8200, 
  csr = 0x1e000003, 
  pad_78 = 0x0, 
  pad_7c = 0x0, 
  sscgr = 0x0, 
  plli2scfgr = 0x44003008, 

	PLLI2SR = 4
	PLLI2SQ = 4
	PLLI2SSRC = 0	HSE (due to PLLSRC)
	PLLI2SN = 0xc0	192
	PLLI2SM = 8

		clk_plli2sin = 8000000 / 8 = 1000000
		vcoi2s = 1000000 * 192 = 192000000
		ck_pl2q = 192000000 / 4 = 48000000
		ck_pl2r = 192000000 / 4 = 48000000

  pad_88 = 0x0, 
  dckcfgr = 0x0, 


  ckgatenr = 0x0,

	All clock gates enabled

  dckcfgr2 = 0x08000000

	LPTIMER1SEL = 0		APB
	CKSDIOSEL = 0		CK_48MHz
	CK48MSEL = 1		PLLI2S_Q
	I2CFMP1SEL = 0		APB
}

*/

/*
 *
 * altos clock configuration
 * (gdb) print/x stm_rcc
 * $8 = {
 *	  altos			demo firmware
 * cr = 0x0307 7d80,		0x0f077d83, 
 *
 *   PLLI2SRDY 0		1
 *   PLLI2SON  0		1
 *   PLLRDY    1		1
 *   PLLON     1		1
 *   CSSON     0		0
 *   HSEBYP    1		1
 *   HSERDY    1		1
 *   HSEON     1		1
 *   HSICAL    0x7d		0x7d
 *   HSITRIM   0x10		0x10
 *   HSIRDY    0		1
 *   HSION     0		1
 *
 * pllcfgr = 0x24403008,	0x27403208,
 *   PLLR	2		2
 *   PLLQ	4 		7
 *   PLLSRC	1		1
 *   PLLP	0 (/2)		0 (/2)
 *   PLLN	192		200
 *   PLLM	8		8
 *
 * cfgr = 0x3640100a,		0x0000100a,
 *   upper bits are just MCO
 *
 *   cir = 0x0,			0x0
 *   ahb1rstr = 0x0,		0x0
 *   ahb2rstr = 0x0,		0x0
 *   ahb3rstr = 0x0,
 *   pad_1c = 0x0,
 *   apb1rstr = 0x0,
 *   apb2rstr = 0x0,
 *   pad_28 = 0x0,
 *   pad_2c = 0x0,
 *   _ahb1enr = 0x55,		0x80
 *   _ahb2enr = 0x80,		0xc800
 *   ahbdnr = 0x0,
 *   pad_3c = 0x0,
 *   apb1enr = 0x10000400,
 *   apb2enr = 0x8020,
 *   pad_48 = 0x0,
 *   pad_4c = 0x0,
 *   ahb1lpenr = 0x6390ff,	0x6390ff
 *   ahb2lpenr = 0xd0,		0xd0
 *   ahb3lpenr = 0x3,		0x3
 *   pad_5c = 0x0,
 *   apb1lpenr = 0xfffecfff,	0xfffecfff,
 *   apb2lpenr = 0x357f9f3,	0x357f9f3,
 *   pad_68 = 0x0,
 *   pad_6c = 0x0,
 *   bdcr = 0x0,		0x8200,
 *   csr = 0x0,			0x1e000003,
 *   pad_78 = 0x0,
 *   pad_7c = 0x0,
 *   sscgr = 0x0,
 *   plli2scfgr = 0x24003010,	0x44003008,
 *   pad_88 = 0x0,
 *   dckcfgr = 0x0,
 *   ckgatenr = 0x0,
 *   dckcfgr2 = 0x8000000	0x08000000
 * }
 *
 *
 */

/*
 *
 * main
 *  	HAL_Init
 *  	SystemClock_Config
 *   
 * 	USBD_Init
 *  		USBD_LL_Init
 *    			HAL_PCD_Init
 *     				HAL_PCD_MspInit
 *					__HAL_RCC_GPIOA_CLK_ENABLE
 *					HAL_GPIO_Init
 *					__HAL_RCC_USB_OTG_FS_CLK_ENABLE
 *					HAL_NVIC_SetPriority
 *					HAL_NVIC_EnableIRQ
 *     				USB_CoreInit
 *					Select FS Embedded PHY
 *					USB_CoreReset
 *					Deactivate the power down
 *     				USB_SetCurrentMode
 *     				USB_DevInit
 *					VBUS sensing stuff
 *					Restart PHY clock
 *					USB_SetDevSpeed
 *					USB_FlushTxFifo
 *					USB_FlushRxFifo
 *					Clear pending interrupts
 *					Disable all endpoints
 *					Disable all interrupts
 *					Clear pending interrupts
 *					enable interrupts
 *     				USB_DevDisconnect
 *					Turn on SDIS bit
 *					delay 3ms
 *    			HAL_PCDEx_SeRxFifo
 *    			HAL_PCDEx_SetTxFifo
 *  	USBD_RegisterClass
 *  	USBD_MSC_RegisterStorage
 *  	USBD_Start
 *		USBD_LL_Start
 *			HAL_PCD_Start
 *				__HAL_LOCK
 *				USB_DevConnect
 *					Turn off SDIS bit
 *					delay 3ms
 *				__HAL_PCD_ENABLE
 *					USB_EnableGlobalInt
 *  						USBx->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
 *				__HAL_UNLOCK
 *
 */
