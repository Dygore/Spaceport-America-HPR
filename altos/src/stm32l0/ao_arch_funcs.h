/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_ARCH_FUNCS_H_
#define _AO_ARCH_FUNCS_H_

/* ao_spi_stm.c
 */

/* PCLK is set to 16MHz (HCLK 32MHz, APB prescaler 2) */

#define _AO_SPI_SPEED_8MHz	STM_SPI_CR1_BR_PCLK_2
#define _AO_SPI_SPEED_4MHz	STM_SPI_CR1_BR_PCLK_4
#define _AO_SPI_SPEED_2MHz	STM_SPI_CR1_BR_PCLK_8
#define _AO_SPI_SPEED_1MHz	STM_SPI_CR1_BR_PCLK_16
#define _AO_SPI_SPEED_500kHz	STM_SPI_CR1_BR_PCLK_32
#define _AO_SPI_SPEED_250kHz	STM_SPI_CR1_BR_PCLK_64
#define _AO_SPI_SPEED_125kHz	STM_SPI_CR1_BR_PCLK_128
#define _AO_SPI_SPEED_62500Hz	STM_SPI_CR1_BR_PCLK_256

/* Companion bus wants something no faster than 200kHz */

static inline uint32_t
ao_spi_speed(uint32_t hz)
{
	if (hz >= 4000000) return _AO_SPI_SPEED_4MHz;
	if (hz >= 2000000) return _AO_SPI_SPEED_2MHz;
	if (hz >= 1000000) return _AO_SPI_SPEED_1MHz;
	if (hz >=  500000) return _AO_SPI_SPEED_500kHz;
	if (hz >=  250000) return _AO_SPI_SPEED_250kHz;
	if (hz >=  125000) return _AO_SPI_SPEED_125kHz;
	return _AO_SPI_SPEED_62500Hz;
}

#define AO_SPI_CPOL_BIT		4
#define AO_SPI_CPHA_BIT		5

#define AO_SPI_CONFIG_1		0x00
#define AO_SPI_1_CONFIG_PA5_PA6_PA7	AO_SPI_CONFIG_1

#define AO_SPI_CONFIG_2		0x04
#define AO_SPI_1_CONFIG_PA12_PA13_PA14	AO_SPI_CONFIG_2
#define AO_SPI_2_CONFIG_PD1_PD3_PD4	AO_SPI_CONFIG_2

#define AO_SPI_CONFIG_3		0x08
#define AO_SPI_1_CONFIG_PB3_PB4_PB5	AO_SPI_CONFIG_3

#define AO_SPI_CONFIG_NONE	0x0c

#define AO_SPI_INDEX_MASK	0x01
#define AO_SPI_CONFIG_MASK	0x0c

#define AO_SPI_1_PA5_PA6_PA7	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PA5_PA6_PA7)
#define AO_SPI_1_PA12_PA13_PA14	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PA12_PA13_PA14)
#define AO_SPI_1_PB3_PB4_PB5	(STM_SPI_INDEX(1) | AO_SPI_1_CONFIG_PB3_PB4_PB5)

#define AO_SPI_2_PB13_PB14_PB15	(STM_SPI_INDEX(2) | AO_SPI_2_CONFIG_PB13_PB14_PB15)
#define AO_SPI_2_PD1_PD3_PD4	(STM_SPI_INDEX(2) | AO_SPI_2_CONFIG_PD1_PD3_PD4)

#define AO_SPI_INDEX(id)	((id) & AO_SPI_INDEX_MASK)
#define AO_SPI_CONFIG(id)	((id) & AO_SPI_CONFIG_MASK)
#define AO_SPI_PIN_CONFIG(id)	((id) & (AO_SPI_INDEX_MASK | AO_SPI_CONFIG_MASK))
#define AO_SPI_CPOL(id)		((uint32_t) (((id) >> AO_SPI_CPOL_BIT) & 1))
#define AO_SPI_CPHA(id)		((uint32_t) (((id) >> AO_SPI_CPHA_BIT) & 1))

#define AO_SPI_MAKE_MODE(pol,pha)	(((pol) << AO_SPI_CPOL_BIT) | ((pha) << AO_SPI_CPHA_BIT))
#define AO_SPI_MODE_0		AO_SPI_MAKE_MODE(0,0)
#define AO_SPI_MODE_1		AO_SPI_MAKE_MODE(0,1)
#define AO_SPI_MODE_2		AO_SPI_MAKE_MODE(1,0)
#define AO_SPI_MODE_3		AO_SPI_MAKE_MODE(1,1)

uint8_t
ao_spi_try_get(uint8_t spi_index, uint32_t speed, uint8_t task_id);

void
ao_spi_get(uint8_t spi_index, uint32_t speed);

void
ao_spi_put(uint8_t spi_index);

void
ao_spi_send(const void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_send_fixed(uint8_t value, uint16_t len, uint8_t spi_index);

void
ao_spi_send_sync(const void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_start_bytes(uint8_t spi_index);

void
ao_spi_stop_bytes(uint8_t spi_index);

static inline void
ao_spi_send_byte(uint8_t byte, uint8_t spi_index)
{
	struct stm_spi	*stm_spi = &stm_spi1;
	(void) spi_index;

	while (!(stm_spi->sr & (1 << STM_SPI_SR_TXE)))
		;
	stm_spi->dr = byte;
	while (!(stm_spi->sr & (1 << STM_SPI_SR_RXNE)))
		;
	(void) stm_spi->dr;
}

static inline uint8_t
ao_spi_recv_byte(uint8_t spi_index)
{
	struct stm_spi	*stm_spi = &stm_spi1;
	(void) spi_index;

	while (!(stm_spi->sr & (1 << STM_SPI_SR_TXE)))
		;
	stm_spi->dr = 0xff;
	while (!(stm_spi->sr & (1 << STM_SPI_SR_RXNE)))
		;
	return (uint8_t) stm_spi->dr;
}

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_duplex(const void *out, void *in, uint16_t len, uint8_t spi_index);

void
ao_spi_init(void);

#define ao_spi_set_cs(reg,mask) ((reg)->bsrr = ((uint32_t) (mask)) << 16)
#define ao_spi_clr_cs(reg,mask) ((reg)->bsrr = (mask))

#define ao_spi_get_mask(reg,mask,bus, speed) do {		\
		ao_spi_get(bus, speed);				\
		ao_spi_set_cs(reg,mask);			\
	} while (0)

static inline uint8_t
ao_spi_try_get_mask(struct stm_gpio *reg, uint16_t mask, uint8_t bus, uint32_t speed, uint8_t task_id)
{
	if (!ao_spi_try_get(bus, speed, task_id))
		return 0;
	ao_spi_set_cs(reg, mask);
	return 1;
}

#define ao_spi_put_mask(reg,mask,bus) do {	\
		ao_spi_clr_cs(reg,mask);	\
		ao_spi_put(bus);		\
	} while (0)

#define ao_spi_get_bit(reg,bit,bus,speed) ao_spi_get_mask(reg,(1<<bit),bus,speed)
#define ao_spi_put_bit(reg,bit,bus) ao_spi_put_mask(reg,(1<<bit),bus)

#define ao_enable_port(port) do {					\
		if ((port) == &stm_gpioa)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPAEN); \
		else if ((port) == &stm_gpiob)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPBEN); \
		else if ((port) == &stm_gpioc)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPCEN); \
		else if ((port) == &stm_gpiod)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPDEN); \
		else if ((port) == &stm_gpioe)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPEEN); \
		else if ((port) == &stm_gpioh)				\
			stm_rcc.iopenr |= (1 << STM_RCC_IOPENR_IOPHEN); \
	} while (0)

#define ao_disable_port(port) do {					\
		if ((port) == &stm_gpioa)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPAEN); \
		else if ((port) == &stm_gpiob)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPBEN); \
		else if ((port) == &stm_gpioc)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPCEN); \
		else if ((port) == &stm_gpiod)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPDEN); \
		else if ((port) == &stm_gpioe)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPEEN); \
		else if ((port) == &stm_gpioh)				\
			stm_rcc.iopenr &= ~(1 << STM_RCC_IOPENR_IOPHEN); \
	} while (0)


#define ao_gpio_set(port, bit, v) stm_gpio_set(port, bit, v)

#define ao_gpio_get(port, bit) stm_gpio_get(port, bit)

#define ao_gpio_set_bits(port, bits) stm_gpio_set_bits(port, bits)

#define ao_gpio_set_mask(port, bits, mask) stm_gpio_set_mask(port, bits, mask)

#define ao_gpio_clr_bits(port, bits) stm_gpio_clr_bits(port, bits);

#define ao_gpio_get_all(port) stm_gpio_get_all(port)

#define ao_enable_output(port,bit,v) do {			\
		ao_enable_port(port);				\
		ao_gpio_set(port, bit, v);			\
		stm_moder_set(port, bit, STM_MODER_OUTPUT);\
	} while (0)

#define ao_enable_output_mask(port,bits,mask) do {		\
		ao_enable_port(port);				\
		ao_gpio_set_mask(port, bits, mask);		\
		ao_set_output_mask(port, mask);			\
	} while (0)

#define AO_OUTPUT_PUSH_PULL	STM_OTYPER_PUSH_PULL
#define AO_OUTPUT_OPEN_DRAIN	STM_OTYPER_OPEN_DRAIN

#define ao_gpio_set_output_mode(port,bit,mode) \
	stm_otyper_set(port, pin, mode)

#define ao_gpio_set_mode(port,bit,mode) do {				\
		if (mode == AO_EXTI_MODE_PULL_UP)			\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_UP);	\
		else if (mode == AO_EXTI_MODE_PULL_DOWN)		\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_DOWN);	\
		else							\
			stm_pupdr_set(port, bit, STM_PUPDR_NONE);	\
	} while (0)

#define ao_gpio_set_mode_mask(port,mask,mode) do {			\
		if (mode == AO_EXTI_MODE_PULL_UP)			\
			stm_pupdr_set_mask(port, mask, STM_PUPDR_PULL_UP); \
		else if (mode == AO_EXTI_MODE_PULL_DOWN)		\
			stm_pupdr_set_mask(port, mask, STM_PUPDR_PULL_DOWN); \
		else							\
			stm_pupdr_set_mask(port, mask, STM_PUPDR_NONE);	\
	} while (0)

#define ao_set_input(port, bit) do {				\
		stm_moder_set(port, bit, STM_MODER_INPUT);	\
	} while (0)

#define ao_set_output(port, bit, v) do {			\
		ao_gpio_set(port, bit, v);			\
		stm_moder_set(port, bit, STM_MODER_OUTPUT);	\
	} while (0)

#define ao_set_output_mask(port, mask) do {			\
		stm_moder_set_mask(port, mask, STM_MODER_OUTPUT);	\
	} while (0)

#define ao_set_input_mask(port, mask) do {				\
		stm_moder_set_mask(port, mask, STM_MODER_INPUT);	\
	} while (0)

#define ao_enable_input(port,bit,mode) do {				\
		ao_enable_port(port);					\
		ao_set_input(port, bit);				\
		ao_gpio_set_mode(port, bit, mode);			\
	} while (0)

#define ao_enable_input_mask(port,mask,mode) do {	\
		ao_enable_port(port);			\
		ao_gpio_set_mode_mask(port, mask, mode);	\
		ao_set_input_mask(port, mask);		\
	} while (0)

#define _ao_enable_cs(port, bit) do {				\
		stm_gpio_set((port), bit, 1);			\
		stm_moder_set((port), bit, STM_MODER_OUTPUT);	\
	} while (0)

#define ao_enable_cs(port,bit) do {				\
		ao_enable_port(port);				\
		_ao_enable_cs(port, bit);			\
	} while (0)

#define ao_spi_init_cs(port, mask) do {				\
		ao_enable_port(port);				\
		if ((mask) & 0x0001) _ao_enable_cs(port, 0);	\
		if ((mask) & 0x0002) _ao_enable_cs(port, 1);	\
		if ((mask) & 0x0004) _ao_enable_cs(port, 2);	\
		if ((mask) & 0x0008) _ao_enable_cs(port, 3);	\
		if ((mask) & 0x0010) _ao_enable_cs(port, 4);	\
		if ((mask) & 0x0020) _ao_enable_cs(port, 5);	\
		if ((mask) & 0x0040) _ao_enable_cs(port, 6);	\
		if ((mask) & 0x0080) _ao_enable_cs(port, 7);	\
		if ((mask) & 0x0100) _ao_enable_cs(port, 8);	\
		if ((mask) & 0x0200) _ao_enable_cs(port, 9);	\
		if ((mask) & 0x0400) _ao_enable_cs(port, 10);\
		if ((mask) & 0x0800) _ao_enable_cs(port, 11);\
		if ((mask) & 0x1000) _ao_enable_cs(port, 12);\
		if ((mask) & 0x2000) _ao_enable_cs(port, 13);\
		if ((mask) & 0x4000) _ao_enable_cs(port, 14);\
		if ((mask) & 0x8000) _ao_enable_cs(port, 15);\
	} while (0)

/* ao_dma_stm.c
 */

extern uint8_t ao_dma_done[STM_NUM_DMA];

void
ao_dma_set_transfer(uint8_t 		index,
		    volatile void	*peripheral,
		    void		*memory,
		    uint16_t		count,
		    uint32_t		ccr);

void
ao_dma_set_isr(uint8_t index, void (*isr)(int index));

void
ao_dma_start(uint8_t index);

void
ao_dma_done_transfer(uint8_t index);

void
ao_dma_alloc(uint8_t index, uint8_t cselr);

void
ao_dma_init(void);

/* ao_i2c_stm.c */

void
ao_i2c_get(uint8_t i2c_index);

uint8_t
ao_i2c_start(uint8_t i2c_index, uint16_t address);

void
ao_i2c_put(uint8_t i2c_index);

uint8_t
ao_i2c_send(void *block, uint16_t len, uint8_t i2c_index, uint8_t stop);

uint8_t
ao_i2c_recv(void *block, uint16_t len, uint8_t i2c_index, uint8_t stop);

void
ao_i2c_init(void);

#if USE_SERIAL_1_SW_FLOW || USE_SERIAL_2_SW_FLOW || USE_SERIAL_3_SW_FLOW
#define HAS_SERIAL_SW_FLOW 1
#else
#define HAS_SERIAL_SW_FLOW 0
#endif

#if USE_SERIAL_1_FLOW && !USE_SERIAL_1_SW_FLOW || USE_SERIAL_2_FLOW && !USE_SERIAL_2_SW_FLOW || USE_SERIAL_3_FLOW && !USE_SERIAL_3_SW_FLOW
#define HAS_SERIAL_HW_FLOW 1
#else
#define HAS_SERIAL_HW_FLOW 0
#endif

/* ao_serial_stm.c */
struct ao_stm_usart {
	struct ao_fifo		rx_fifo;
	struct ao_fifo		tx_fifo;
	struct stm_usart	*reg;
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

#include <ao_lpuart.h>

void
ao_debug_out(char c);

#if HAS_SERIAL_1
extern struct ao_stm_usart	ao_stm_usart1;
#endif

#if HAS_SERIAL_2
extern struct ao_stm_usart	ao_stm_usart2;
#endif

#if HAS_SERIAL_3
extern struct ao_stm_usart	ao_stm_usart3;
#endif

#define ARM_PUSH32(stack, val)	(*(--(stack)) = (val))

typedef uint32_t	ao_arch_irq_t;

static inline void
ao_arch_block_interrupts(void) {
#ifdef AO_NONMASK_INTERRUPTS
	asm("msr basepri,%0" : : "r" (AO_STM_NVIC_BASEPRI_MASK));
#else
	asm("cpsid i");
#endif
}

static inline void
ao_arch_release_interrupts(void) {
#ifdef AO_NONMASK_INTERRUPTS
	asm("msr basepri,%0" : : "r" (0x0));
#else
	asm("cpsie i");
#endif
}

static inline uint32_t
ao_arch_irqsave(void) {
	uint32_t	val;
#ifdef AO_NONMASK_INTERRUPTS
	asm("mrs %0,basepri" : "=r" (val));
#else
	asm("mrs %0,primask" : "=r" (val));
#endif
	ao_arch_block_interrupts();
	return val;
}

static inline void
ao_arch_irqrestore(uint32_t basepri) {
#ifdef AO_NONMASK_INTERRUPTS
	asm("msr basepri,%0" : : "r" (basepri));
#else
	asm("msr primask,%0" : : "r" (basepri));
#endif
}

static inline void
ao_arch_memory_barrier(void) {
	asm volatile("" ::: "memory");
}

static inline void
ao_arch_irq_check(void) {
#ifdef AO_NONMASK_INTERRUPTS
	uint32_t	basepri;
	asm("mrs %0,basepri" : "=r" (basepri));
	if (basepri == 0)
		ao_panic(AO_PANIC_IRQ);
#else
	uint32_t	primask;
	asm("mrs %0,primask" : "=r" (primask));
	if ((primask & 1) == 0)
		ao_panic(AO_PANIC_IRQ);
#endif
}

#if HAS_TASK
static inline void
ao_arch_init_stack(struct ao_task *task, void *start)
{
	uint32_t	*sp = &task->stack32[AO_STACK_SIZE >> 2];
	uint32_t	a = (uint32_t) start;
	int		i;

	/* Return address (goes into LR) */
	ARM_PUSH32(sp, a);

	/* Clear register values r0-r7 */
	i = 8;
	while (i--)
		ARM_PUSH32(sp, 0);

	/* APSR */
	ARM_PUSH32(sp, 0);

	/* PRIMASK with interrupts enabled */
	ARM_PUSH32(sp, 0);

	task->sp32 = sp;
}

static inline void ao_arch_save_regs(void) {
	/* Save general registers */
	asm("push {r0-r7,lr}\n");

	/* Save APSR */
	asm("mrs r0,apsr");
	asm("push {r0}");

	/* Save PRIMASK */
	asm("mrs r0,primask");
	asm("push {r0}");
}

static inline void ao_arch_save_stack(void) {
	uint32_t	*sp;
	asm("mov %0,sp" : "=&r" (sp) );
	ao_cur_task->sp32 = (sp);
}

static inline void ao_arch_restore_stack(void) {
	/* Switch stacks */
	asm("mov sp, %0" : : "r" (ao_cur_task->sp32) );

	/* Restore PRIMASK */
	asm("pop {r0}");
	asm("msr primask,r0");

	/* Restore APSR */
	asm("pop {r0}");
	asm("msr apsr_nczvq,r0");

	/* Restore general registers */
	asm("pop {r0-r7,pc}\n");
}

#ifndef HAS_SAMPLE_PROFILE
#define HAS_SAMPLE_PROFILE 0
#endif

#if DEBUG
#define HAS_ARCH_VALIDATE_CUR_STACK	1

static inline void
ao_validate_cur_stack(void)
{
	uint8_t		*psp;

	asm("mrs %0,psp" : "=&r" (psp));
	if (ao_cur_task &&
	    psp <= ao_cur_task->stack &&
	    psp >= ao_cur_task->stack - 256)
		ao_panic(AO_PANIC_STACK);
}
#endif

#if !HAS_SAMPLE_PROFILE
#define HAS_ARCH_START_SCHEDULER	1

static inline void ao_arch_start_scheduler(void) {
	uint32_t	sp;
	uint32_t	control;

	asm("mrs %0,msp" : "=&r" (sp));
	asm("msr psp,%0" : : "r" (sp));
	asm("mrs %0,control" : "=r" (control));
	control |= (1 << 1);
	asm("msr control,%0" : : "r" (control));
	asm("isb");
}
#endif

#define ao_arch_isr_stack()

#endif

static inline void
ao_arch_wait_interrupt(void) {
#ifdef AO_NONMASK_INTERRUPTS
	asm(
	    "dsb\n"			/* Serialize data */
	    "isb\n"			/* Serialize instructions */
	    "cpsid i\n"			/* Block all interrupts */
	    "msr basepri,%0\n"		/* Allow all interrupts through basepri */
	    "wfi\n"			/* Wait for an interrupt */
	    "cpsie i\n"			/* Allow all interrupts */
	    "msr basepri,%1\n"		/* Block interrupts through basepri */
	    : : "r" (0), "r" (AO_STM_NVIC_BASEPRI_MASK));
#else
	asm("\twfi\n");
	ao_arch_release_interrupts();
	ao_arch_block_interrupts();
#endif
}

#define ao_arch_critical(b) do {			\
		uint32_t __mask = ao_arch_irqsave();	\
		do { b } while (0);			\
		ao_arch_irqrestore(__mask);		\
	} while (0)

void start(void);

bool
ao_storage_device_is_erased(uint32_t pos);

#endif /* _AO_ARCH_FUNCS_H_ */
