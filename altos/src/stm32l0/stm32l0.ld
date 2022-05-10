/* IOPORT */
stm_gpioh  = 0x50001c00;
stm_gpioe  = 0x50001000;
stm_gpiod  = 0x50000c00;
stm_gpioc  = 0x50000800;
stm_gpiob  = 0x50000400;
stm_gpioa  = 0x50000000;

/* AHB */
stm_aes    = 0x40026000;
stm_crc    = 0x40023000;
stm_flash  = 0x40022000;
stm_rcc    = 0x40021000;
stm_dma1   = 0x40020000;

/* APB2 */
stm_dbg    = 0x40015800;
stm_usart1 = 0x40013800;
stm_spi1   = 0x40013000;
stm_adc    = 0x40012400;
stm_firewall = 0x40011c00;
stm_tim22  = 0x40011400;
stm_tim21  = 0x40010800;
stm_exti   = 0x40010400;
stm_syscfg = 0x40010000;
stm_comp   = 0x40010000;

/* APB1 */
stm_lptim1 = 0x40007c00;
stm_i2c3   = 0x40007800;
stm_pwr    = 0x40007000;
stm_i2c2   = 0x40005800;
stm_i2c1   = 0x40005400;
stm_usart5 = 0x40005000;
stm_usart4 = 0x40004c00;
stm_lpuart1 = 0x40004800;
stm_usart2 = 0x40004400;
stm_spi2   = 0x40003800;
stm_iwdg   = 0x40003000;
stm_wwdg   = 0x40002c00;
stm_rtc    = 0x40002800;
stm_timer7 = 0x40001400;
stm_timer6 = 0x40001000;
stm_timer3 = 0x40000400;
stm_timer2 = 0x40000000;

/* ARM */
stm_systick = 0xe000e010;

stm_nvic   = 0xe000e100;

stm_scb    = 0xe000ed00;

stm_mpu    = 0xe000ed90;

/* ID registers */
stm_flash_size = 0x1ff8007c;
stm_device_id = 0x1ff80050;
