/*
 * Copyright © 2013 Keith Packard <keithp@keithp.com>
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

#define ao_spi_get_bit(reg,bit,bus,speed) ao_spi_get_mask(reg,(1<<bit),bus,speed)
#define ao_spi_put_bit(reg,bit,bus) ao_spi_put_mask(reg,(1<<bit),bus)

#define ao_enable_port(port) (lpc_scb.sysahbclkctrl |= (1 << LPC_SCB_SYSAHBCLKCTRL_GPIO))
#define ao_disable_port(port) (lpc_scb.sysahbclkctrl &= ~(1UL << LPC_SCB_SYSAHBCLKCTRL_GPIO))

#define lpc_all_bit(port,bit)	(((port) << 5) | (bit))

#define ao_gpio_set(port, bit, v)	(lpc_gpio.byte[lpc_all_bit(port,bit)] = (v))

#define ao_gpio_get(port, bit) 	(lpc_gpio.byte[lpc_all_bit(port,bit)])

#define PORT0_JTAG_REGS	((1 << 11) | (1 << 12) | (1 << 14))

static inline void lpc_set_gpio(int port, int bit) {
	if (port == 0 && (1 << bit) & (PORT0_JTAG_REGS)) {
		vuint32_t *_ioconf = &lpc_ioconf.pio0_0 + ((port)*24+(bit));

		*_ioconf = (*_ioconf & ~LPC_IOCONF_FUNC_MASK) | LPC_IOCONF_FUNC_PIO0_11;
	}
}

#define ao_enable_output(port,bit,v) do {			\
		ao_enable_port(port);				\
		lpc_set_gpio(port,bit);				\
		ao_gpio_set(port, bit, v);			\
		lpc_gpio.dir[port] |= (1 << bit);		\
	} while (0)

#define ao_gpio_set_mode(port,bit,mode) do {				\
		vuint32_t *_ioconf = &lpc_ioconf.pio0_0 + ((port)*24+(bit)); \
		vuint32_t _mode;					\
		if (mode == AO_EXTI_MODE_PULL_UP)			\
			_mode = LPC_IOCONF_MODE_PULL_UP << LPC_IOCONF_MODE; \
		else if (mode == AO_EXTI_MODE_PULL_DOWN)		\
			_mode = LPC_IOCONF_MODE_PULL_UP << LPC_IOCONF_MODE; \
		else							\
			_mode = LPC_IOCONF_MODE_INACTIVE << LPC_IOCONF_MODE; \
		*_ioconf = ((*_ioconf & ~(LPC_IOCONF_MODE_MASK << LPC_IOCONF_MODE)) | \
			    _mode |					\
			    (1 << LPC_IOCONF_ADMODE));			\
	} while (0)

#define ao_enable_input(port,bit,mode) do {				\
		ao_enable_port(port);					\
		lpc_set_gpio(port,bit);					\
		lpc_gpio.dir[port] &= ~(1UL << bit);			\
		ao_gpio_set_mode(port,bit,mode);			\
	} while (0)

#define lpc_token_paster_2(x,y)		x ## y
#define lpc_token_evaluator_2(x,y)	lpc_token_paster_2(x,y)
#define lpc_token_paster_3(x,y,z)	x ## y ## z
#define lpc_token_evaluator_3(x,y,z)	lpc_token_paster_3(x,y,z)
#define lpc_token_paster_4(w,x,y,z)	w ## x ## y ## z
#define lpc_token_evaluator_4(w,x,y,z)	lpc_token_paster_4(w,x,y,z)
#define analog_reg(port,bit)		lpc_token_evaluator_4(pio,port,_,bit)
#define analog_func(id)			lpc_token_evaluator_2(LPC_IOCONF_FUNC_AD,id)

#define ao_enable_analog(port,bit,id) do {				\
		ao_enable_port(port);					\
		lpc_gpio.dir[port] &= ~(1UL << bit);			\
		lpc_ioconf.analog_reg(port,bit) = ((analog_func(id) << LPC_IOCONF_FUNC) | \
						   (0 << LPC_IOCONF_ADMODE)); \
	} while (0)
	
#define ARM_PUSH32(stack, val)	(*(--(stack)) = (val))

static inline uint32_t
ao_arch_irqsave(void) {
	uint32_t	primask;
	asm("mrs %0,primask" : "=&r" (primask));
	ao_arch_block_interrupts();
	return primask;
}

static inline void
ao_arch_irqrestore(uint32_t primask) {
	asm("msr primask,%0" : : "r" (primask));
}

static inline void
ao_arch_memory_barrier(void) {
	asm volatile("" ::: "memory");
}

#if HAS_TASK
static inline void
ao_arch_init_stack(struct ao_task *task, uint32_t *sp, void *start)
{
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
	if (sp < &ao_cur_task->stack32[0])
		ao_panic (AO_PANIC_STACK);
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

	/* Restore general registers and return */
	asm("pop {r0-r7,pc}\n");
}

#define ao_arch_isr_stack()

#endif /* HAS_TASK */

#define ao_arch_wait_interrupt() do {				\
		asm("\twfi\n");					\
		ao_arch_release_interrupts();			\
		asm(".global ao_idle_loc\n\nao_idle_loc:");	\
		ao_arch_block_interrupts();			\
	} while (0)

#define ao_arch_critical(b) do {			\
		uint32_t __mask = ao_arch_irqsave();	\
		do { b } while (0);			\
		ao_arch_irqrestore(__mask);		\
	} while (0)

/*
 * SPI
 */

#define ao_spi_set_cs(port,mask) (lpc_gpio.clr[port] = (mask))
#define ao_spi_clr_cs(port,mask) (lpc_gpio.set[port] = (mask))

#define ao_spi_get_mask(port,mask,bus,speed) do {	\
		ao_spi_get(bus, speed);			\
		ao_spi_set_cs(port, mask);		\
	} while (0)

#define ao_spi_put_mask(reg,mask,bus) do {	\
		ao_spi_clr_cs(reg,mask);	\
		ao_spi_put(bus);		\
	} while (0)

#define ao_spi_get_bit(reg,bit,bus,speed) ao_spi_get_mask(reg,(1<<bit),bus,speed)
#define ao_spi_put_bit(reg,bit,bus) ao_spi_put_mask(reg,(1<<bit),bus)

void
ao_spi_get(uint8_t spi_index, uint32_t speed);

void
ao_spi_put(uint8_t spi_index);

void
ao_spi_send(const void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_send_fixed(uint8_t value, uint16_t len, uint8_t spi_index);

void
ao_spi_recv(void *block, uint16_t len, uint8_t spi_index);

void
ao_spi_duplex(const void *out, void *in, uint16_t len, uint8_t spi_index);

void
ao_spi_init(void);

static inline void
ao_spi_send_sync(const void *block, uint16_t len, uint8_t spi_index)
{
	ao_spi_send(block, len, spi_index);
}

static inline void ao_spi_send_byte(uint8_t byte, uint8_t spi_index)
{
	struct lpc_ssp	*lpc_ssp;
	switch (spi_index) {
	case 0:
		lpc_ssp = &lpc_ssp0;
		break;
	case 1:
		lpc_ssp = &lpc_ssp1;
		break;
	}
	lpc_ssp->dr = byte;
	while ((lpc_ssp->sr & (1 << LPC_SSP_SR_RNE)) == 0);
	(void) lpc_ssp->dr;
}

#define ao_spi_init_cs(port, mask) do {					\
		uint8_t __bit__;					\
		for (__bit__ = 0; __bit__ < 32; __bit__++) {		\
			if (mask & (1 << __bit__))			\
				ao_enable_output(port, __bit__, 1); \
		}							\
	} while (0)

void
ao_debug_out(char c);

#define HAS_ARCH_START_SCHEDULER	1

static inline void ao_arch_start_scheduler(void) {
	uint32_t	sp;
	uint32_t	control;

	asm("mrs %0,msp" : "=&r" (sp));
	asm("msr psp,%0" : : "r" (sp));
	asm("mrs %0,control" : "=&r" (control));
	control |= (1 << 1);
	asm("msr control,%0" : : "r" (control));
	asm("isb");
}

void start(void);

#endif /* _AO_ARCH_FUNCS_H_ */
