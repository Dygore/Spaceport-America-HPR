/*
 * Copyright © 2020 Keith Packard <keithp@keithp.com>
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

static uint8_t		ao_spi_mutex;
static uint8_t		ao_spi_pin_config;

#define AO_DMA_SPI1_RX_INDEX	STM_DMA_INDEX(2)
#define AO_DMA_SPI1_RX_CSELR	STM_DMA_CSELR_C2S_SPI1_RX
#define AO_DMA_SPI1_TX_INDEX	STM_DMA_INDEX(3)
#define AO_DMA_SPI1_TX_CSELR	STM_DMA_CSELR_C3S_SPI1_TX

#if 0
static uint8_t	spi_dev_null;

static void
ao_spi_set_dma_mosi(uint8_t id, const void *data, uint16_t len, uint32_t minc)
{
	(void) id;
	struct stm_spi *stm_spi = &stm_spi1;

	ao_dma_set_transfer(AO_DMA_SPI1_TX_INDEX,
			    &stm_spi->dr,
			    (void *) data,
			    len,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_MEDIUM << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (minc << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_MEM_TO_PER << STM_DMA_CCR_DIR));
}

static void
ao_spi_set_dma_miso(uint8_t id, void *data, uint16_t len, uint32_t minc)
{
	(void) id;
	uint8_t	miso_dma_index = STM_DMA_INDEX(2);
	struct stm_spi *stm_spi = &stm_spi1;

	ao_dma_set_transfer(miso_dma_index,
			    &stm_spi->dr,
			    data,
			    len,
			    (0 << STM_DMA_CCR_MEM2MEM) |
			    (STM_DMA_CCR_PL_HIGH << STM_DMA_CCR_PL) |
			    (STM_DMA_CCR_MSIZE_8 << STM_DMA_CCR_MSIZE) |
			    (STM_DMA_CCR_PSIZE_8 << STM_DMA_CCR_PSIZE) |
			    (minc << STM_DMA_CCR_MINC) |
			    (0 << STM_DMA_CCR_PINC) |
			    (0 << STM_DMA_CCR_CIRC) |
			    (STM_DMA_CCR_DIR_PER_TO_MEM << STM_DMA_CCR_DIR));
}

static void
ao_spi_run(uint8_t id, uint8_t which, uint16_t len)
{
	(void) id;
	uint8_t	mosi_dma_index = STM_DMA_INDEX(3);
	uint8_t	miso_dma_index = STM_DMA_INDEX(2);
	struct stm_spi *stm_spi = &stm_spi1;

	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(1 << STM_SPI_CR2_TXDMAEN) |
			(1 << STM_SPI_CR2_RXDMAEN));

	ao_dma_start(miso_dma_index);
	ao_dma_start(mosi_dma_index);

	ao_arch_critical(
		while (!ao_dma_done[miso_dma_index])
			ao_sleep(&ao_dma_done[miso_dma_index]);
		);

	while ((stm_spi->sr & (1 << STM_SPI_SR_TXE)) == 0);
	while (stm_spi->sr & (1 << STM_SPI_SR_BSY));

	stm_spi->cr2 = 0;

	ao_dma_done_transfer(mosi_dma_index);
	ao_dma_done_transfer(miso_dma_index);
}
#endif

void
ao_spi_send(const void *block, uint16_t len, uint8_t spi_index)
{
	(void) spi_index;
#if 0
	/* Set up the transmit DMA to deliver data */
	ao_spi_set_dma_mosi(id, block, len, 1);

	/* Set up the receive DMA -- when this is done, we know the SPI unit
	 * is idle. Without this, we'd have to poll waiting for the BSY bit to
	 * be cleared
	 */
	ao_spi_set_dma_miso(id, &spi_dev_null, len, 0);

	ao_spi_run(id, 1, len);
#else
	const uint8_t *bytes = block;
	struct stm_spi *stm_spi = &stm_spi1;

	while (len--) {
		while ((stm_spi->sr & (1 << STM_SPI_SR_TXE)) == 0);
		stm_spi->dr = *bytes++;
		while ((stm_spi->sr & (1 << STM_SPI_SR_RXNE)) == 0);
		(void) stm_spi->dr;
	}
#endif
}

#if 0
void
ao_spi_send_fixed(uint8_t value, uint16_t len, uint8_t spi_index)
{
	uint8_t id = AO_SPI_INDEX(spi_index);

	/* Set up the transmit DMA to deliver data */
	ao_spi_set_dma_mosi(id, &value, len, 0);

	/* Set up the receive DMA -- when this is done, we know the SPI unit
	 * is idle. Without this, we'd have to poll waiting for the BSY bit to
	 * be cleared
	 */
	ao_spi_set_dma_miso(id, &spi_dev_null, len, 0);

	ao_spi_run(id, 3, len);
}

void
ao_spi_start_bytes(uint8_t spi_index)
{
	(void) spi_index;
	struct stm_spi *stm_spi = &stm_spi1;

	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(0 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));
}

void
ao_spi_stop_bytes(uint8_t spi_index)
{
	(void) spi_index;
	struct stm_spi *stm_spi = &stm_spi1;

	while ((stm_spi->sr & (1 << STM_SPI_SR_TXE)) == 0)
		;
	while (stm_spi->sr & (1 << STM_SPI_SR_BSY))
		;
	/* Clear the OVR flag */
	(void) stm_spi->dr;
	(void) stm_spi->sr;
	stm_spi->cr2 = 0;
}

void
ao_spi_send_sync(const void *block, uint16_t len, uint8_t spi_index)
{
	(void) spi_index;
	const uint8_t	*b = block;
	struct stm_spi	*stm_spi = &stm_spi1;

	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(0 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));
	while (len--) {
		while (!(stm_spi->sr & (1 << STM_SPI_SR_TXE)));
		stm_spi->dr = *b++;
	}
	while ((stm_spi->sr & (1 << STM_SPI_SR_TXE)) == 0)
		;
	while (stm_spi->sr & (1 << STM_SPI_SR_BSY))
		;
	/* Clear the OVR flag */
	(void) stm_spi->dr;
	(void) stm_spi->sr;
}
#endif

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index)
{
	(void) spi_index;
#if 0

	spi_dev_null = 0xff;

	/* Set up transmit DMA to make the SPI hardware actually run */
	ao_spi_set_dma_mosi(id, &spi_dev_null, len, 0);

	/* Set up the receive DMA to capture data */
	ao_spi_set_dma_miso(id, block, len, 1);

	ao_spi_run(id, 9, len);
#else
	uint8_t *bytes = block;
	struct stm_spi *stm_spi = &stm_spi1;

	while (len--) {
		while ((stm_spi->sr & (1 << STM_SPI_SR_TXE)) == 0);
		stm_spi->dr = 0xff;
		while ((stm_spi->sr & (1 << STM_SPI_SR_RXNE)) == 0);
		*bytes++ = (uint8_t) stm_spi->dr;
	}
#endif
}

#if 0
void
ao_spi_duplex(const void *out, void *in, uint16_t len, uint8_t spi_index)
{
	uint8_t		id = AO_SPI_INDEX(spi_index);

	/* Set up transmit DMA to send data */
	ao_spi_set_dma_mosi(id, out, len, 1);

	/* Set up the receive DMA to capture data */
	ao_spi_set_dma_miso(id, in, len, 1);

	ao_spi_run(id, 11, len);
}
#endif

static void
ao_spi_disable_pin_config(uint8_t spi_pin_config)
{
	/* Disable current config
	 */
	switch (spi_pin_config) {
	case AO_SPI_1_PA5_PA6_PA7:
		stm_gpio_set(&stm_gpioa, 5, 1);
		stm_moder_set(&stm_gpioa, 5, STM_MODER_OUTPUT);
		stm_moder_set(&stm_gpioa, 6, STM_MODER_INPUT);
		stm_moder_set(&stm_gpioa, 7, STM_MODER_OUTPUT);
		break;
	case AO_SPI_1_PA12_PA13_PA14:
		stm_gpio_set(&stm_gpioa, 13, 1);
		stm_moder_set(&stm_gpioa, 13, STM_MODER_OUTPUT);	/* clk */
		stm_moder_set(&stm_gpioa, 12, STM_MODER_OUTPUT);	/* mosi */
		stm_moder_set(&stm_gpioa, 14, STM_MODER_INPUT);		/* miso */
		break;
	case AO_SPI_1_PB3_PB4_PB5:
		stm_gpio_set(&stm_gpiob, 3, 1);
		stm_moder_set(&stm_gpiob, 3, STM_MODER_OUTPUT);
		stm_moder_set(&stm_gpiob, 4, STM_MODER_INPUT);
		stm_moder_set(&stm_gpiob, 5, STM_MODER_OUTPUT);
		break;
	}
}

static void
ao_spi_enable_pin_config(uint8_t spi_pin_config)
{
	/* Enable new config
	 */
	switch (spi_pin_config) {
	case AO_SPI_1_PA5_PA6_PA7:
		stm_afr_set(&stm_gpioa, 5, STM_AFR_AF0);
		stm_afr_set(&stm_gpioa, 6, STM_AFR_AF0);
		stm_afr_set(&stm_gpioa, 7, STM_AFR_AF0);
		break;
	case AO_SPI_1_PA12_PA13_PA14:
		stm_afr_set(&stm_gpioe, 12, STM_AFR_AF0);	/* yes, AF0 */
		stm_afr_set(&stm_gpioe, 13, STM_AFR_AF5);
		stm_afr_set(&stm_gpioe, 14, STM_AFR_AF5);
		break;
	case AO_SPI_1_PB3_PB4_PB5:
		stm_afr_set(&stm_gpiob, 3, STM_AFR_AF0);
		stm_afr_set(&stm_gpiob, 4, STM_AFR_AF0);
		stm_afr_set(&stm_gpiob, 5, STM_AFR_AF0);
		break;
	}
}

static void
ao_spi_config(uint8_t spi_index, uint32_t speed)
{
	uint8_t		spi_pin_config = AO_SPI_PIN_CONFIG(spi_index);
	struct stm_spi	*stm_spi = &stm_spi1;

	if (spi_pin_config != ao_spi_pin_config) {

		/* Disable old config
		 */
		ao_spi_disable_pin_config(ao_spi_pin_config);

		/* Enable new config
		 */
		ao_spi_enable_pin_config(spi_pin_config);

		/* Remember current config
		 */
		ao_spi_pin_config = spi_pin_config;
	}

	/* Turn the SPI transceiver on and set the mode */
	stm_spi->cr1 = ((0 << STM_SPI_CR1_BIDIMODE) |		/* Three wire mode */
			(0 << STM_SPI_CR1_BIDIOE) |
			(0 << STM_SPI_CR1_CRCEN) |		/* CRC disabled */
			(0 << STM_SPI_CR1_CRCNEXT) |
			(0 << STM_SPI_CR1_DFF) |
			(0 << STM_SPI_CR1_RXONLY) |
			(1 << STM_SPI_CR1_SSM) |        	/* Software SS handling */
			(1 << STM_SPI_CR1_SSI) |		/*  ... */
			(0 << STM_SPI_CR1_LSBFIRST) |		/* Big endian */
			(1 << STM_SPI_CR1_SPE) |		/* Enable SPI unit */
			(speed << STM_SPI_CR1_BR) |		/* baud rate to pclk/4 */
			(1 << STM_SPI_CR1_MSTR) |
			(AO_SPI_CPOL(spi_index) << STM_SPI_CR1_CPOL) |	/* Format */
			(AO_SPI_CPHA(spi_index) << STM_SPI_CR1_CPHA));
}

#if HAS_TASK
uint8_t
ao_spi_try_get(uint8_t spi_index, uint32_t speed, uint8_t task_id)
{
	uint8_t		id = AO_SPI_INDEX(spi_index);

	if (!ao_mutex_try(&ao_spi_mutex[id], task_id))
		return 0;
	ao_spi_config(spi_index, speed);
	return 1;
}
#endif

void
ao_spi_get(uint8_t spi_index, uint32_t speed)
{
	(void) spi_index;

	ao_mutex_get(&ao_spi_mutex);
	ao_spi_config(spi_index, speed);
}

void
ao_spi_put(uint8_t spi_index)
{
	(void) spi_index;
	struct stm_spi	*stm_spi = &stm_spi1;

	stm_spi->cr1 = 0;
	ao_mutex_put(&ao_spi_mutex);
}

static void
ao_spi_channel_init(uint8_t spi_index)
{
	(void) spi_index;
	struct stm_spi	*stm_spi = &stm_spi1;

	ao_spi_disable_pin_config(AO_SPI_PIN_CONFIG(spi_index));

	stm_spi->cr1 = 0;
	stm_spi->cr2 = ((0 << STM_SPI_CR2_TXEIE) |
			(0 << STM_SPI_CR2_RXNEIE) |
			(0 << STM_SPI_CR2_ERRIE) |
			(0 << STM_SPI_CR2_SSOE) |
			(0 << STM_SPI_CR2_TXDMAEN) |
			(0 << STM_SPI_CR2_RXDMAEN));

	/* Clear any pending data and error flags */
	(void) stm_spi->dr;
	(void) stm_spi->sr;
}

#if DEBUG
void
ao_spi_dump_cmd(void)
{
	int s;

	for (s = 0; s < 64; s++) {
		int i = (spi_task_index + s) & 63;
		if (spi_tasks[i].which) {
			int t;
			const char *name = "(none)";
			for (t = 0; t < ao_num_tasks; t++)
				if (ao_tasks[t]->task_id == spi_tasks[i].task) {
					name = ao_tasks[t]->name;
					break;
				}
			printf("%2d: %5d task %2d which %2d len %5d %s\n",
			       s,
			       spi_tasks[i].tick,
			       spi_tasks[i].task,
			       spi_tasks[i].which,
			       spi_tasks[i].len,
			       name);
		}
	}
	for (s = 0; s < STM_NUM_SPI; s++) {
		struct stm_spi *spi = ao_spi_stm_info[s].stm_spi;

		printf("%1d: mutex %2d index %3d miso dma %3d mosi dma %3d",
		       s, ao_spi_mutex[s], ao_spi_index[s],
		       ao_spi_stm_info[s].miso_dma_index,
		       ao_spi_stm_info[s].mosi_dma_index);
		printf(" cr1 %04x cr2 %02x sr %03x\n",
		       spi->cr1, spi->cr2, spi->sr);
	}

}

static const struct ao_cmds ao_spi_cmds[] = {
	{ ao_spi_dump_cmd, 	"S\0Dump SPI status" },
	{ 0, NULL }
};
#endif

void
ao_spi_init(void)
{
#if HAS_SPI_1
# if SPI_1_PA5_PA6_PA7
	ao_enable_port(&stm_gpioa);
	stm_ospeedr_set(&stm_gpioa, 5, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpioa, 6, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpioa, 7, SPI_1_OSPEEDR);
# endif
# if SPI_1_PB3_PB4_PB5
	ao_enable_port(&stm_gpiob);
	stm_ospeedr_set(&stm_gpiob, 3, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpiob, 4, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpiob, 5, SPI_1_OSPEEDR);
# endif
# if SPI_1_PE13_PE14_PE15
	ao_enable_port(&stm_gpioe);
	stm_ospeedr_set(&stm_gpioe, 13, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpioe, 14, SPI_1_OSPEEDR);
	stm_ospeedr_set(&stm_gpioe, 15, SPI_1_OSPEEDR);
# endif
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_SPI1EN);
	ao_spi_pin_config = AO_SPI_CONFIG_NONE;
	ao_spi_channel_init(0);
#if 0
	ao_dma_alloc(AO_DMA_SPI1_RX_INDEX, AO_DMA_SPI1_RX_CSELR);
	ao_dma_alloc(AO_DMA_SPI1_TX_INDEX, AO_DMA_SPI1_TX_CSELR);
#endif
#endif

#if HAS_SPI_2
# if SPI_2_PB13_PB14_PB15
	ao_enable_port(&stm_gpiob);
	stm_ospeedr_set(&stm_gpiob, 13, SPI_2_OSPEEDR);
	stm_ospeedr_set(&stm_gpiob, 14, SPI_2_OSPEEDR);
	stm_ospeedr_set(&stm_gpiob, 15, SPI_2_OSPEEDR);
# define HAS_SPI_2_CONFIG 1
# endif
# if SPI_2_PD1_PD3_PD4
	ao_enable_port(&stm_gpiod);
	stm_rcc.ahbenr |= (1 << STM_RCC_AHBENR_GPIODEN);
	stm_ospeedr_set(&stm_gpiod, 1, SPI_2_OSPEEDR);
	stm_ospeedr_set(&stm_gpiod, 3, SPI_2_OSPEEDR);
	stm_ospeedr_set(&stm_gpiod, 4, SPI_2_OSPEEDR);
# define HAS_SPI_2_CONFIG 1
# endif
# ifndef HAS_SPI_2_CONFIG 1
	#error "no config for SPI2"
# endif
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_SPI2EN);
	ao_spi_pin_config[1] = AO_SPI_CONFIG_NONE;
	ao_spi_channel_init(1);
	ao_dma_alloc(AO_DMA_SPI2_RX_INDEX, AO_DMA_SPI2_RX_CSELR);
	ao_dma_alloc(AO_DMA_SPI2_TX_INDEX, AO_DMA_SPI2_TX_CSELR);
#endif
#if DEBUG
	ao_cmd_register(&ao_spi_cmds[0]);
#endif
}
