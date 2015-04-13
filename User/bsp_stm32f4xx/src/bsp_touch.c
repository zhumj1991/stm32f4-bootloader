/*
*********************************************************************************************************
*
*	模块名称 : 电阻式触摸板驱动模块
*	文件名称 : bsp_touch.h
*	版    本 : V1.1
*	说    明 : 驱动TS2046芯片 和 RA8875内置触摸
*	修改记录 :
*		版本号  日期        作者    说明
*       v1.0    2012-07-06 armfly  ST固件库V3.5.0版本。
*		v1.1    2012-10-22 armfly  增加4点校准
*
*	Copyright (C), 2013-2014
*   QQ超级群：216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395
*********************************************************************************************************
*/

#include "stm32f4xx.h"
#include <stdio.h>

#include "bsp_touch.h"
#include "bsp_tft_lcd.h"
#include "LCD_RA8875.h"
#include "bsp_timer.h"

/*
【1】安富莱STM32-X2开发板 + 3.0寸显示模块， 显示模块上的触摸芯片为 TSC2046或其兼容芯片
	PA8   --> TP_CS
	PD3   --> TP_BUSY
	PA5   --> TP_SCK
	PA6   --> TP_MISO
	PA7   --> TP_MOSI
	PC7   --> TP_PEN_INT

【2】安富莱STM32开发板 + 4.3寸或7寸显示模块（内置RA8875芯片)
	RA8875内置触摸屏接口，因此直接通过FSMC访问RA8875相关寄存器即可。


	本程序未使用触笔中断功能。在1ms周期的 Systick定时中断服务程序中对触摸ADC值进行采样和
	滤波处理。当触笔按下超过40ms后，开始采集ADC值（每1ms采集1次，连续采集10次），然后对
	这10个样本进行排序，丢弃2头的样本，对中间一段样本进行求和并计算平均值。

	采样2点校准方法，校准后保存2个校准点的ADC值，实际工作时，根据2点直线方程计算屏幕坐标。
	校准参数有保存接口，本程序主要用于演示，未做保存功能。
	大家可以自己修改  TOUCH_SaveParam() 和 TOUCH_LoadParam() 两个函数实现保存功能。

*/

/* TSC2046 片选 */
#define TSC2046_CS_1()	GPIOI->BSRRL = 	GPIO_Pin_10
#define TSC2046_CS_0()	GPIOI->BSRRH = 	GPIO_Pin_10
#define TOUCH_PressValid    GPIO_ReadInputDataBit(GPIOI, GPIO_Pin_3)

/* TSC2046 内部ADC通道号 */
#define ADC_CH_X	1		/* X通道，测量X位置 */
#define ADC_CH_Y	5		/* Y通道，测量Y位置 */


/* 触屏模块用到的全局变量 */
TOUCH_T g_tTP;

static void TSC2046_InitHard(void);
uint16_t TSC2046_ReadAdc(uint8_t _ucCh);
void TOUCH_LoadParam(void);
void TOUCH_SaveParam(void);
int32_t TOUCH_Abs(int32_t x);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTouch
*	功能说明: 配置STM32和触摸相关的口线，使能硬件SPI1, 片选由软件控制
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_InitHard(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
	
	/* 配置 PC7 为浮空输入模式，用于触笔中断 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	
	if (g_ChipID == IC_8875)
	{
		RA8875_TouchInit();

		g_tTP.usMaxAdc = 1023;	/* 10位ADC */
	}
	else
	{
		TSC2046_InitHard();

		g_tTP.usMaxAdc = 4095;	/* 12位ADC */
	}

	TOUCH_LoadParam();	/* 读取校准参数 */

	g_tTP.Enable = 1;
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_DataFilter
*	功能说明: 读取一个坐标值(x或者y)
*             连续读取XPT2046_READ_TIMES次数据,对这些数据升序排列,
*             然后去掉最低和最高XPT2046_LOST_VAL个数,取平均值 
*	形    参：无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
/* 读取次数 */
#define XPT2046_READ_TIMES    5 
/* 丢弃值  */	
#define XPT2046_LOST_VAL      1	  	
uint16_t TOUCH_DataFilter(uint8_t _ucCh)
{
	uint16_t i, j;
	uint16_t buf[XPT2046_READ_TIMES];
	uint16_t usSum;
	uint16_t usTemp;

	/* 读取READ_TIMES次数据*/
	for(i=0; i < XPT2046_READ_TIMES; i++)
	{
		if (g_ChipID == IC_8875)
	    {
			if(_ucCh == ADC_CH_X)
			{
				buf[i] = RA8875_TouchReadY();
			}
			else
			{
				buf[i] = RA8875_TouchReadX();	
			}
		}
		else
		{
			buf[i] = TSC2046_ReadAdc(_ucCh);	
		}	
	}
	
	/* 升序排列 */		 		    
	for(i = 0; i < XPT2046_READ_TIMES - 1; i++)
	{
		for(j = i + 1; j < XPT2046_READ_TIMES; j++)
		{
			if(buf[i] > buf[j])
			{
				usTemp = buf[i];
				buf[i] = buf[j];
				buf[j] = usTemp;
			}
		}
	}
		  
	usSum = 0;

	/*求和 */
	for(i = XPT2046_LOST_VAL; i < XPT2046_READ_TIMES - XPT2046_LOST_VAL; i++)
	{
		usSum += buf[i];
	}
	/*求平均 */
	usTemp = usSum / (XPT2046_READ_TIMES - 2 * XPT2046_LOST_VAL);

	return usTemp; 
} 

/*
*********************************************************************************************************
*	函 数 名: TOUCH_ReadAdcXY
*	功能说明: 连续2次读取触摸屏IC,且这两次的偏差不能超过
*             ADC_ERR_RANGE,满足条件,则认为读数正确,否则读数错误.	   
*             该函数能大大提高准确度
*	形    参：x,y:读取到的坐标值
*	返 回 值: 0,失败;1,成功
*********************************************************************************************************
*/
/* 误差范围 */  
uint8_t ADC_ERR_RANGE = 5; 
uint8_t TOUCH_ReadAdcXY(int16_t *_usX, int16_t *_usY) 
{
	uint16_t iX1, iY1;
 	uint16_t iX2, iY2;
	uint16_t iX, iY;

 	iX1 = TOUCH_DataFilter(ADC_CH_X);
	iY1 = TOUCH_DataFilter(ADC_CH_Y);
	iX2 = TOUCH_DataFilter(ADC_CH_X);
	iY2 = TOUCH_DataFilter(ADC_CH_Y);
	
	iX = TOUCH_Abs(iX1 - iX2);
	iY = TOUCH_Abs(iY1 - iY2); 
	
	/* 前后两次采样在+-ERR_RANGE内 */  
    if ((iX <= ADC_ERR_RANGE) && (iY <= ADC_ERR_RANGE))
    {       
		
		if(g_ucGPIX == 1)
		{
			*_usY = (iX1 + iX2) / 2;
	        *_usX = (iY1 + iY2) / 2;		
		}
		else
		{
			*_usX = (iX1 + iX2) / 2;
	        *_usY = (iY1 + iY2) / 2;	
		}
	

        return 1;
    }
	else 
	{
		return 0;
	}	  
} 

/*
*********************************************************************************************************
*	函 数 名: TOUCH_Scan
*	功能说明: 触摸板事件检测程序。该函数被周期性调用，每ms调用1次. 见 bsp_Timer.c
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_SCAN(void)
{
	uint8_t s_invalid_count = 0;
	
	if(TOUCH_PressValid == 0)
	{			
		while(!TOUCH_ReadAdcXY(&g_tTP.usAdcNowX, &g_tTP.usAdcNowY)&&s_invalid_count < 20);
		{
			s_invalid_count++;
		}
		if(s_invalid_count >= 20)
		{
			g_tTP.usAdcNowX = -1;
			g_tTP.usAdcNowY = -1;	
		}
	}
	else
	{
		g_tTP.usAdcNowX = -1;
		g_tTP.usAdcNowY = -1;	
	}
			
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitTouch
*	功能说明: 配置STM32和触摸相关的口线，使能硬件SPI1, 片选由软件控制
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void TSC2046_InitHard(void)
{
/*
【1】安富莱STM32-X2, X4 开发板 + 3.0寸显示模块， 显示模块上的触摸芯片为 TSC2046或其兼容芯片
	PA8   --> TP_CS
	PD3   --> TP_BUSY
	PA5   --> TP_SCK
	PA6   --> TP_MISO
	PA7   --> TP_MOSI
	PC7   --> TP_PEN_INT
*/

	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/* 开启GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOI, ENABLE);

	/* 开启 SPI3 外设时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* 配置 PB3、PB4、PB5 为复用推挽输出，用于 SCK, MISO and MOSI */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	/* 配置 PI10 脚为推挽输出，用于 TP_CS  */
	TSC2046_CS_1();
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOI, &GPIO_InitStructure);

	/* 配置 PD3 引脚为上拉输入，用于 TP_BUSY */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* 配置 SPI1工作模式 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 		/* 软件控制片选 */
	/*
		SPI_BaudRatePrescaler_64 对应SCK时钟频率约1M
		TSC2046 对SCK时钟的要求，高电平和低电平最小200ns，周期400ns，也就是2.5M

		示波器实测频率
		SPI_BaudRatePrescaler_64 时，SCK时钟频率约 1.116M
		SPI_BaudRatePrescaler_32 时，SCK时钟频率月 2.232M
	*/
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* 使能 SPI1 */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: SPI_ShiftByte
*	功能说明: 向SPI总线发送一个字节，同时返回接收到的字节
*	形    参：无
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

/*
*********************************************************************************************************
*	函 数 名: SpiDelay
*	功能说明: 向SPI总线发送一个字节，同时返回接收到的字节
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void SpiDelay(uint32_t DelayCnt)
{
	uint32_t i;

	for(i=0;i<DelayCnt;i++);
}

/*
*********************************************************************************************************
*	函 数 名: TSC2046_ReadAdc
*	功能说明: 选择一个模拟通道，启动ADC，并返回ADC采样结果
*	形    参：_ucCh = 0 表示X通道； 1表示Y通道
*	返 回 值: 12位ADC值
*********************************************************************************************************
*/
uint16_t TSC2046_ReadAdc(uint8_t _ucCh)
{
	uint16_t usAdc;

	TSC2046_CS_0();		/* 使能TS2046的片选 */

	/*
		TSC2046 控制字（8Bit）
		Bit7   = S     起始位，必须是1
		Bit6:4 = A2-A0 模拟输入通道选择A2-A0; 共有6个通道。
		Bit3   = MODE  ADC位数选择，0 表示12Bit;1表示8Bit
		Bit2   = SER/DFR 模拟输入形式，  1表示单端输入；0表示差分输入
		Bit1:0 = PD1-PD0 掉电模式选择位
	*/
	SPI_ShiftByte((1 << 7) | (_ucCh << 4));			/* 选择通道1, 测量X位置 */

	/* 读ADC结果, 12位ADC值的高位先传，前12bit有效，最后4bit填0 */
	usAdc = SPI_ShiftByte(0x00);		/* 发送的0x00可以为任意值，无意义 */
	usAdc <<= 8;
	usAdc += SPI_ShiftByte(0x00);		/* 获得12位的ADC采样值 */

	usAdc >>= 3;						/* 右移3位，保留12位有效数字 */

	TSC2046_CS_1();					/* 禁能片选 */

	return (usAdc);
}



/*
*********************************************************************************************************
*	函 数 名: TOUCH_Abs
*	功能说明: 计算绝对值
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int32_t TOUCH_Abs(int32_t x)
{
	if (x >= 0)
	{
		return x;
	}
	else
	{
		return -x;
	}
}


/*
*********************************************************************************************************
*	函 数 名: TOUCH_SaveParam
*	功能说明: 保存校准参数	s_usAdcX1 s_usAdcX2 s_usAdcY1 s_usAdcX2
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_SaveParam(void)
{
#if 0
	/* 保存下面的4个变量即可 */
	uint16_t usBuf[5];

	usBuf[0] = g_tTP.usAdcX1;
	usBuf[1] = g_tTP.usAdcY1;

	usBuf[2] = g_tTP.usAdcX2;
	usBuf[3] = g_tTP.usAdcY2;

	usBuf[4] = g_tTP.XYChange;

#endif
}

/*
*********************************************************************************************************
*	函 数 名: TOUCH_LoadParam
*	功能说明: 读取校准参数
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void TOUCH_LoadParam(void)
{
#if 0
	/* 保存下面的5个变量即可 */
	uint16_t usBuf[5];

	/* ReadParamToBuf() */

	g_tTP.usAdcX1 = usBuf[0];
	g_tTP.usAdcY1 = usBuf[1];

	g_tTP.usAdcX2 = usBuf[2];
	g_tTP.usAdcY2 = usBuf[3];


	g_tTP.XYChange = usBuf[4];
#endif
}
