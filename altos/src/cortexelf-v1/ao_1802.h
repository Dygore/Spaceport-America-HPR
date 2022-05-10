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

#ifndef _AO_1802_H_
#define _AO_1802_H_

/* Decoded address driven by TPA/TPB signals */
extern uint16_t		ADDRESS;

/* Decoded data, driven by TPB signal */
extern uint8_t		DATA;

uint8_t
MRD(void);

void
MRD_set(uint8_t value);

uint8_t
MWR(void);

void
MWR_set(uint8_t value);

uint8_t
TPA(void);

void
TPA_set(uint8_t tpa);

uint8_t
TPB(void);

void
TPB_set(uint8_t tpb);

uint8_t
MA(void);

void
MA_set(uint8_t ma);

/* Tri-state data bus */

uint8_t
BUS(void);

void
BUS_set(uint8_t bus);

void
BUS_stm(void);

void
BUS_1802(void);

/* Pins controlled by 1802 */
uint8_t
SC(void);

uint8_t
Q(void);

uint8_t
N(void);

/* Pins controlled by STM */
uint8_t
EF(void);

void
EF_set(uint8_t ef);

uint8_t
DMA_IN(void);

void
DMA_IN_set(uint8_t dma_in);

uint8_t
DMA_OUT(void);

void
DMA_OUT_set(uint8_t dma_out);

uint8_t
INT(void);

void
INT_set(uint8_t dma_out);

uint8_t
CLEAR(void);

void
CLEAR_set(uint8_t dma_out);

uint8_t
WAIT(void);

void
WAIT_set(uint8_t dma_out);

#define SC_FETCH	0
#define SC_EXECUTE	1
#define SC_DMA		2
#define SC_INTERRUPT	3

void
MUX_1802(void);

void
MUX_stm(void);

void
ao_1802_init(void);

#endif /* _AO_1802_H_ */
