/*
 * Copyright © 2017 Keith Packard <keithp@keithp.com>
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
#include <ao_flip_bits.h>
#include <ao_1802.h>
#include <ao_exti.h>

/* Decoded address driven by TPA/TPB signals */
uint16_t	ADDRESS;

/* Decoded data, driven by TPB signal */
uint8_t		DATA;

/* Mux control */
#define _MUX_1802		0
#define _MUX_STM		1

uint8_t		MUX_CONTROL;

/* Signals muxed between 1802 and STM */
uint8_t
MRD(void) {
	return ao_gpio_get(MRD_PORT, MRD_BIT, MRD_PIN);
}

void
MRD_set(uint8_t value) {
	ao_gpio_set(MRD_PORT, MRD_BIT, MRD_PIN, value);
}

uint8_t
MWR(void) {
	return ao_gpio_get(MWR_PORT, MWR_BIT, MWR_PIN);
}

void
MWR_set(uint8_t value) {
	ao_gpio_set(MWR_PORT, MWR_BIT, MWR_PIN, value);
}

static void
TPA_rising(void)
{
	ADDRESS = (ADDRESS & 0x00ff) | ((uint16_t) MA() << 8);
	ao_wakeup(&ADDRESS);
}

uint8_t
TPA(void) {
	return ao_gpio_get(TPA_PORT, TPA_BIT, TPA_PIN);
}

void
TPA_set(uint8_t tpa) {
	ao_gpio_set(TPA_PORT, TPA_BIT, TPA_PIN, tpa);
	if (tpa)
		TPA_rising();
}

static void
TPB_rising(void)
{
	ADDRESS = (ADDRESS & 0xff00) | MA();
	if (MWR() == 0 || MRD() == 0)
		DATA = BUS();
	ao_wakeup(&ADDRESS);
}

static void
TPB_falling(void)
{
}

uint8_t
TPB(void) {
	return ao_gpio_get(TPB_PORT, TPB_BIT, TPB_PIN);
}

void
TPB_set(uint8_t tpb) {
	ao_gpio_set(TPB_PORT, TPB_BIT, TPB_PIN, tpb);
	if (tpb)
		TPB_rising();
	else
		TPB_falling();
}

uint8_t
MA(void) {
	return (ao_gpio_get_all(MA_PORT) >> MA_SHIFT) & MA_MASK;
}

void
MA_set(uint8_t ma) {
	ao_gpio_set_mask(MA_PORT, ((uint16_t) ma) << MA_SHIFT, MA_MASK << MA_SHIFT);
}

/* Tri-state data bus */

uint8_t
BUS(void) {
	return ao_flip_bits_8[(ao_gpio_get_all(BUS_PORT) >> BUS_SHIFT) & BUS_MASK];
}

void
BUS_set(uint8_t bus) {
	ao_gpio_set_mask(BUS_PORT, ao_flip_bits_8[bus] << BUS_SHIFT, BUS_MASK << BUS_SHIFT);
}

void
BUS_stm(void)
{
	ao_set_output_mask(BUS_PORT, BUS_MASK << BUS_SHIFT);
}

void
BUS_1802(void)
{
	ao_set_input_mask(BUS_PORT, BUS_MASK << BUS_SHIFT);
}

/* Pins controlled by 1802 */
uint8_t
SC(void) {
	return ao_flip_bits_2[(ao_gpio_get_all(SC_PORT) >> SC_SHIFT) & SC_MASK];
}

uint8_t
Q(void) {
	return ao_gpio_get(Q_PORT, Q_BIT, Q_PIN);
}

uint8_t
N(void) {
	return (ao_gpio_get_all(N_PORT) >> N_SHIFT) & N_MASK;
}

/* Pins controlled by STM */
uint8_t
EF(void) {
	return (ao_gpio_get_all(EF_PORT) >> EF_SHIFT) & EF_MASK;
}

void
EF_set(uint8_t ef) {
	ao_gpio_set_mask(EF_PORT, ef << EF_SHIFT, EF_MASK << EF_SHIFT);
}

uint8_t
DMA_IN(void) {
	return ao_gpio_get(DMA_IN_PORT, DMA_IN_BIT, DMA_IN_PIN);
}

void
DMA_IN_set(uint8_t dma_in) {
	ao_gpio_set(DMA_IN_PORT, DMA_IN_BIT, DMA_IN_PIN, dma_in);
}

uint8_t
DMA_OUT(void) {
	return ao_gpio_get(DMA_OUT_PORT, DMA_OUT_BIT, DMA_OUT_PIN);
}

void
DMA_OUT_set(uint8_t dma_out) {
	ao_gpio_set(DMA_OUT_PORT, DMA_OUT_BIT, DMA_OUT_PIN, dma_out);
}

uint8_t
INT(void) {
	return ao_gpio_get(INT_PORT, INT_BIT, INT_PIN);
}

void
INT_set(uint8_t dma_out) {
	ao_gpio_set(INT_PORT, INT_BIT, INT_PIN, dma_out);
}

uint8_t
CLEAR(void) {
	return ao_gpio_get(CLEAR_PORT, CLEAR_BIT, CLEAR_PIN);
}

void
CLEAR_set(uint8_t dma_out) {
	ao_gpio_set(CLEAR_PORT, CLEAR_BIT, CLEAR_PIN, dma_out);
}

uint8_t
WAIT(void) {
	return ao_gpio_get(WAIT_PORT, WAIT_BIT, WAIT_PIN);
}

void
WAIT_set(uint8_t dma_out) {
	ao_gpio_set(WAIT_PORT, WAIT_BIT, WAIT_PIN, dma_out);
}

void
tpb_isr(void) {
	/* Latch low address and data on rising edge of TPB */
	if (TPB())
		TPB_rising();
	else
		TPB_falling();
}

void
tpa_isr(void) {
	/* Latch high address on rising edge of TPA */
	if (TPA())
		TPA_rising();
}

#define ao_1802_in(port, bit, mode) do {		\
		ao_gpio_set_mode(port, bit, mode);	\
		ao_set_input(port, bit);		\
	} while (0)

#define ao_1802_in_isr(port, bit, mode) do {		\
		ao_gpio_set_mode(port, bit, mode);	\
		ao_set_input(port, bit);		\
		ao_exti_enable(port, bit);		\
	} while (0)

#define ao_1802_out_isr(port, bit) do { \
		ao_exti_disable(port, bit); \
		ao_set_output(port, bit); \
	} while (0)

void
MUX_1802(void)
{
	if (MUX_CONTROL != _MUX_1802) {
		/* Set pins to input, but pulled to idle value */
		ao_1802_in(MRD_PORT, MRD_BIT, AO_EXTI_MODE_PULL_UP);
		ao_1802_in(MWR_PORT, MWR_BIT, AO_EXTI_MODE_PULL_UP);
		ao_1802_in_isr(TPB_PORT, TPB_BIT, AO_EXTI_MODE_PULL_DOWN);
		ao_1802_in_isr(TPA_PORT, TPA_BIT, AO_EXTI_MODE_PULL_DOWN);
		ao_set_input_mask(MA_PORT, MA_MASK << MA_SHIFT);

		ao_gpio_set(MUX_PORT, MUX_BIT, MUX_PIN, 0);

		/* Now change the pins to eliminate the pull up/down */
		ao_gpio_set_mode(MRD_PORT, MRD_BIT, 0);
		ao_gpio_set_mode(MWR_PORT, MWR_BIT, 0);
		ao_gpio_set_mode(TPB_PORT, TPB_BIT, 0);
		ao_gpio_set_mode(TPA_PORT, TPA_BIT, 0);

		MUX_CONTROL = _MUX_1802;
	}
}

void
MUX_stm(void)
{
	if (MUX_CONTROL != _MUX_STM) {
		/* Set the pins back to pull to the idle value */
		ao_gpio_set_mode(MRD_PORT, MRD_BIT, AO_EXTI_MODE_PULL_UP);
		ao_gpio_set_mode(MWR_PORT, MWR_BIT, AO_EXTI_MODE_PULL_UP);
		ao_gpio_set_mode(TPB_PORT, TPB_BIT, AO_EXTI_MODE_PULL_DOWN);
		ao_gpio_set_mode(TPA_PORT, TPA_BIT, AO_EXTI_MODE_PULL_DOWN);

		ao_gpio_set(MUX_PORT, MUX_BIT, MUX_PIN, 1);

		/* Now set the pins as output, driven to the idle value */
		ao_set_output(MRD_PORT, MRD_BIT, MRD_PIN, 1);
		ao_set_output(MWR_PORT, MWR_BIT, MWR_PIN, 1);
		ao_set_output(TPB_PORT, TPB_BIT, TPB_PIN, 0);
		ao_set_output(TPA_PORT, TPA_BIT, TPA_PIN, 0);
		ao_set_output_mask(MA_PORT, MA_MASK << MA_SHIFT);
		MUX_CONTROL = _MUX_STM;
	}
}

void
ao_1802_init(void)
{
	/* Multiplexed signals*/

	/* active low signals */
	ao_enable_input(MRD_PORT, MRD_BIT, AO_EXTI_MODE_PULL_UP);
	ao_enable_input(MWR_PORT, MWR_BIT, AO_EXTI_MODE_PULL_UP);

	/* active high signals with interrupts */
	ao_exti_setup(TPA_PORT, TPA_BIT,
		      AO_EXTI_MODE_PULL_DOWN | AO_EXTI_MODE_RISING | AO_EXTI_MODE_FALLING,
		      tpa_isr);
	ao_exti_setup(TPB_PORT, TPB_BIT,
		      AO_EXTI_MODE_PULL_DOWN | AO_EXTI_MODE_RISING | AO_EXTI_MODE_FALLING,
		      tpb_isr);

	/* multiplexed address bus */
	ao_enable_input_mask(MA_PORT, MA_MASK << MA_SHIFT, 0);

	/* Data bus */

	ao_enable_input_mask(BUS_PORT, BUS_MASK << BUS_SHIFT, 0);

	/* Pins controlled by 1802 */
	ao_enable_input_mask(SC_PORT, SC_MASK << SC_SHIFT, 0);
	ao_enable_input(Q_PORT, Q_BIT, 0);
	ao_enable_input_mask(N_PORT, N_MASK << N_SHIFT, 0);

	/* Pins controlled by STM */
	ao_enable_output_mask(EF_PORT, 0, EF_MASK << EF_SHIFT);
	ao_enable_output(DMA_IN_PORT, DMA_IN_BIT, DMA_IN_PIN, 1);
	ao_enable_output(DMA_OUT_PORT, DMA_OUT_BIT, DMA_OUT_PIN, 1);
	ao_enable_output(INT_PORT, INT_BIT, INT_PIN, 1);
	ao_enable_output(CLEAR_PORT, CLEAR_BIT, CLEAR_PIN, 1);
	ao_enable_output(WAIT_PORT, WAIT_BIT, WAIT_PIN, 1);

	/* Force configuration to STM so that MUX_1802 will do something */
	MUX_CONTROL = _MUX_STM;
	MUX_1802();
}
