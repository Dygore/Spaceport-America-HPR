stm_gpiof  = 0x48001400;
stm_gpioc  = 0x48000800;
stm_gpiob  = 0x48000400;
stm_gpioa  = 0x48000000;

stm_tsc    = 0x40024000;
stm_crc    = 0x40023000;
stm_flash  = 0x40022000;
stm_rcc    = 0x40021000;
stm_dma    = 0x40020000;
stm_dbgmcu = 0x40015800;
stm_tim17  = 0x40014800;
stm_tim16  = 0x40014400;
stm_usart1 = 0x40013800;
stm_spi1   = 0x40013000;
stm_tim1   = 0x40012c00;
stm_adc    = 0x40012400;

stm_exti   = 0x40010400;
stm_syscfg = 0x40010000;

stm_cec    = 0x40007800;

stm_pwr    = 0x40007000;
stm_crs    = 0x40006c00;

stm_bxcan  = 0x40006400;
stm_usb_sram = 0x40006000;
stm_usb_bdt = 0x40006000;
stm_usb    = 0x40005c00;

stm_i2c1   = 0x40005400;

stm_usart2 = 0x40004400;

stm_spi2   = 0x40003800;	/* docs are broken here */

stm_iwdg   = 0x40003000;
stm_wwdg   = 0x40002c00;
stm_rtc    = 0x40002800;

stm_tim14  = 0x40002000;

stm_tim3   = 0x40000400;
stm_tim2   = 0x40000000;

stm_systick = 0xe000e010;

stm_nvic   = 0xe000e100;

stm_scb    = 0xe000ed00;

stm_mpu    = 0xe000ed90;

/* calibration data in system memory */
stm_cal = 0x1ffff7b8;
stm_flash_size_04x = 0x1ffff7cc;
stm_device_id = 0x1ffff7ac;
