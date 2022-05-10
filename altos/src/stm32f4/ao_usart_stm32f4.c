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

#include <ao.h>
#include <ao_exti.h>

/* ao_serial_stm.c */
struct ao_stm_usart {
	struct ao_fifo		rx_fifo;
	struct ao_fifo		tx_fifo;
	struct stm_usart	*reg;
	uint32_t		clk;
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
_ao_usart_tx_start(struct ao_stm_usart *usart)
{
	if (!ao_fifo_empty(usart->tx_fifo)) {
#if HAS_SERIAL_SW_FLOW
		if (usart->gpio_cts && ao_gpio_get(usart->gpio_cts, usart->pin_cts) == 1) {
			ao_exti_enable(usart->gpio_cts, usart->pin_cts);
			return 0;
		}
#endif
		if (usart->reg->sr & (1 << STM_USART_SR_TXE))
		{
			usart->tx_running = 1;
			usart->reg->cr1 |= (1 << STM_USART_CR1_TXEIE) | (1 << STM_USART_CR1_TCIE);
			ao_fifo_remove(usart->tx_fifo, usart->reg->dr);
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
	if (usart->reg->sr & (1 << STM_USART_SR_RXNE)) {
		if (!ao_fifo_full(usart->rx_fifo)) {
			ao_fifo_insert(usart->rx_fifo, usart->reg->dr);
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
			usart->reg->cr1 &= ~(1 << STM_USART_CR1_RXNEIE);
		}
	}
}

static void
ao_usart_isr(struct ao_stm_usart *usart, int is_stdin)
{
	_ao_usart_rx(usart, is_stdin);

	if (!_ao_usart_tx_start(usart))
		usart->reg->cr1 &= ~(1<< STM_USART_CR1_TXEIE);

	if (usart->reg->sr & (1 << STM_USART_SR_TC)) {
		usart->tx_running = 0;
		usart->reg->cr1 &= ~(1 << STM_USART_CR1_TCIE);
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

static inline uint8_t
_ao_usart_sleep_for(struct ao_stm_usart *usart, uint16_t timeout)
{
	return ao_sleep_for(&usart->rx_fifo, timeout);
}

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

static void
ao_usart_set_speed(struct ao_stm_usart *usart, uint32_t speed)
{
	if (speed > 115200)
		return;
	usart->reg->brr = usart->clk / speed;
}

static void
_ao_usart_init(struct ao_stm_usart *usart, int hw_flow)
{
	usart->reg->cr1 = ((0 << STM_USART_CR1_OVER8) |
			  (1 << STM_USART_CR1_UE) |
			  (0 << STM_USART_CR1_M) |
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
			  (0 << STM_USART_CR1_RWU) |
			  (0 << STM_USART_CR1_SBK));

	usart->reg->cr2 = ((0 << STM_USART_CR2_LINEN) |
			  (STM_USART_CR2_STOP_1 << STM_USART_CR2_STOP) |
			  (0 << STM_USART_CR2_CLKEN) |
			  (0 << STM_USART_CR2_CPOL) |
			  (0 << STM_USART_CR2_CPHA) |
			  (0 << STM_USART_CR2_LBCL) |
			  (0 << STM_USART_CR2_LBDIE) |
			  (0 << STM_USART_CR2_LBDL) |
			  (0 << STM_USART_CR2_ADD));

	usart->reg->cr3 = ((0 << STM_USART_CR3_ONEBIT) |
			  (0 << STM_USART_CR3_CTSIE) |
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
		usart->reg->cr3 |= ((1 << STM_USART_CR3_CTSE) |
				    (1 << STM_USART_CR3_RTSE));

	/* Pick a 9600 baud rate */
	ao_usart_set_speed(usart, 9600);
}

#if HAS_SERIAL_HW_FLOW
static void
ao_usart_set_flow(struct ao_stm_usart *usart)
{
}
#endif

#if HAS_SERIAL_6

struct ao_stm_usart ao_stm_usart6;

void stm_usart6_isr(void) { ao_usart_isr(&ao_stm_usart6, USE_SERIAL_6_STDIN); }

char
ao_serial6_getchar(void)
{
	return ao_usart_getchar(&ao_stm_usart6);
}

void
ao_serial6_putchar(char c)
{
	ao_usart_putchar(&ao_stm_usart6, c);
}

int
_ao_serial6_pollchar(void)
{
	return _ao_usart_pollchar(&ao_stm_usart6);
}

uint8_t
_ao_serial6_sleep_for(uint16_t timeout)
{
	return _ao_usart_sleep_for(&ao_stm_usart6, timeout);
}

void
ao_serial6_set_speed(uint32_t speed)
{
	ao_usart_drain(&ao_stm_usart6);
	ao_usart_set_speed(&ao_stm_usart6, speed);
}

void
ao_serial6_drain(void)
{
	ao_usart_drain(&ao_stm_usart6);
}
#endif	/* HAS_SERIAL_6 */

#if HAS_SERIAL_SW_FLOW
static void
ao_serial_set_sw_rts_cts(struct ao_stm_usart *usart,
			 void (*isr)(void),
			 struct stm_gpio *port_rts,
			 int pin_rts,
			 struct stm_gpio *port_cts,
			 int pin_cts)
{
	/* Pull RTS low to note that there's space in the FIFO */
	ao_enable_output(port_rts, pin_rts, 0);
	usart->gpio_rts = port_rts;
	usart->pin_rts = pin_rts;
	usart->rts = 1;

	ao_exti_setup(port_cts, pin_cts, AO_EXTI_MODE_FALLING|AO_EXTI_PRIORITY_MED, isr);
	usart->gpio_cts = port_cts;
	usart->pin_cts = pin_cts;
}
#endif

void
ao_usart_init(void)
{
#if HAS_SERIAL_6
	stm_rcc.apb2enr |= (1 << STM_RCC_APB2ENR_USART6EN);

	ao_stm_usart6.reg = &stm_usart6;
	ao_stm_usart6.clk = AO_P2CLK;

	ao_enable_port(SERIAL_6_RX_PORT);
	ao_enable_port(SERIAL_6_TX_PORT);

	stm_afr_set(SERIAL_6_RX_PORT, SERIAL_6_RX_PIN, STM_AFR_AF8);
	stm_afr_set(SERIAL_6_TX_PORT, SERIAL_6_TX_PIN, STM_AFR_AF8);

	stm_nvic_set_enable(STM_ISR_USART6_POS);
	stm_nvic_set_priority(STM_ISR_USART6_POS, AO_STM_NVIC_MED_PRIORITY);

	_ao_usart_init(&ao_stm_usart6, USE_SERIAL_6_FLOW && !USE_SERIAL_6_SW_FLOW);

# if USE_SERIAL_6_FLOW
#  if USE_SERIAL_6_SW_FLOW
	ao_serial_set_sw_rts_cts(&ao_stm_usart6,
				 ao_serial6_cts,
				 SERIAL_6_PORT_RTS,
				 SERIAL_6_PIN_RTS,
				 SERIAL_6_PORT_CTS,
				 SERIAL_6_PIN_CTS);
#  else
	stm_afr_set(SERIAL_6_PORT_RTS, SERIAL_6_PIN_RTS, STM_AFR_AF8);
	stm_afr_set(SERIAL_6_PORT_CTS, SERIAL_6_PIN_CTS, STM_AFR_AF8);
#  endif
#endif

#if USE_SERIAL_6_STDIN && !DELAY_SERIAL_6_STDIN
	ao_add_stdio(_ao_serial6_pollchar,
		     ao_serial6_putchar,
		     NULL);
#endif
#endif
}
