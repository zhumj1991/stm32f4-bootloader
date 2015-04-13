/*
*********************************************************************************************************
*
*	模块名称 : DAC8501 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_dac8501.c
*	版    本 : V1.0
*	说    明 : DAC8501模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口和软件SPI接口。
*			  通过宏切换。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2014-01-17  armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#define SOFT_SPI		/* 定义此行表示使用GPIO模拟SPI接口 */
//#define HARD_SPI		/* 定义此行表示使用CPU的硬件SPI接口 */

/*
	DAC8501模块可以直接插到STM32-V5开发板CN19排母(2*4P 2.54mm)接口上

    DAC8501模块    STM32F407开发板
	  VCC   ------  3.3V
	  GND   ------  GND
      SCLK  ------  PB3/SPI3_SCK
      MOSI  ------  PB5/SPI3_MOSI
      CS1   ------  PF7/NRF24L01_CSN
	  CS2   ------  PA4/NRF905_TX_EN/NRF24L01_CE/DAC1_OUT
			------  PB4/SPI3_MISO
			------  PH7/NRF24L01_IRQ

*/

/*
	DAC8501基本特性:
	1、供电2.7 - 5V;  【本例使用3.3V】
	4、参考电压2.5V (推荐缺省的，外置的）

	对SPI的时钟速度要求: 高达30MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* 软件SPI */
	/* 定义GPIO端口 */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOB
	#define PORT_SCLK	GPIOB
	#define PIN_SCLK	GPIO_Pin_3

	#define RCC_MOSI 	RCC_AHB1Periph_GPIOB
	#define PORT_MOSI	GPIOB
	#define PIN_MOSI		GPIO_Pin_5

	#define RCC_CS1 	RCC_AHB1Periph_GPIOF
	#define PORT_CS1	GPIOF
	#define PIN_CS1		GPIO_Pin_7
	
	/* 第2路片选 */
	#define RCC_CS2 	RCC_AHB1Periph_GPIOA
	#define PORT_CS2	GPIOA
	#define PIN_CS2		GPIO_Pin_4

	/* 定义口线置0和置1的宏 */
	#if 0 /* 库函数方式 */
		#define CS1_0()		GPIO_ResetBits(PORT_CS1, PIN_CS1)
		#define CS1_1()		GPIO_SetBits(PORT_CS1, PIN_CS1)

		#define SCLK_0()	GPIO_ResetBits(PORT_SCLK, PIN_SCLK)
		#define SCLK_1()	GPIO_SetBits(PORT_SCLK, PIN_SCLK)

		#define MOSI_0()	GPIO_ResetBits(PORT_MOSI, PIN_MOSI)
		#define MOSI_1()	GPIO_SetBits(PORT_MOSI, PIN_MOSI)

		#define CS2_0()		GPIO_ResetBits(PORT_CS2, PIN_CS2)
		#define CS2_1()		GPIO_SetBits(PORT_CS2, PIN_CS2)
	#else	/* 直接操作寄存器，提高速度 */
		#define CS1_0()		PORT_CS1->BSRRH = PIN_CS1 
		#define CS1_1()		PORT_CS1->BSRRL = PIN_CS1

		#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
		#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

		#define MOSI_0()	PORT_MOSI->BSRRH = PIN_MOSI
		#define MOSI_1()	PORT_MOSI->BSRRL = PIN_MOSI

		#define CS2_0()		PORT_CS2->BSRRH = PIN_CS2
		#define CS2_1()		PORT_CS2->BSRRL = PIN_CS2
	#endif
#endif

#ifdef HARD_SPI		/* 硬件SPI (未做) */
	;
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDAC8501
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 ADS1256
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC8501(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	SCLK_0();		/* SPI总线空闲时，钟线是低电平 */
	CS1_1();		/* 先不选中, SYNC相当于SPI设备的片选 */
	CS2_1();

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_MOSI | RCC_CS1 | RCC_CS2, ENABLE);

	/* 配置几个推完输出IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = PIN_SCLK;
	GPIO_Init(PORT_SCLK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_MOSI;
	GPIO_Init(PORT_MOSI, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CS1;
	GPIO_Init(PORT_CS1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CS2;
	GPIO_Init(PORT_CS2, &GPIO_InitStructure);
#endif

	DAC8501_SendData(0, 0);
	DAC8501_SendData(1, 0);
}

/*
*********************************************************************************************************
*	函 数 名: DAC8501_SendData
*	功能说明: 向SPI总线发送24个bit数据。
*	形    参: _ch, 通道, 
*		     _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8501_SendData(uint8_t _ch, uint16_t _dac)
{
	uint8_t i;
	uint32_t data;

	/*
		DAC8501.pdf page 12 有24bit定义

		DB24:18 = xxxxx 保留
		DB17： PD1
		DB16： PD0

		DB15：0  16位数据

		其中 PD1 PD0 决定4种工作模式
		      0   0  ---> 正常工作模式
		      0   1  ---> 输出接1K欧到GND
		      1   0  ---> 输出100K欧到GND
		      1   1  ---> 输出高阻
	*/

	data = _dac; /* PD1 PD0 = 00 正常模式 */

	if (_ch == 0)
	{
		CS1_0();
	}
	else
	{
		CS2_0();
	}

	/*　DAC8501 SCLK时钟高达30M，因此可以不延迟 */
	for(i = 0; i < 24; i++)
	{
		if (data & 0x800000)
		{
			MOSI_1();
		}
		else
		{
			MOSI_0();
		}
		SCLK_1();
		data <<= 1;
		SCLK_0();
	}
	
	if (_ch == 0)
	{
		CS1_1();
	}
	else
	{
		CS2_1();
	}

}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
