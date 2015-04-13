/*
*********************************************************************************************************
*
*	模块名称 : RA8875芯片和MCU接口模块
*	文件名称 : bsp_ra8875_port.c
*	版    本 : V1.6
*	说    明 : RA8875底层驱动函数集。
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#ifdef RA_SOFT_8080_8		/* 软件模拟8080总线，8 bit */
	#error "STM32_V5 dont's surport soft 8080"
#endif

#ifdef RA_SOFT_SPI			/* 软件模SPI总线 */
	#error "STM32_V5 dont's surport soft SPI"
#endif

#ifdef RA_HARD_SPI	/* 硬件 SPI 界面 (需要改变RA8875屏上的2个电阻位置) */
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

	static uint8_t SPI_ShiftByte(uint8_t _ucByte);
#endif

#ifdef RA_HARD_8080_16		/* 8080硬件总线 （安富莱RA8875屏缺省模式） */
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

/*
*********************************************************************************************************
*	函 数 名: RA8875_Delaly1us
*	功能说明: 延迟函数, 不准, 主要用于RA8875 PLL启动之前发送指令间的延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_Delaly1us(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++);	/* 延迟, 不准 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_Delaly1ms
*	功能说明: 延迟函数.  主要用于RA8875 PLL启动之前发送指令间的延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_Delaly1ms(void)
{
	uint16_t i;

	for (i = 0; i < 5000; i++);	/* 延迟, 不准 */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteCmd
*	功能说明: 写RA8875指令寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_WriteCmd(uint8_t _ucRegAddr)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_CMD);
	SPI_ShiftByte(_ucRegAddr);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16			/* 8080硬件总线 */
	RA8875_REG = _ucRegAddr;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteData
*	功能说明: 写RA8875指令寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_WriteData(uint8_t _ucRegValue)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_ucRegValue);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16			/* 8080硬件总线 */
	RA8875_RAM = _ucRegValue;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadData
*	功能说明: 读RA8875寄存器值
*	形    参: 无
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint8_t RA8875_ReadData(void)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080硬件总线 */
	uint8_t value;

	value = RA8875_RAM;		/* 读取寄存器值 */

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_WriteData16
*	功能说明: 写RA8875数据总线，16bit，用于RGB显存写入
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_WriteData16(uint16_t _usRGB)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB >> 8);
	RA8875_CS_1();

	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16		/* 8080硬件总线 */
	RA8875_RAM = _usRGB;	/* 设置寄存器地址 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadData16
*	功能说明: 读RA8875显存，16bit RGB
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t RA8875_ReadData16(void)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	value <<= 8;
	value += SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080硬件总线 */
	uint16_t value;

	value = RA8875_RAM;		/* 读取寄存器值 */

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_ReadStatus
*	功能说明: 读RA8875状态寄存器
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t RA8875_ReadStatus(void)
{
#ifdef RA_HARD_SPI	/* 硬件SPI接口 */
	uint8_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_STATUS);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080硬件总线 */
	uint8_t value;

	value = RA8875_REG;

	return value;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_GetDispMemAddr
*	功能说明: 获得显存地址。
*	形    参: 无
*	返 回 值: 显存地址
*********************************************************************************************************
*/
uint32_t RA8875_GetDispMemAddr(void)
{
	#ifdef RA_HARD_8080_16		/* 8080硬件总线 （安富莱RA8875屏缺省模式） */
		return RA8875_RAM_ADDR;
	
	#else
		return 0;
	#endif
}

#ifdef RA_HARD_SPI	   /* 四线SPI接口模式 */

/*
*********************************************************************************************************
*	函 数 名: RA8875_InitSPI
*	功能说明: 配置STM32和RA8875的SPI口线，使能硬件SPI1, 片选由软件控制
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_InitSPI(void)
{
	/*
		安富莱STM32-V5 开发板口线分配：  串行Flash型号为 W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK
		PB4/SPI3_MISO
		PB5/SPI3_MOSI
		PI10/TP_NCS			--- 触摸芯片的片选		(RA8875屏无需SPI接口触摸芯片）

		STM32硬件SPI接口 = SPI3
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 使能GPIO 时钟 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF, ENABLE);

		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* 配置片选口线为推挽输出模式 */
		RA8875_CS_1();		/* 片选置高，不选中 */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
	}
	
	RA8875_LowSpeedSPI();
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_LowSpeedSPI
*	功能说明: 配置STM32内部SPI硬件的工作模式、速度等参数，用于访问SPI接口的RA8875. 低速。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_LowSpeedSPI(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI1, ENABLE);				/* 使能SPI  */
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_HighSpeedSPI
*	功能说明: 配置STM32内部SPI硬件的工作模式、速度等参数，用于访问SPI接口的RA8875.
*			  初始化完成后，可以将SPI切换到高速模式，以提高刷屏效率。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8875_HighSpeedSPI(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI1, ENABLE);				/* 使能SPI  */
}

/*
*********************************************************************************************************
*	函 数 名: SPI_ShiftByte
*	功能说明: 向SPI总线发送一个字节，同时返回接收到的字节
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t SPI_ShiftByte(uint8_t _ucByte)
{
	uint8_t ucRxByte;

	/* 等待发送缓冲区空 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* 发送一个字节 */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* 等待数据接收完毕 */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* 读取接收到的数据 */
	ucRxByte = SPI_I2S_ReceiveData(SPI1);

	/* 返回读到的数据 */
	return ucRxByte;
}

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
