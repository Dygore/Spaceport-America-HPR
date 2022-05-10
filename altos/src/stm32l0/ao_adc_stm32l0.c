/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_adc_stm32l0.h>

void
ao_adc_init(void)
{
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_ADCEN);

	/* Configure */
	stm_adc.cfgr1 = ((0 << STM_ADC_CFGR1_AWDCH) |				  /* analog watchdog channel 0 */
			 (0 << STM_ADC_CFGR1_AWDEN) |				  /* Disable analog watchdog */
			 (0 << STM_ADC_CFGR1_AWDSGL) |				  /* analog watchdog on all channels */
			 (0 << STM_ADC_CFGR1_DISCEN) |				  /* Not discontinuous mode. All channels converted with one trigger */
			 (0 << STM_ADC_CFGR1_AUTOOFF) |				  /* Leave ADC running */
			 (1 << STM_ADC_CFGR1_WAIT) |				  /* Wait for data to be read before next conversion */
			 (0 << STM_ADC_CFGR1_CONT) |				  /* only one set of conversions per trigger */
			 (1 << STM_ADC_CFGR1_OVRMOD) |				  /* overwrite on overrun */
			 (STM_ADC_CFGR1_EXTEN_DISABLE << STM_ADC_CFGR1_EXTEN) |	  /* SW trigger */
			 (0 << STM_ADC_CFGR1_ALIGN) |				  /* Align to LSB */
			 (STM_ADC_CFGR1_RES_12 << STM_ADC_CFGR1_RES) |		  /* 12 bit resolution */
			 (STM_ADC_CFGR1_SCANDIR_UP << STM_ADC_CFGR1_SCANDIR) |	  /* scan 0 .. n */
			 (STM_ADC_CFGR1_DMACFG_ONESHOT << STM_ADC_CFGR1_DMACFG) | /* one set of conversions then stop */
			 (0 << STM_ADC_CFGR1_DMAEN));				  /* disable DMA */

	/* Set the clock */
	stm_adc.cfgr2 = STM_ADC_CFGR2_CKMODE_PCLK_2 << STM_ADC_CFGR2_CKMODE;

	/* Shortest sample time */
	stm_adc.smpr = STM_ADC_SMPR_SMP_71_5 << STM_ADC_SMPR_SMP;

#define AO_ADC_LFMEN	(AO_SYSCLK < 3500000)

	stm_adc.ccr = ((AO_ADC_LFMEN << STM_ADC_CCR_LFMEN) |
		       (0 << STM_ADC_CCR_VLCDEN) |
		       (0 << STM_ADC_CCR_TSEN) |
		       (0 << STM_ADC_CCR_VREFEN));

	/* Calibrate. This also enables the ADC vreg */
	stm_adc.cr |= (1UL << STM_ADC_CR_ADCAL);
	while ((stm_adc.cr & (1UL << STM_ADC_CR_ADCAL)) != 0)
		;

	/* Enable */
	stm_adc.isr = (1 << STM_ADC_ISR_ADRDY);	/* Clear ADRDY bit */
	stm_adc.cr |= (1 << STM_ADC_CR_ADEN);
	while ((stm_adc.isr & (1 << STM_ADC_ISR_ADRDY)) == 0)
		;
}

static void
ao_adc_shutdown(void)
{
	/* Disable ADC */
	stm_adc.cr |= (1 << STM_ADC_CR_ADDIS);
	while ((stm_adc.cr & (1 << STM_ADC_CR_ADEN)) != 0)
		;

	/* Clear ADRDY bit */
	stm_adc.isr = (1 << STM_ADC_ISR_ADRDY);

	/* Disable ADC vreg */
	stm_adc.cr &= ~(1UL << STM_ADC_CR_ADVREGEN);

	/* Disable ADC clocks */
	stm_rcc.apb2enr &= ~(1UL << STM_RCC_APB2ENR_ADCEN);
}

uint16_t
ao_adc_read_vref(void)
{
	uint16_t value;

	ao_adc_init();

	/* Turn on voltage reference */
	stm_adc.ccr |= (1 << STM_ADC_CCR_VREFEN);

	/* Select VREF */
	stm_adc.chselr = (1 << STM_ADC_CHSEL_VREF);

	/* Start conversion */
	stm_adc.cr |= (1 << STM_ADC_CR_ADSTART);

	/* Wait for conversion complete */
	while ((stm_adc.isr & (1 << STM_ADC_ISR_EOC)) == 0)
		;

	/* Fetch result */
	value = (uint16_t) stm_adc.dr;

	ao_adc_shutdown();
	return value;
}
