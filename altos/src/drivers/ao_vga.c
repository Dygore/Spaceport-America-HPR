/*
 * Copyright © 2016 Keith Packard <keithp@keithp.com>
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

#include "ao.h"
#include "ao_vga.h"

/* VGA output from the SPI port
 *
 * Connections:
 *
 *			STM	VGA
 *	GND			4,6,7,8,9,10
 *	HSYNC		PA5	13
 *	VSYNC		PB5	14
 *	RGB		PB4	1,2,3
 *
 *	pixel clock	PA8 -> PB3
 *	pixel enable	PA1 -> PA15
 */

/* GRF formula for 640x480 yields a pixel clock very close to 24MHz. Pad by
 * three scanlines to hit exactly that value
 */

#define HACTIVE 	(640)
#define HSYNC_START 	(656)
#define HSYNC_END	(720)
#define HTOTAL		(800)

#define	VACTIVE		480
#define VSYNC_START	481
#define VSYNC_END	484
#define VTOTAL		500

/*
 * The horizontal counter is set so that the end of hsync is reached
 * at the maximum counter value. That means that the hblank interval
 * is offset by HSYNC_END.
 */

#define HSYNC		(HSYNC_END - HSYNC_START)
#define HBLANK_END	(HTOTAL - HSYNC_END)
#define HBLANK_START	(HBLANK_END + HACTIVE)

/*
 * The vertical counter is set so that the end of vsync is reached at
 * the maximum counter value.  That means that the vblank interval is
 * offset by VSYNC_END. We send a blank line at the start of the
 * frame, so each of these is off by one
 */
#define VSYNC		(VSYNC_END - VSYNC_START)
#define VBLANK_END	(VTOTAL - VSYNC_END)
#define VBLANK_START	(VBLANK_END + VACTIVE)

#define WIDTH_BYTES	(AO_VGA_WIDTH >> 3)
#define SCANOUT		((WIDTH_BYTES+2) >> 1)

uint32_t	ao_vga_fb[AO_VGA_STRIDE * AO_VGA_HEIGHT];

const struct ao_bitmap ao_vga_bitmap = {
	.base = ao_vga_fb,
	.stride = AO_VGA_STRIDE,
	.width = AO_VGA_WIDTH,
	.height = AO_VGA_HEIGHT
};

static uint32_t	*scanline;

#define DMA_INDEX	STM_DMA_INDEX(STM_DMA_CHANNEL_SPI1_TX)

#define DMA_CCR(en)	((0 << STM_DMA_CCR_MEM2MEM) |			\
			 (STM_DMA_CCR_PL_VERY_HIGH << STM_DMA_CCR_PL) | \
			 (STM_DMA_CCR_MSIZE_16 << STM_DMA_CCR_MSIZE) |	\
			 (STM_DMA_CCR_PSIZE_16 << STM_DMA_CCR_PSIZE) |	\
			 (1 << STM_DMA_CCR_MINC) |			\
			 (0 << STM_DMA_CCR_PINC) |			\
			 (0 << STM_DMA_CCR_CIRC) |			\
			 (STM_DMA_CCR_DIR_MEM_TO_PER << STM_DMA_CCR_DIR) | \
			 (0 << STM_DMA_CCR_TCIE) |			\
			 (en << STM_DMA_CCR_EN))


void stm_tim2_isr(void)
{
	int16_t	line = stm_tim3.cnt;

	if (VBLANK_END <= line && line < VBLANK_START) {
		/* Disable */
		stm_dma.channel[DMA_INDEX].ccr = DMA_CCR(0);
		/* Reset DMA engine for the next scanline */
		stm_dma.channel[DMA_INDEX].cmar = scanline;
		stm_dma.channel[DMA_INDEX].cndtr = SCANOUT;

		/* reset SPI */
		(void) stm_spi1.dr;
		(void) stm_spi1.sr;

		/* Enable */
		stm_dma.channel[DMA_INDEX].ccr = DMA_CCR(1);
		if (((line - VBLANK_END) & 1))
			scanline += AO_VGA_STRIDE;
	} else {
		scanline = ao_vga_fb;
	}
	stm_tim2.sr = 0;
}


void
ao_vga_init(void)
{
	uint32_t	cfgr;

	/* Initialize spi1 using MISO PB4 for output */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);

	stm_ospeedr_set(&stm_gpiob, 4, STM_OSPEEDR_40MHz);
	stm_afr_set(&stm_gpiob, 4, STM_AFR_AF5);
	stm_afr_set(&stm_gpiob, 3, STM_AFR_AF5);
	stm_afr_set(&stm_gpioa, 15, STM_AFR_AF5);

	/* turn on SPI */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SPI1EN);

	stm_spi1.cr1 = ((1 << STM_SPI_CR1_BIDIMODE) |		/* Two wire mode */
			(1 << STM_SPI_CR1_BIDIOE) |
			(0 << STM_SPI_CR1_CRCEN) |		/* CRC disabled */
			(0 << STM_SPI_CR1_CRCNEXT) |
			(1 << STM_SPI_CR1_DFF) |
			(0 << STM_SPI_CR1_RXONLY) |		/* transmit, not receive */
			(0 << STM_SPI_CR1_SSM) |        	/* Software SS handling */
			(1 << STM_SPI_CR1_SSI) |		/*  ... */
			(1 << STM_SPI_CR1_LSBFIRST) |		/* Little endian */
			(1 << STM_SPI_CR1_SPE) |		/* Enable SPI unit */
			(0 << STM_SPI_CR1_BR) |			/* baud rate to pclk/2 */
			(0 << STM_SPI_CR1_MSTR) |		/* slave */
			(0 << STM_SPI_CR1_CPOL) |		/* Format 0 */
			(0 << STM_SPI_CR1_CPHA));
	stm_spi1.cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(1 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));

	(void) stm_spi1.dr;
	(void) stm_spi1.sr;

	/* Grab the DMA channel for SPI1 MOSI */
	stm_dma.channel[DMA_INDEX].cpar = &stm_spi1.dr;
	stm_dma.channel[DMA_INDEX].cmar = ao_vga_fb;

	/*
	 * Hsync Configuration
	 */
	/* Turn on timer 2 */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM2EN);

	/* tim2 runs at full speed */
	stm_tim2.psc = 0;

	/* Disable channels while modifying */
	stm_tim2.ccer = 0;

	/* Channel 1 hsync PWM values */
	stm_tim2.ccr1 = HSYNC;

	/* Channel 2 trigger scanout */
	/* wait for the time to start scanout */
	stm_tim2.ccr2 = HBLANK_END;

	stm_tim2.ccr3 = 32;

	/* Configure channel 1 to output on the pin and
	 * channel 2 to to set the trigger for the vsync timer
	 */
	stm_tim2.ccmr1 = ((0 << STM_TIM234_CCMR1_OC2CE) |
			  (STM_TIM234_CCMR1_OC2M_PWM_MODE_1 << STM_TIM234_CCMR1_OC2M)  |
			  (1 << STM_TIM234_CCMR1_OC2PE) |
			  (0 << STM_TIM234_CCMR1_OC2FE) |
			  (STM_TIM234_CCMR1_CC2S_OUTPUT << STM_TIM234_CCMR1_CC2S) |

			  (0 << STM_TIM234_CCMR1_OC1CE) |
			  (STM_TIM234_CCMR1_OC1M_PWM_MODE_1 << STM_TIM234_CCMR1_OC1M)  |
			  (1 << STM_TIM234_CCMR1_OC1PE) |
			  (0 << STM_TIM234_CCMR1_OC1FE) |
			  (STM_TIM234_CCMR1_CC1S_OUTPUT << STM_TIM234_CCMR1_CC1S));

	stm_tim2.ccmr2 = ((0 << STM_TIM234_CCMR2_OC4CE) |
			  (0 << STM_TIM234_CCMR2_OC4M)  |
			  (0 << STM_TIM234_CCMR2_OC4PE) |
			  (0 << STM_TIM234_CCMR2_OC4FE) |
			  (0 << STM_TIM234_CCMR2_CC4S) |

			  (0 << STM_TIM234_CCMR2_OC3CE) |
			  (STM_TIM234_CCMR2_OC3M_PWM_MODE_1 << STM_TIM234_CCMR2_OC3M)  |
			  (1 << STM_TIM234_CCMR2_OC3PE) |
			  (0 << STM_TIM234_CCMR2_OC3FE) |
			  (0 << STM_TIM234_CCMR2_CC3S));

	/* One scanline */
	stm_tim2.arr = HTOTAL;

	stm_tim2.cnt = 0;

	/* Update the register contents */
	stm_tim2.egr |= (1 << STM_TIM234_EGR_UG);

	/* Enable the timer */

	/* Enable the output */
	stm_tim2.ccer = ((0 << STM_TIM234_CCER_CC2NP) |
			 (STM_TIM234_CCER_CC2P_ACTIVE_HIGH << STM_TIM234_CCER_CC2P) |
			 (1 << STM_TIM234_CCER_CC2E) |
			 (0 << STM_TIM234_CCER_CC1NP) |
			 (STM_TIM234_CCER_CC1P_ACTIVE_LOW << STM_TIM234_CCER_CC1P) |
			 (1 << STM_TIM234_CCER_CC1E));

	stm_tim2.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
			(STM_TIM234_CR2_MMS_UPDATE << STM_TIM234_CR2_MMS) |
			(0 << STM_TIM234_CR2_CCDS));

	/* hsync is not a slave timer */
	stm_tim2.smcr = 0;

	/* Send an interrupt on channel 3 */
	stm_tim2.dier = ((1 << STM_TIM234_DIER_CC3IE));

	stm_tim2.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
			(1 << STM_TIM234_CR1_ARPE) |
			(STM_TIM234_CR1_CMS_EDGE << STM_TIM234_CR1_CMS) |
			(STM_TIM234_CR1_DIR_UP << STM_TIM234_CR1_DIR) |
			(0 << STM_TIM234_CR1_OPM) |
			(1 << STM_TIM234_CR1_URS) |
			(0 << STM_TIM234_CR1_UDIS) |
			(0 << STM_TIM234_CR1_CEN));

	/* Hsync is on PA5 which is Timer 2 CH1 output */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);
	stm_ospeedr_set(&stm_gpioa, 5, STM_OSPEEDR_40MHz);
	stm_afr_set(&stm_gpioa, 5, STM_AFR_AF1);

	/* pixel transmit enable is on PA1 */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOAEN);
	stm_ospeedr_set(&stm_gpioa, 1, STM_OSPEEDR_40MHz);
	stm_afr_set(&stm_gpioa, 1, STM_AFR_AF1);

	/*
	 * Vsync configuration
	 */

	/* Turn on timer 3, slaved to timer 1 using ITR1 (table 61) */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_TIM3EN);

	/* No prescale */
	stm_tim3.psc = 0;

	/* Channel 1 or 2 vsync PWM values */
	stm_tim3.ccr1 = VSYNC;
	stm_tim3.ccr2 = VSYNC;

	stm_tim3.ccmr1 = ((0 << STM_TIM234_CCMR1_OC2CE) |
			  (STM_TIM234_CCMR1_OC2M_PWM_MODE_1 << STM_TIM234_CCMR1_OC2M)  |
			  (1 << STM_TIM234_CCMR1_OC2PE) |
			  (0 << STM_TIM234_CCMR1_OC2FE) |
			  (STM_TIM234_CCMR1_CC2S_OUTPUT << STM_TIM234_CCMR1_CC2S) |

			  (0 << STM_TIM234_CCMR1_OC1CE) |
			  (STM_TIM234_CCMR1_OC1M_PWM_MODE_1 << STM_TIM234_CCMR1_OC1M)  |
			  (1 << STM_TIM234_CCMR1_OC1PE) |
			  (0 << STM_TIM234_CCMR1_OC1FE) |
			  (STM_TIM234_CCMR1_CC1S_OUTPUT << STM_TIM234_CCMR1_CC1S));

	stm_tim3.arr = VTOTAL;
	stm_tim3.cnt = 0;

	/* Update the register contents */
	stm_tim3.egr |= (1 << STM_TIM234_EGR_UG);

	/* Enable the timer */

	/* Enable the output */
	stm_tim3.ccer = ((0 << STM_TIM234_CCER_CC1NP) |
			 (STM_TIM234_CCER_CC2P_ACTIVE_LOW << STM_TIM234_CCER_CC2P) |
			 (1 << STM_TIM234_CCER_CC2E) |
			 (STM_TIM234_CCER_CC1P_ACTIVE_LOW << STM_TIM234_CCER_CC1P) |
			 (1 << STM_TIM234_CCER_CC1E));

	stm_tim3.cr2 = ((0 << STM_TIM234_CR2_TI1S) |
			(STM_TIM234_CR2_MMS_UPDATE << STM_TIM234_CR2_MMS) |
			(0 << STM_TIM234_CR2_CCDS));

	stm_tim3.smcr = 0;
	stm_tim3.smcr = ((0 << STM_TIM234_SMCR_ETP) |
			 (0 << STM_TIM234_SMCR_ECE) |
			 (STM_TIM234_SMCR_ETPS_OFF << STM_TIM234_SMCR_ETPS) |
			 (STM_TIM234_SMCR_ETF_NONE << STM_TIM234_SMCR_ETF) |
			 (0 << STM_TIM234_SMCR_MSM) |
			 (STM_TIM234_SMCR_TS_ITR1 << STM_TIM234_SMCR_TS) |
			 (0 << STM_TIM234_SMCR_OCCS) |
			 (STM_TIM234_SMCR_SMS_EXTERNAL_CLOCK << STM_TIM234_SMCR_SMS));

	stm_tim3.dier = 0;

	stm_tim3.cr1 = ((STM_TIM234_CR1_CKD_1 << STM_TIM234_CR1_CKD) |
			(1 << STM_TIM234_CR1_ARPE) |
			(STM_TIM234_CR1_CMS_EDGE << STM_TIM234_CR1_CMS) |
			(STM_TIM234_CR1_DIR_UP << STM_TIM234_CR1_DIR) |
			(0 << STM_TIM234_CR1_OPM) |
			(1 << STM_TIM234_CR1_URS) |
			(0 << STM_TIM234_CR1_UDIS) |
			(1 << STM_TIM234_CR1_CEN));

	/* Vsync is on PB5 which is is Timer 3 CH2 output */
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIOBEN);
	stm_ospeedr_set(&stm_gpiob, 5, STM_OSPEEDR_40MHz);
	stm_afr_set(&stm_gpiob, 5, STM_AFR_AF2);

	/* Use MCO for the pixel clock, that appears on PA8 */
	cfgr = stm_rcc.cfgr & ~((STM_RCC_CFGR_MCOPRE_MASK << STM_RCC_CFGR_MCOPRE) |
				(STM_RCC_CFGR_MCOSEL_MASK << STM_RCC_CFGR_MCOSEL));

	cfgr |= ((STM_RCC_CFGR_MCOPRE_DIV_2 << STM_RCC_CFGR_MCOPRE) |
		 (STM_RCC_CFGR_MCOSEL_SYSCLK << STM_RCC_CFGR_MCOSEL));

	stm_rcc.cfgr = cfgr;

	stm_ospeedr_set(&stm_gpioa, 8, STM_OSPEEDR_40MHz);
	stm_afr_set(&stm_gpioa, 8, STM_AFR_AF0);

	/* Enable the scanline interrupt */
	stm_nvic_set_priority(STM_ISR_TIM2_POS, AO_STM_NVIC_NONMASK_PRIORITY);
	stm_nvic_set_enable(STM_ISR_TIM2_POS);
}

uint8_t	enabled;

void
ao_vga_enable(int enable)
{
	if (enable) {
		if (!enabled) {
			++ao_task_minimize_latency;
			enabled = 1;
		}
		stm_tim2.cr1 |= (1 << STM_TIM234_CR1_CEN);
	} else {
		if (enabled) {
			--ao_task_minimize_latency;
			enabled = 0;
		}
		stm_tim2.cr1 &= ~(1 << STM_TIM234_CR1_CEN);
	}
}
