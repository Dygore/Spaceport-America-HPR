/*
 * Copyright © 2015 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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
#include <ao_data.h>

#define AO_ADC_DEBUG	0

static uint8_t	ao_adc_ready;

/*
 * Callback from DMA ISR
 *
 * Mark time in ring, shut down DMA engine
 */
static void ao_adc_done(int index)
{
	(void) index;
	/* Clear ISR bits */
	stm_adc.isr = ((1 << STM_ADC_ISR_AWD) |
		       (1 << STM_ADC_ISR_OVR) |
		       (1 << STM_ADC_ISR_EOSEQ) |
		       (1 << STM_ADC_ISR_EOC));

	AO_DATA_PRESENT(AO_DATA_ADC);
	ao_dma_done_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));
	ao_data_fill(ao_data_head);
	ao_adc_ready = 1;
}

/*
 * Start the ADC sequence using the DMA engine
 */
void
ao_adc_poll(void)
{
	if (!ao_adc_ready)
		return;
	ao_adc_ready = 0;
	stm_adc.isr = 0;
	ao_dma_set_transfer(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1),
			    &stm_adc.dr,
			    (void *) (&ao_data_ring[ao_data_head].adc),
			    AO_NUM_ADC,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_HIGH << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_16 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_16 << STM_DMA_CCR_PSIZE) |
			    (1 << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR) |
			    (1 << STM_DMA_CCR_TCIE));
	ao_dma_set_isr(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1), ao_adc_done);
	ao_dma_start(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));

	stm_adc.cr |= (1 << STM_ADC_CR_ADSTART);
}

static void
ao_adc_dump(void)
{
	struct ao_data	packet;

	ao_data_get(&packet);
	AO_ADC_DUMP(&packet);
}

#if AO_ADC_DEBUG
static void
ao_adc_one(void)
{
	int		ch;
	uint16_t	value;

	ch = ao_cmd_decimal();
	if (ao_cmd_status != ao_cmd_success)
		return;
	if (ch < 0 || AO_NUM_ADC <= ch) {
		ao_cmd_status = ao_cmd_syntax_error;
		return;
	}

	ao_timer_set_adc_interval(0);
	ao_delay(1);

	printf("At top, data %u isr %04x cr %04x\n", stm_adc.dr, stm_adc.isr, stm_adc.cr);

	if (stm_adc.cr & (1 << STM_ADC_CR_ADEN)) {
		printf("Disabling\n"); flush();
		stm_adc.cr |= (1 << STM_ADC_CR_ADDIS);
		while (stm_adc.cr & (1 << STM_ADC_CR_ADDIS))
			;
		printf("Disabled\n"); flush();
	}

	/* Turn off everything */
	stm_adc.cr &= ~((1 << STM_ADC_CR_ADCAL) |
			(1 << STM_ADC_CR_ADSTP) |
			(1 << STM_ADC_CR_ADSTART) |
			(1 << STM_ADC_CR_ADEN));

	printf("After disable, ADC status %04x\n", stm_adc.cr);

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

	stm_adc.chselr = (1 << ch);

	/* Longest sample time */
	stm_adc.smpr = STM_ADC_SMPR_SMP_41_5 << STM_ADC_SMPR_SMP;

	printf("Before enable, ADC status %04x\n", stm_adc.cr); flush();
	/* Enable */
	stm_adc.cr |= (1 << STM_ADC_CR_ADEN);
	while ((stm_adc.isr & (1 << STM_ADC_ISR_ADRDY)) == 0)
		;

	/* Start */
	stm_adc.cr |= (1 << STM_ADC_CR_ADSTART);

	/* Wait for conversion complete */
	while (!(stm_adc.isr & (1 << STM_ADC_ISR_EOC)))
		;

	value = stm_adc.dr;
	printf ("value %u, cr is %04x isr is %04x\n",
		value, stm_adc.cr, stm_adc.isr);


	/* Clear ISR bits */
	stm_adc.isr = ((1 << STM_ADC_ISR_AWD) |
		       (1 << STM_ADC_ISR_OVR) |
		       (1 << STM_ADC_ISR_EOSEQ) |
		       (1 << STM_ADC_ISR_EOC));
}
#endif

const struct ao_cmds ao_adc_cmds[] = {
	{ ao_adc_dump,	"a\0Display current ADC values" },
#if AO_ADC_DEBUG
	{ ao_adc_one,	"A ch\0Display one ADC channel" },
#endif
	{ 0, NULL },
};

void
ao_adc_init(void)
{
	uint32_t	chselr;

	/* Reset ADC */
	stm_rcc.apb2rstr |= (1 << STM_RCC_APB2RSTR_ADCRST);
	stm_rcc.apb2rstr &= ~(1UL << STM_RCC_APB2RSTR_ADCRST);

	/* Turn on ADC pins */
	stm_rcc.ahbenr |= AO_ADC_RCC_AHBENR;

#ifdef AO_ADC_PIN0_PORT
	stm_moder_set(AO_ADC_PIN0_PORT, AO_ADC_PIN0_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN0_PORT, AO_ADC_PIN0_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN1_PORT
	stm_moder_set(AO_ADC_PIN1_PORT, AO_ADC_PIN1_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN1_PORT, AO_ADC_PIN1_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN2_PORT
	stm_moder_set(AO_ADC_PIN2_PORT, AO_ADC_PIN2_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN2_PORT, AO_ADC_PIN2_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN3_PORT
	stm_moder_set(AO_ADC_PIN3_PORT, AO_ADC_PIN3_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN3_PORT, AO_ADC_PIN3_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN4_PORT
	stm_moder_set(AO_ADC_PIN4_PORT, AO_ADC_PIN4_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN4_PORT, AO_ADC_PIN4_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN5_PORT
	stm_moder_set(AO_ADC_PIN5_PORT, AO_ADC_PIN5_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN5_PORT, AO_ADC_PIN5_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN6_PORT
	stm_moder_set(AO_ADC_PIN6_PORT, AO_ADC_PIN6_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN6_PORT, AO_ADC_PIN6_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN7_PORT
	stm_moder_set(AO_ADC_PIN7_PORT, AO_ADC_PIN7_PIN, STM_MODER_ANALOG);
	stm_pupdr_set(AO_ADC_PIN7_PORT, AO_ADC_PIN7_PIN, STM_PUPDR_NONE);
#endif
#ifdef AO_ADC_PIN24_PORT
	#error "Too many ADC ports"
#endif

	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_ADCEN);

	chselr = 0;
#if AO_NUM_ADC > 0
	chselr |= (1 << AO_ADC_PIN0_CH);
#endif
#if AO_NUM_ADC > 1
	chselr |= (1 << AO_ADC_PIN1_CH);
#endif
#if AO_NUM_ADC > 2
	chselr |= (1 << AO_ADC_PIN2_CH);
#endif
#if AO_NUM_ADC > 3
	chselr |= (1 << AO_ADC_PIN3_CH);
#endif
#if AO_NUM_ADC > 4
	chselr |= (1 << AO_ADC_PIN4_CH);
#endif
#if AO_NUM_ADC > 5
	chselr |= (1 << AO_ADC_PIN5_CH);
#endif
#if AO_NUM_ADC > 6
	chselr |= (1 << AO_ADC_PIN6_CH);
#endif
#if AO_NUM_ADC > 7
	chselr |= (1 << AO_ADC_PIN7_CH);
#endif
#if AO_NUM_ADC > 8
#error Need more ADC defines
#endif

	/* Wait for ADC to be idle */
	while (stm_adc.cr & ((1UL << STM_ADC_CR_ADCAL) |
			     (1UL << STM_ADC_CR_ADDIS)))
		;

	/* Disable */
	if (stm_adc.cr & (1 << STM_ADC_CR_ADEN)) {
		stm_adc.cr |= (1 << STM_ADC_CR_ADDIS);
		while (stm_adc.cr & (1 << STM_ADC_CR_ADDIS))
			;
	}

	/* Turn off everything */
	stm_adc.cr &= ~((1UL << STM_ADC_CR_ADCAL) |
			(1UL << STM_ADC_CR_ADSTP) |
			(1UL << STM_ADC_CR_ADSTART) |
			(1UL << STM_ADC_CR_ADEN));

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
			 (1 << STM_ADC_CFGR1_DMAEN));				  /* enable DMA */

	/* Set the clock */
	stm_adc.cfgr2 = STM_ADC_CFGR2_CKMODE_PCLK_2 << STM_ADC_CFGR2_CKMODE;

	/* Shortest sample time */
	stm_adc.smpr = STM_ADC_SMPR_SMP_71_5 << STM_ADC_SMPR_SMP;

	stm_adc.chselr = chselr;

	stm_adc.ccr = ((0 << STM_ADC_CCR_VBATEN) |
		       (0 << STM_ADC_CCR_TSEN) |
		       (0 << STM_ADC_CCR_VREFEN));

	/* Calibrate */
	stm_adc.cr |= (1UL << STM_ADC_CR_ADCAL);
	while ((stm_adc.cr & (1UL << STM_ADC_CR_ADCAL)) != 0)
		;

	/* Enable */
	stm_adc.cr |= (1 << STM_ADC_CR_ADEN);
	while ((stm_adc.isr & (1 << STM_ADC_ISR_ADRDY)) == 0)
		;

	/* Clear any stale status bits */
	stm_adc.isr = 0;

	/* Turn on syscfg */
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SYSCFGCOMPEN);

	/* Set ADC to use DMA channel 1 (option 1) */
	stm_syscfg.cfgr1 &= ~(1UL << STM_SYSCFG_CFGR1_ADC_DMA_RMP);

	ao_dma_alloc(STM_DMA_INDEX(STM_DMA_CHANNEL_ADC_1));

	ao_cmd_register(&ao_adc_cmds[0]);

	ao_adc_ready = 1;
}
