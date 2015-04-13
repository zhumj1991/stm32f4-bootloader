#ifndef _BSP_H_
#define _BSP_H

#include "stm32f4xx.h"

#include "stdio.h"
#include "string.h"

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */
//#include "bsp_uart_fifo.h"
#include "bsp_led.h"
//#include "bsp_timer.h"
//#include "bsp_key.h"
#include "bsp_uart.h"
#include "bsp_systick.h"
#include "bsp_iwdg.h"
#include "bsp_tim_pwm.h"
#include "bsp_rtc.h"
#include "bsp_spi_flash.h"
//#include "bsp_stm32_flash.h"
//#include "bsp_cpu_flash.h"
//#include "bsp_sdio_sd.h"
//#include "bsp_i2c_gpio.h"
//#include "bsp_eeprom_24xx.h"
//#include "bsp_si4730.h"
//#include "bsp_hmc5883l.h"
//#include "bsp_mpu6050.h"
//#include "bsp_bh1750.h"
//#include "bsp_bmp085.h"
//#include "bsp_wm8978.h"
//#include "bsp_fsmc_sram.h"
//#include "bsp_nand_flash.h"
//#include "bsp_nor_flash.h"
//#include "LCD_RA8875.h"
//#include "LCD_SPFD5420.h"
//#include "bsp_touch.h"
//#include "bsp_camera.h"
//#include "bsp_ad7606.h"
//#include "bsp_gps.h"
//#include "bsp_oled.h"
//#include "bsp_mg323.h"

/*
void BSP_Init(void);
void BSP_DelayUS(uint32_t _ulDelayTime);
void BSP_Tick_Init (void);
static void NVIC_Configuration(void);
*/

#endif
