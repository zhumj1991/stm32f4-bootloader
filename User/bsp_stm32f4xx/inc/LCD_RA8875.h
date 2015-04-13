/*
*********************************************************************************************************
*
*	模块名称 : RA8875芯片驱动模块器驱动模块
*	文件名称 : LCD_RA8875.h
*	版    本 : V1.6
*	说    明 : 头文件
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _LCD_RA8875_H
#define _LCD_RA8875_H

#include "stm32f4xx.h"
#include "bsp_tft_lcd.h"

/* 选择RA8875的接口模式, 必须和硬件匹配 */
//#define IF_SPI_EN			/* SPI接口 */
#define IF_8080_EN		/* 8080 总线接口 */


#ifdef IF_SPI_EN	/* 4-Wire SPI 界面 (需要改变RA8875屏上的2个电阻位置) */
	/*
	【1】安富莱STM32-V5开发板,STM32F407IGT6
		PD3/LCD_BUSY		--- 触摸芯片忙       （RA8875屏是RA8875芯片的忙信号)
		PF6/LCD_PWM			--- LCD背光PWM控制  （RA8875屏无需此脚，背光由RA8875控制)

		PI10/TP_NCS			--- 触摸芯片的片选		(RA8875屏无需SPI接口触摸芯片）
		PB3/SPI3_SCK		--- 触摸芯片SPI时钟		(RA8875屏无需SPI接口触摸芯片）
		PB4/SPI3_MISO		--- 触摸芯片SPI数据线MISO(RA8875屏无需SPI接口触摸芯片）
		PB5/SPI3_MOSI		--- 触摸芯片SPI数据线MOSI(RA8875屏无需SPI接口触摸芯片）

		PC3/TP_INT			--- 触摸芯片中断 （对于RA8875屏，是RA8875输出的中断)
	*/
	#define RA8875_CS_0()	GPIOI->BSRRH = GPIO_Pin_10
	#define RA8875_CS_1()	GPIOI->BSRRL = GPIO_Pin_10

	#define SPI_WRITE_DATA	0x00
	#define SPI_READ_DATA	0x40
	#define SPI_WRITE_CMD	0x80
	#define SPI_READ_STATUS	0xC0
#else		/* 8080总线 （安富莱RA8875屏缺省模式） */
	/*
		安富莱STM32-V5开发板接线方法：

		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE		--- 读控制信号，OE = Output Enable ， N 表示低有效
		PD5/FSMC_NWE		--- 写控制信号，WE = Output Enable ， N 表示低有效
		PD8/FSMC_D13
		PD9/FSMC_D14
		PD10/FSMC_D15
		PD13/FSMC_A18		--- 地址 RS
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE4/FSMC_A20		--- 和主片选一起译码
		PE5/FSMC_A21		--- 和主片选一起译码
		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7
		PE11/FSMC_D8
		PE12/FSMC_D9
		PE13/FSMC_D10
		PE14/FSMC_D11
		PE15/FSMC_D12

		PG12/FSMC_NE4		--- 主片选（TFT, OLED 和 AD7606）

		PC3/TP_INT			--- 触摸芯片中断 （对于RA8875屏，是RA8875输出的中断)

		---- 下面是 TFT LCD接口其他信号 （FSMC模式不使用）----
		PD3/LCD_BUSY		--- 触摸芯片忙       （RA8875屏是RA8875芯片的忙信号)
		PF6/LCD_PWM			--- LCD背光PWM控制  （RA8875屏无需此脚，背光由RA8875控制)

		PI10/TP_NCS			--- 触摸芯片的片选		(RA8875屏无需SPI接口触摸芯片）
		PB3/SPI3_SCK		--- 触摸芯片SPI时钟		(RA8875屏无需SPI接口触摸芯片）
		PB4/SPI3_MISO		--- 触摸芯片SPI数据线MISO(RA8875屏无需SPI接口触摸芯片）
		PB5/SPI3_MOSI		--- 触摸芯片SPI数据线MOSI(RA8875屏无需SPI接口触摸芯片）
	*/
	#define RA8875_BASE		((uint32_t)(0x6C000000 | 0x00000000))

	#define RA8875_REG		*(__IO uint16_t *)(RA8875_BASE +  (1 << (18 + 1)))	/* FSMC 16位总线模式下，FSMC_A18口线对应物理地址A19 */
	#define RA8875_RAM		*(__IO uint16_t *)(RA8875_BASE)

	#define RA8875_RAM_ADDR		RA8875_BASE
#endif

enum
{
	RA_FONT_16 = 0,		/* RA8875 字体 16点阵 */
	RA_FONT_24 = 1,		/* RA8875 字体 24点阵 */
	RA_FONT_32 = 2		/* RA8875 字体 32点阵 */
};

/* 文字放大参数 */
enum
{
	RA_SIZE_X1 = 0,		/* 原始大小 */
	RA_SIZE_X2 = 1,		/* 放大2倍 */
	RA_SIZE_X3 = 2,		/* 放大3倍 */
	RA_SIZE_X4 = 3		/* 放大4倍 */
};

/* 可供外部模块调用的函数 */
void RA8875_InitHard(void);
uint16_t RA8875_ReadID(void);
uint8_t RA8875_IsBusy(void);
void RA8875_Layer1Visable(void);
void RA8875_Layer2Visable(void);
void RA8875_DispOn(void);
void RA8875_DispOff(void);
void RA8875_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor);
void RA8875_SetCursor(uint16_t _usX, uint16_t _usY);
uint16_t RA8875_GetPixel(uint16_t _usX, uint16_t _usY);
void RA8875_SetFrontColor(uint16_t _usColor);
void RA8875_SetBackColor(uint16_t _usColor);
void RA8875_SetFont(uint8_t _ucFontType, uint8_t _ucLineSpace, uint8_t _ucCharSpace);
void RA8875_SetTextZoom(uint8_t _ucHSize, uint8_t _ucVSize);
void RA8875_DispAscii(uint16_t _usX, uint16_t _usY, char *_ptr);
void RA8875_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr);
void RA8875_ClrScr(uint16_t _usColor);
void RA8875_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void RA8875_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor);
void RA8875_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8875_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void RA8875_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8875_FillCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
void RA8875_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
void RA8875_QuitWinMode(void);
void RA8875_CtrlGPO(uint8_t _pin, uint8_t _value);
void RA8875_SetBackLight(uint8_t _bright);

void RA8875_TouchInit(void);
uint16_t RA8875_TouchReadX(void);
uint16_t RA8875_TouchReadY(void);

/* 用于UCGUI/emWin的接口函数 */
void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer);
void BTE_SetOperateCode(uint8_t _ucOperate);
void BTE_Start(void);
void BTE_Wait(void);
void RA8875_DrawVLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usY2 , uint16_t _usColor);
void RA8875_DrawHColorLine(uint16_t _usX1 , uint16_t _usY1, uint16_t _usWidth, uint16_t *_pColor);
void RA8875_DrawHLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usColor);


void RA8875_InitSPI(void);
void RA8875_Sleep(void);

/* 下面3个函数用于直接写显存，比如摄像头直接DMA到显示器. 不支持SPI接口模式 */
void RA8875_StartDirectDraw(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
uint32_t RA8875_GetDispMemAddr(void);
void RA8875_QuitDirectDraw(void);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
