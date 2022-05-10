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
 */

#include <ao.h>
#include <ao_exti.h>

static int
_ao_usart_tx_start(struct ao_stm_usart *usart)
{
	if (!ao_fifo_empty(usart->tx_fifo)) {
#if HAS_SERIAL_SW_FLOW
		if (usart->gpio_cts && ao_gpio_get(usart->gpio_cts, usart->pin_cts) == 1) {
			ao_exti_enable(usart->gpio_cts, usart->pin_cts);
			return 0;
		}
#endif
		if (usart->reg->isr & (1 << STM_USART_ISR_TXE))
		{
			usart->tx_running = 1;
			usart->reg->cr1 |= (1 << STM_USART_CR1_TXEIE) | (1 << STM_USART_CR1_TCIE);
			ao_fifo_remove(usart->tx_fifo, usart->reg->tdr);
			ao_wakeup(&usart->tx_fifo);
			return 1;
		}
	}
	return 0;
}

#if HAS_SERIAL_SW_FLOW
static void
_ao_usart_cts(struct ao_stm_usart *usart)
{
	if (_ao_usart_tx_start(usart))
		ao_exti_disable(usart->gpio_cts, usart->pin_cts);
}
#endif

static void
_ao_usart_rx(struct ao_stm_usart *usart, int is_stdin)
{
	if (usart->reg->isr & (1 << STM_USART_ISR_RXNE)) {
		usart->reg->icr = (1 << STM_USART_ICR_ORECF);
		if (!ao_fifo_full(usart->rx_fifo)) {
			ao_fifo_insert(usart->rx_fifo, usart->reg->rdr);
			ao_wakeup(&usart->rx_fifo);
			if (is_stdin)
				ao_wakeup(&ao_stdin_ready);
#if HAS_SERIAL_SW_FLOW
			/* If the fifo is nearly full, turn off RTS and wait
			 * for it to drain a bunch
			 */
			if (usart->gpio_rts && ao_fifo_mostly(usart->rx_fifo)) {
				ao_gpio_set(usart->gpio_rts, usart->pin_rts, 1);
				usart->rts = 0;
			}
#endif
		} else {
			usart->reg->cr1 &= ~(1UL << STM_USART_CR1_RXNEIE);
		}
	}
}

static void
ao_usart_isr(struct ao_stm_usart *usart, int is_stdin)
{
	_ao_usart_rx(usart, is_stdin);

	if (!_ao_usart_tx_start(usart))
		usart->reg->cr1 &= ~(1UL << STM_USART_CR1_TXEIE);

	if (usart->reg->isr & (1 << STM_USART_ISR_TC)) {
		usart->tx_running = 0;
		usart->reg->cr1 &= ~(1UL << STM_USART_CR1_TCIE);
		if (usart->draining) {
			usart->draining = 0;
			ao_wakeup(&usart->tx_fifo);
		}
	}
}

static int
_ao_usart_pollchar(struct ao_stm_usart *usart)
{
	int	c;

	if (ao_fifo_empty(usart->rx_fifo))
		c = AO_READ_AGAIN;
	else {
		uint8_t	u;
		ao_fifo_remove(usart->rx_fifo,u);
		if ((usart->reg->cr1 & (1 << STM_USART_CR1_RXNEIE)) == 0) {
			if (ao_fifo_barely(usart->rx_fifo))
				usart->reg->cr1 |= (1 << STM_USART_CR1_RXNEIE);
		}
#if HAS_SERIAL_SW_FLOW
		/* If we've cleared RTS, check if there's space now and turn it back on */
		if (usart->gpio_rts && usart->rts == 0 && ao_fifo_barely(usart->rx_fifo)) {
			ao_gpio_set(usart->gpio_rts, usart->pin_rts, 0);
			usart->rts = 1;
		}
#endif
		c = u;
	}
	return c;
}

static char
ao_usart_getchar(struct ao_stm_usart *usart)
{
	int c;
	ao_arch_block_interrupts();
	while ((c = _ao_usart_pollchar(usart)) == AO_READ_AGAIN)
		ao_sleep(&usart->rx_fifo);
	ao_arch_release_interrupts();
	return (char) c;
}

#if 0
static inline uint8_t
_ao_usart_sleep_for(struct ao_stm_usart *usart, uint16_t timeout)
{
	return ao_sleep_for(&usart->rx_fifo, timeout);
}
#endif

static void
ao_usart_putchar(struct ao_stm_usart *usart, char c)
{
	ao_arch_block_interrupts();
	while (ao_fifo_full(usart->tx_fifo))
		ao_sleep(&usart->tx_fifo);
	ao_fifo_insert(usart->tx_fifo, c);
	_ao_usart_tx_start(usart);
	ao_arch_release_interrupts();
}

#if 0
static void
ao_usart_drain(struct ao_stm_usart *usart)
{
	ao_arch_block_interrupts();
	while (!ao_fifo_empty(usart->tx_fifo) || usart->tx_running) {
		usart->draining = 1;
		ao_sleep(&usart->tx_fifo);
	}
	ao_arch_release_interrupts();
}
#endif

const uint32_t ao_usart_speeds[] = {
	[AO_SERIAL_SPEED_4800] = 4800,
	[AO_SERIAL_SPEED_9600] = 9600,
	[AO_SERIAL_SPEED_19200] = 19200,
	[AO_SERIAL_SPEED_57600] = 57600,
	[AO_SERIAL_SPEED_115200] = 115200,
};

static void
ao_usart_set_speed(struct ao_stm_usart *usart, uint8_t speed)
{
	if (speed > AO_SERIAL_SPEED_115200)
		return;
	usart->reg->brr = AO_PCLK2 / ao_usart_speeds[speed];
}

static void
ao_usart_init(struct ao_stm_usart *usart, int hw_flow)
{
	usart->reg->cr1 = ((0 << STM_USART_CR1_M1) |
			   (0 << STM_USART_CR1_EOBIE) |
			   (0 << STM_USART_CR1_RTOIE) |
			   (0 << STM_USART_CR1_DEAT) |
			   (0 << STM_USART_CR1_DEDT) |
			   (0 << STM_USART_CR1_OVER8) |
			   (0 << STM_USART_CR1_CMIE) |
			   (0 << STM_USART_CR1_MME) |
			   (0 << STM_USART_CR1_M0) |
			   (0 << STM_USART_CR1_WAKE) |
			   (0 << STM_USART_CR1_PCE) |
			   (0 << STM_USART_CR1_PS) |
			   (0 << STM_USART_CR1_PEIE) |
			   (0 << STM_USART_CR1_TXEIE) |
			   (0 << STM_USART_CR1_TCIE) |
			   (1 << STM_USART_CR1_RXNEIE) |
			   (0 << STM_USART_CR1_IDLEIE) |
			   (1 << STM_USART_CR1_TE) |
			   (1 << STM_USART_CR1_RE) |
			   (0 << STM_USART_CR1_UESM) |
			   (0 << STM_USART_CR1_UE));

	usart->reg->cr2 = ((0 << STM_USART_CR2_ADD) |
			   (0 << STM_USART_CR2_RTOEN) |
			   (0 << STM_USART_CR2_ABRMOD) |
			   (0 << STM_USART_CR2_ABREN) |
			   (0 << STM_USART_CR2_MSBFIRST) |
			   (0 << STM_USART_CR2_DATAINV) |
			   (0 << STM_USART_CR2_TXINV) |
			   (0 << STM_USART_CR2_RXINV) |
			   (0 << STM_USART_CR2_SWAP) |
			   (0 << STM_USART_CR2_LINEN) |
			   (0 << STM_USART_CR2_STOP) |
			   (0 << STM_USART_CR2_CLKEN) |
			   (0 << STM_USART_CR2_CPOL) |
			   (0 << STM_USART_CR2_CHPA) |
			   (0 << STM_USART_CR2_LBCL) |
			   (0 << STM_USART_CR2_LBDIE) |
			   (0 << STM_USART_CR2_LBDL) |
			   (0 << STM_USART_CR2_ADDM7));

	uint32_t cr3 = ((0 << STM_USART_CR3_WUFIE) |
			(0 << STM_USART_CR3_WUS) |
			(0 << STM_USART_CR3_SCARCNT) |
			(0 << STM_USART_CR3_DEP) |
			(0 << STM_USART_CR3_DEM) |
			(0 << STM_USART_CR3_DDRE) |
			(0 << STM_USART_CR3_OVRDIS) |
			(0 << STM_USART_CR3_ONEBIT) |
			(0 << STM_USART_CR3_CTIIE) |
			(0 << STM_USART_CR3_CTSE) |
			(0 << STM_USART_CR3_RTSE) |
			(0 << STM_USART_CR3_DMAT) |
			(0 << STM_USART_CR3_DMAR) |
			(0 << STM_USART_CR3_SCEN) |
			(0 << STM_USART_CR3_NACK) |
			(0 << STM_USART_CR3_HDSEL) |
			(0 << STM_USART_CR3_IRLP) |
			(0 << STM_USART_CR3_IREN) |
			(0 << STM_USART_CR3_EIE));

	if (hw_flow)
		cr3 |= ((1 << STM_USART_CR3_CTSE) |
			(1 << STM_USART_CR3_RTSE));

	usart->reg->cr3 = cr3;

	/* Pick a 9600 baud rate */
	ao_usart_set_speed(usart, AO_SERIAL_SPEED_9600);

	/* Enable the usart */
	usart->reg->cr1 |= (1 << STM_USART_CR1_UE);
}

#if HAS_SERIAL_1

struct ao_stm_usart ao_stm_usart1;

void stm_usart1_isr(void) { ao_usart_isr(&ao_stm_usart1, USE_SERIAL_1_STDIN); }

char
ao_serial1_getchar(void)
{
	return ao_usart_getchar(&ao_stm_usart1);
}

void
ao_serial1_putchar(char c)
{
	ao_usart_putchar(&ao_stm_usart1, c);
}

int
_ao_serial1_pollchar(void)
{
	return _ao_usart_pollchar(&ao_stm_usart1);
}

#if 0
uint8_t
_ao_serial1_sleep_for(uint16_t timeout)
{
	return _ao_usart_sleep_for(&ao_stm_usart1, timeout);
}
#endif

#if 0
void
ao_serial1_drain(void)
{
	ao_usart_drain(&ao_stm_usart1);
}

void
ao_serial1_set_speed(uint8_t speed)
{
	ao_usart_drain(&ao_stm_usart1);
	ao_usart_set_speed(&ao_stm_usart1, speed);
}
#endif
#endif	/* HAS_SERIAL_1 */

#if HAS_SERIAL_2

struct ao_stm_usart ao_stm_usart2;

void stm_usart2_isr(void) { ao_usart_isr(&ao_stm_usart2, USE_SERIAL_2_STDIN); }

char
ao_serial2_getchar(void)
{
	return ao_usart_getchar(&ao_stm_usart2);
}

void
ao_serial2_putchar(char c)
{
	ao_usart_putchar(&ao_stm_usart2, c);
}

int
_ao_serial2_pollchar(void)
{
	return _ao_usart_pollchar(&ao_stm_usart2);
}

#if 0
uint8_t
_ao_serial2_sleep_for(uint16_t timeout)
{
	return _ao_usart_sleep_for(&ao_stm_usart2, timeout);
}

void
ao_serial2_drain(void)
{
	ao_usart_drain(&ao_stm_usart2);
}

void
ao_serial2_set_speed(uint8_t speed)
{
	ao_usart_drain(&ao_stm_usart2);
	ao_usart_set_speed(&ao_stm_usart2, speed);
}
#endif

#if USE_SERIAL_2_FLOW && USE_SERIAL_2_SW_FLOW
void
ao_serial2_cts(void)
{
	_ao_usart_cts(&ao_stm_usart2);
}
#endif

#endif	/* HAS_SERIAL_2 */

#if HAS_SERIAL_SW_FLOW
static void
ao_serial_set_sw_rts_cts(struct ao_stm_usart *usart,
			 void (*isr)(void),
			 struct stm_gpio *port_rts,
			 int pin_rts,
			 struct stm_gpio *port_cts,
			 int pin_cts)
{
	/* Pull RTS low to note that there's space in the FIFO
	 */
	ao_enable_output(port_rts, pin_rts, 0);
	usart->gpio_rts = port_rts;
	usart->pin_rts = pin_rts;
	usart->rts = 1;

	ao_exti_setup(port_cts, pin_cts, AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED, isr);
	usart->gpio_cts = port_cts;
	usart->pin_cts = pin_cts;
}
#endif

#if 0
void
ao_serial_shutdown(void)
{
# if SERIAL_2_PA2_PA3
	stm_moder_set(&stm_gpioa, 2, STM_MODER_INPUT);
	stm_moder_set(&stm_gpioa, 3, STM_MODER_INPUT);
# elif SERIAL_2_PA9_PA10
	stm_moder_set(&stm_gpioa, 9, STM_MODER_INPUT);
	stm_moder_set(&stm_gpioa, 10, STM_MODER_INPUT);
# elif SERIAL_2_PA14_PA15
	stm_moder_set(&stm_gpioa, 14, STM_MODER_INPUT);
	stm_moder_set(&stm_gpioa, 15, STM_MODER_INPUT);
# elif SERIAL_2_PB6_PB7
	stm_moder_set(&stm_gpiob, 6, STM_MODER_INPUT);
	stm_moder_set(&stm_gpiob, 7, STM_MODER_INPUT);
#endif
#if HAS_SERIAL_1
	stm_rcc.apb2enr &= ~(1 << STM_RCC_APB2ENR_USART1EN);
#endif
#if HAS_SERIAL_2
	stm_nvic_set_disable(STM_ISR_USART2_POS);
	stm_rcc.apb1enr &= ~(1 << STM_RCC_APB1ENR_USART2EN);
#endif
}
#endif

void
ao_serial_init(void)
{
#if HAS_SERIAL_1
#endif

#if HAS_SERIAL_2
	/*
	 *	TX	RX
	 *	PA2	PA3
	 *	PA9	PA10
	 *	PA14	PA15
	 *	PB6	PB7
	 */

# if SERIAL_2_PA2_PA3
	ao_enable_port(&stm_gpioa);
	stm_afr_set(&stm_gpioa, 2, STM_AFR_AF4);
	stm_afr_set(&stm_gpioa, 3, STM_AFR_AF4);
# elif SERIAL_2_PA9_PA10
	ao_enable_port(&stm_gpioa);
	stm_afr_set(&stm_gpioa, 9, STM_AFR_AF4);
	stm_afr_set(&stm_gpioa, 10, STM_AFR_AF4);
# elif SERIAL_2_PA14_PA15
	ao_enable_port(&stm_gpioa);
	stm_afr_set(&stm_gpioa, 14, STM_AFR_AF4);
	stm_afr_set(&stm_gpioa, 15, STM_AFR_AF4);
# elif SERIAL_2_PB6_PB7
	ao_enable_port(&stm_gpiob);
	stm_afr_set(&stm_gpiob, 6, STM_AFR_AF0);
	stm_afr_set(&stm_gpiob, 7, STM_AFR_AF0);
# else
#  error "No SERIAL_2 port configuration specified"
# endif
	/* Enable USART */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_USART2EN);

	ao_stm_usart2.reg = &stm_usart2;
	ao_usart_init(&ao_stm_usart2, USE_SERIAL_2_FLOW && !USE_SERIAL_2_SW_FLOW);

	stm_nvic_set_enable(STM_ISR_USART2_POS);
	stm_nvic_set_priority(STM_ISR_USART2_POS, 4);
# if USE_SERIAL_2_STDIN && !DELAY_SERIAL_2_STDIN
	ao_add_stdio(_ao_serial2_pollchar,
		     ao_serial2_putchar,
		     NULL);
# endif
#endif
}
