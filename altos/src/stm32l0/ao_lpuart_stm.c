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
#include <ao_lpuart.h>

struct ao_stm_lpuart {
	struct ao_fifo		rx_fifo;
	struct ao_fifo		tx_fifo;
	struct stm_lpuart	*reg;
	uint8_t			tx_running;
	uint8_t			draining;
#if HAS_SERIAL_SW_FLOW
	/* RTS - 0 if we have FIFO space, 1 if not
	 * CTS - 0 if we can send, 0 if not
	 */
	struct stm_gpio		*gpio_rts;
	struct stm_gpio		*gpio_cts;
	uint8_t			pin_rts;
	uint8_t			pin_cts;
	uint8_t			rts;
#endif
};

static int
_ao_lpuart_tx_start(struct ao_stm_lpuart *lpuart)
{
	if (!ao_fifo_empty(lpuart->tx_fifo)) {
#if HAS_LPUART_SW_FLOW
		if (lpuart->gpio_cts && ao_gpio_get(lpuart->gpio_cts, lpuart->pin_cts) == 1) {
			ao_exti_enable(lpuart->gpio_cts, lpuart->pin_cts);
			return 0;
		}
#endif
		if (lpuart->reg->isr & (1 << STM_LPUART_ISR_TXE))
		{
			lpuart->tx_running = 1;
			lpuart->reg->cr1 |= (1 << STM_LPUART_CR1_TXEIE) | (1 << STM_LPUART_CR1_TCIE);
			ao_fifo_remove(lpuart->tx_fifo, lpuart->reg->tdr);
			ao_wakeup(&lpuart->tx_fifo);
			return 1;
		}
	}
	return 0;
}

#if HAS_LPUART_SW_FLOW
static void
_ao_lpuart_cts(struct ao_stm_lpuart *lpuart)
{
	if (_ao_lpuart_tx_start(lpuart))
		ao_exti_disable(lpuart->gpio_cts, lpuart->pin_cts);
}
#endif

static void
_ao_lpuart_rx(struct ao_stm_lpuart *lpuart, int is_stdin)
{
	if (lpuart->reg->isr & (1 << STM_LPUART_ISR_RXNE)) {
		lpuart->reg->icr = (1 << STM_LPUART_ICR_ORECF);
		if (!ao_fifo_full(lpuart->rx_fifo)) {
			ao_fifo_insert(lpuart->rx_fifo, lpuart->reg->rdr);
			ao_wakeup(&lpuart->rx_fifo);
			if (is_stdin)
				ao_wakeup(&ao_stdin_ready);
#if HAS_LPUART_SW_FLOW
			/* If the fifo is nearly full, turn off RTS and wait
			 * for it to drain a bunch
			 */
			if (lpuart->gpio_rts && ao_fifo_mostly(lpuart->rx_fifo)) {
				ao_gpio_set(lpuart->gpio_rts, lpuart->pin_rts, 1);
				lpuart->rts = 0;
			}
#endif
		} else {
			lpuart->reg->cr1 &= ~(1UL << STM_LPUART_CR1_RXNEIE);
		}
	}
}

static void
ao_lpuart_isr(struct ao_stm_lpuart *lpuart, int is_stdin)
{
	_ao_lpuart_rx(lpuart, is_stdin);

	if (!_ao_lpuart_tx_start(lpuart))
		lpuart->reg->cr1 &= ~(1UL << STM_LPUART_CR1_TXEIE);

	if (lpuart->reg->isr & (1 << STM_LPUART_ISR_TC)) {
		lpuart->tx_running = 0;
		lpuart->reg->cr1 &= ~(1UL << STM_LPUART_CR1_TCIE);
		if (lpuart->draining) {
			lpuart->draining = 0;
			ao_wakeup(&lpuart->tx_fifo);
		}
	}
}

static int
_ao_lpuart_pollchar(struct ao_stm_lpuart *lpuart)
{
	int	c;

	if (ao_fifo_empty(lpuart->rx_fifo))
		c = AO_READ_AGAIN;
	else {
		uint8_t	u;
		ao_fifo_remove(lpuart->rx_fifo,u);
		if ((lpuart->reg->cr1 & (1 << STM_LPUART_CR1_RXNEIE)) == 0) {
			if (ao_fifo_barely(lpuart->rx_fifo))
				lpuart->reg->cr1 |= (1 << STM_LPUART_CR1_RXNEIE);
		}
#if HAS_LPUART_SW_FLOW
		/* If we've cleared RTS, check if there's space now and turn it back on */
		if (lpuart->gpio_rts && lpuart->rts == 0 && ao_fifo_barely(lpuart->rx_fifo)) {
			ao_gpio_set(lpuart->gpio_rts, lpuart->pin_rts, 0);
			lpuart->rts = 1;
		}
#endif
		c = u;
	}
	return c;
}

static char
ao_lpuart_getchar(struct ao_stm_lpuart *lpuart)
{
	int c;
	ao_arch_block_interrupts();
	while ((c = _ao_lpuart_pollchar(lpuart)) == AO_READ_AGAIN)
		ao_sleep(&lpuart->rx_fifo);
	ao_arch_release_interrupts();
	return (char) c;
}

#if 0
static inline uint8_t
_ao_lpuart_sleep_for(struct ao_stm_lpuart *lpuart, uint16_t timeout)
{
	return ao_sleep_for(&lpuart->rx_fifo, timeout);
}
#endif

static void
ao_lpuart_putchar(struct ao_stm_lpuart *lpuart, char c)
{
	ao_arch_block_interrupts();
	while (ao_fifo_full(lpuart->tx_fifo))
		ao_sleep(&lpuart->tx_fifo);
	ao_fifo_insert(lpuart->tx_fifo, c);
	_ao_lpuart_tx_start(lpuart);
	ao_arch_release_interrupts();
}

static void
ao_lpuart_drain(struct ao_stm_lpuart *lpuart)
{
	ao_arch_block_interrupts();
	while (!ao_fifo_empty(lpuart->tx_fifo) || lpuart->tx_running) {
		lpuart->draining = 1;
		ao_sleep(&lpuart->tx_fifo);
	}
	ao_arch_release_interrupts();
}

extern const uint32_t ao_usart_speeds[];

static void
ao_lpuart_set_speed(struct ao_stm_lpuart *lpuart, uint8_t speed)
{
	if (speed > AO_SERIAL_SPEED_115200)
		return;
	lpuart->reg->brr = 256 * AO_PCLK1 / ao_usart_speeds[speed];
}

static void
ao_lpuart_enable(struct ao_stm_lpuart *lpuart, int hw_flow)
{
	lpuart->reg->cr1 = ((0 << STM_LPUART_CR1_M1) |
			   (0 << STM_LPUART_CR1_DEAT) |
			   (0 << STM_LPUART_CR1_DEDT) |
			   (0 << STM_LPUART_CR1_CMIE) |
			   (0 << STM_LPUART_CR1_MME) |
			   (0 << STM_LPUART_CR1_M0) |
			   (0 << STM_LPUART_CR1_WAKE) |
			   (0 << STM_LPUART_CR1_PCE) |
			   (0 << STM_LPUART_CR1_PS) |
			   (0 << STM_LPUART_CR1_PEIE) |
			   (0 << STM_LPUART_CR1_TXEIE) |
			   (0 << STM_LPUART_CR1_TCIE) |
			   (1 << STM_LPUART_CR1_RXNEIE) |
			   (0 << STM_LPUART_CR1_IDLEIE) |
			   (1 << STM_LPUART_CR1_TE) |
			   (1 << STM_LPUART_CR1_RE) |
			   (0 << STM_LPUART_CR1_UESM) |
			   (0 << STM_LPUART_CR1_UE));

	lpuart->reg->cr2 = ((0 << STM_LPUART_CR2_ADD) |
			   (0 << STM_LPUART_CR2_MSBFIRST) |
			   (0 << STM_LPUART_CR2_DATAINV) |
			   (0 << STM_LPUART_CR2_TXINV) |
			   (0 << STM_LPUART_CR2_RXINV) |
			   (0 << STM_LPUART_CR2_SWAP) |
			   (0 << STM_LPUART_CR2_STOP) |
			   (0 << STM_LPUART_CR2_ADDM7));

	uint32_t cr3 = ((0 << STM_LPUART_CR3_UCESM) |
			(0 << STM_LPUART_CR3_WUFIE) |
			(0 << STM_LPUART_CR3_WUS) |
			(0 << STM_LPUART_CR3_DEP) |
			(0 << STM_LPUART_CR3_DEM) |
			(0 << STM_LPUART_CR3_DDRE) |
			(0 << STM_LPUART_CR3_OVRDIS) |
			(0 << STM_LPUART_CR3_CTSIE) |
			(0 << STM_LPUART_CR3_CTSE) |
			(0 << STM_LPUART_CR3_RTSE) |
			(0 << STM_LPUART_CR3_DMAT) |
			(0 << STM_LPUART_CR3_DMAR) |
			(0 << STM_LPUART_CR3_HDSEL) |
			(0 << STM_LPUART_CR3_EIE));

	if (hw_flow)
		cr3 |= ((1 << STM_LPUART_CR3_CTSE) |
			(1 << STM_LPUART_CR3_RTSE));

	lpuart->reg->cr3 = cr3;

	/* Pick a 9600 baud rate */
	ao_lpuart_set_speed(lpuart, AO_SERIAL_SPEED_9600);

	/* Enable the lpuart */
	lpuart->reg->cr1 |= (1 << STM_LPUART_CR1_UE);
}

static void
ao_lpuart_disable(struct ao_stm_lpuart *lpuart)
{
	ao_lpuart_drain(lpuart);
	lpuart->reg->cr1 = 0;
}

#if HAS_LPUART_1

struct ao_stm_lpuart ao_stm_lpuart1;

void stm_lpuart1_aes_isr(void) {
	ao_lpuart_isr(&ao_stm_lpuart1, USE_LPUART_1_STDIN);
}

char
ao_lpuart1_getchar(void)
{
	return ao_lpuart_getchar(&ao_stm_lpuart1);
}

void
ao_lpuart1_putchar(char c)
{
	ao_lpuart_putchar(&ao_stm_lpuart1, c);
}

int
_ao_lpuart1_pollchar(void)
{
	return _ao_lpuart_pollchar(&ao_stm_lpuart1);
}

void
ao_lpuart1_drain(void)
{
	ao_lpuart_drain(&ao_stm_lpuart1);
}

void
ao_lpuart1_set_speed(uint8_t speed)
{
	ao_lpuart_drain(&ao_stm_lpuart1);
	ao_lpuart_set_speed(&ao_stm_lpuart1, speed);
}

#endif	/* HAS_LPUART_1 */

#if HAS_LPUART_SW_FLOW
static void
ao_lpuart_set_sw_rts_cts(struct ao_stm_lpuart *lpuart,
			 void (*isr)(void),
			 struct stm_gpio *port_rts,
			 int pin_rts,
			 struct stm_gpio *port_cts,
			 int pin_cts)
{
	/* Pull RTS low to note that there's space in the FIFO
	 */
	ao_enable_output(port_rts, pin_rts, 0);
	lpuart->gpio_rts = port_rts;
	lpuart->pin_rts = pin_rts;
	lpuart->rts = 1;

	ao_exti_setup(port_cts, pin_cts, AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED, isr);
	lpuart->gpio_cts = port_cts;
	lpuart->pin_cts = pin_cts;
}
#endif

void
ao_lpuart1_enable(void)
{
	/*
	 *	TX	RX
	 *	PA1	PA0
	 *	PA2	PA3
	 *	PA4	PA3
	 *	PA14	PA13
	 *	PB6	PB7
	 */

#ifdef HAS_LPUART_1
	/* Clock source defaults to PCLK1, so just leave it */

# if LPUART_1_PA0_PA1
	ao_enable_port(&stm_gpioa);
	stm_afr_set(&stm_gpioa, 0, STM_AFR_AF6);
	stm_afr_set(&stm_gpioa, 1, STM_AFR_AF6);
# else
#  error "No LPUART_1 port configuration specified"
# endif
	/* Enable LPUART */
	stm_rcc.apb1enr |= (1 << STM_RCC_APB1ENR_LPUART1EN);

	ao_stm_lpuart1.reg = &stm_lpuart1;
	ao_lpuart_enable(&ao_stm_lpuart1, USE_LPUART_1_FLOW && !USE_LPUART_1_SW_FLOW);

	stm_nvic_set_enable(STM_ISR_LPUART1_AES_POS);
	stm_nvic_set_priority(STM_ISR_LPUART1_AES_POS, 4);
# if USE_LPUART_1_STDIN && !DELAY_LPUART_1_STDIN
	ao_add_stdio(_ao_lpuart1_pollchar,
		     ao_lpuart1_putchar,
		     NULL);
# endif
#endif
}

void
ao_lpuart1_disable(void)
{
	/* Stop LPUART */
	ao_lpuart_disable(&ao_stm_lpuart1);

	/* Disable interrupts */
	stm_nvic_clear_enable(STM_ISR_LPUART1_AES_POS);

	/* Remap pins to GPIO use */
# if LPUART_1_PA0_PA1
	stm_moder_set(&stm_gpioa, 0, STM_MODER_INPUT);
	stm_moder_set(&stm_gpioa, 1, STM_MODER_OUTPUT);
# else
#  error "No LPUART_1 port configuration specified"
# endif

	/* Disable LPUART */
	stm_rcc.apb1enr &= ~(1UL << STM_RCC_APB1ENR_LPUART1EN);
}
 
