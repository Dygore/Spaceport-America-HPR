/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
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

#ifndef _AO_ARCH_FUNCS_H_
#define _AO_ARCH_FUNCS_H_

/* task functions */

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
ao_arch_init_stack(struct ao_task *task, uint32_t *sp, void *start)
{
	uint32_t	a = (uint32_t) start;
	int		i;

	/* Return address (goes into LR) */
	ARM_PUSH32(sp, a);

	/* Clear register values r0-r12 */
	i = 13;
	while (i--)
		ARM_PUSH32(sp, 0);

	/* APSR */
	ARM_PUSH32(sp, 0);

	/* Clear register values s0-s31 */
	i = 32;
	while (i--)
		ARM_PUSH32(sp, 0);

	/* FPSCR */
	ARM_PUSH32(sp, 0);

	/* BASEPRI with interrupts enabled */
	ARM_PUSH32(sp, 0);

	task->sp32 = sp;
}

static inline void ao_arch_save_regs(void) {
	/* Save general registers */
	asm("push {r0-r12,lr}");

	/* Save APSR */
	asm("mrs r0,apsr");
	asm("push {r0}");

	/* Save FPU registers */
	asm("vpush {s0-s15}");
	asm("vpush {s16-s31}");

	/* Save FPSCR */
	asm("vmrs r0,fpscr");
	asm("push {r0}");

#ifdef AO_NONMASK_INTERRUPTS
	/* Save BASEPRI */
	asm("mrs r0,basepri");
#else
	/* Save PRIMASK */
	asm("mrs r0,primask");
#endif
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

#ifdef AO_NONMASK_INTERRUPTS
	/* Restore BASEPRI */
	asm("pop {r0}");
	asm("msr basepri,r0");
#else
	/* Restore PRIMASK */
	asm("pop {r0}");
	asm("msr primask,r0");
#endif

	/* Restore FPSCR */
	asm("pop {r0}");
	asm("vmsr fpscr,r0");

	/* Restore FPU registers */
	asm("vpop {s16-s31}");
	asm("vpop {s0-s15}");

	/* Restore APSR */
	asm("pop {r0}");
	asm("msr apsr_nczvq,r0");

	/* Restore general registers */
	asm("pop {r0-r12,lr}\n");

	/* Return to calling function */
	asm("bx lr");
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

/* GPIO functions */

#define ao_power_register(gpio)
#define ao_power_unregister(gpio)

static inline void ao_enable_port(struct stm_gpio *port)
{
	if ((port) == &stm_gpioa) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPAEN);
		ao_power_register(&ao_power_gpioa);
	} else if ((port) == &stm_gpiob) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPBEN);
		ao_power_register(&ao_power_gpiob);
	} else if ((port) == &stm_gpioc) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPCEN);
		ao_power_register(&ao_power_gpioc);
	} else if ((port) == &stm_gpiod) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPDEN);
		ao_power_register(&ao_power_gpiod);
	} else if ((port) == &stm_gpioe) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPEEN);
		ao_power_register(&ao_power_gpioe);
	} else if ((port) == &stm_gpiof) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPFEN);
		ao_power_register(&ao_power_gpiof);
	} else if ((port) == &stm_gpiog) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPGEN);
		ao_power_register(&ao_power_gpiog);
	} else if ((port) == &stm_gpioh) {
		stm_rcc_ahb1_clk_enable(1 << STM_RCC_AHB1ENR_IOPHEN);
		ao_power_register(&ao_power_gpioh);
	}
}

static inline void ao_disable_port(struct stm_gpio *port)
{
	if ((port) == &stm_gpioa) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPAEN);
		ao_power_unregister(&ao_power_gpioa);
	} else if ((port) == &stm_gpiob) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPBEN);
		ao_power_unregister(&ao_power_gpiob);
	} else if ((port) == &stm_gpioc) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPCEN);
		ao_power_unregister(&ao_power_gpioc);
	} else if ((port) == &stm_gpiod) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPDEN);
		ao_power_unregister(&ao_power_gpiod);
	} else if ((port) == &stm_gpioe) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPEEN);
		ao_power_unregister(&ao_power_gpioe);
	} else if ((port) == &stm_gpiof) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPFEN);
		ao_power_unregister(&ao_power_gpiof);
	} else if ((port) == &stm_gpiog) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPGEN);
		ao_power_unregister(&ao_power_gpiog);
	} else if ((port) == &stm_gpioh) {
		stm_rcc_ahb1_clk_disable(1 << STM_RCC_AHB1ENR_IOPHEN);
		ao_power_unregister(&ao_power_gpioh);
	}
}

#define ao_gpio_set(port, bit, v) stm_gpio_set(port, bit, v)

#define ao_gpio_get(port, bit) stm_gpio_get(port, bit)

#define ao_enable_output(port,bit,v) do {			\
		ao_enable_port(port);				\
		ao_gpio_set(port, bit, v);			\
		stm_moder_set(port, bit, STM_MODER_OUTPUT);\
	} while (0)

#define ao_gpio_set_mode(port,bit,mode) do {				\
		if (mode == AO_EXTI_MODE_PULL_UP)			\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_UP);	\
		else if (mode == AO_EXTI_MODE_PULL_DOWN)		\
			stm_pupdr_set(port, bit, STM_PUPDR_PULL_DOWN);	\
		else							\
			stm_pupdr_set(port, bit, STM_PUPDR_NONE);	\
	} while (0)

#define ao_enable_input(port,bit,mode) do {				\
		ao_enable_port(port);					\
		stm_moder_set(port, bit, STM_MODER_INPUT);		\
		ao_gpio_set_mode(port, bit, mode);			\
	} while (0)

/* usart */

void
ao_usart_init(void);

void
start(void);

char
ao_serial6_getchar(void);

void
ao_serial6_putchar(char c);

int
_ao_serial6_pollchar(void);

uint8_t
_ao_serial6_sleep_for(uint16_t timeout);

void
ao_serial6_set_speed(uint32_t speed);

void
ao_serial6_drain(void);

#endif /* _AO_ARCH_FUNCS_H_ */
