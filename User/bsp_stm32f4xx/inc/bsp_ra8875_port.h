/*
*********************************************************************************************************
*
*	模块名称 : RA8875芯片和MCU之间的接口驱动
*	文件名称 : bsp_ra8875_port.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _BSP_RA8875_PORT_H
#define _BSP_RA8875_PORT_H

//#define RA_SOFT_SPI	   	/* 软件SPI接口模式 */
//#define RA_HARD_SPI	   	/* 硬件SPI接口模式 */
//#define RA_SOFT_8080_8	/* 软件模拟8080接口,8bit */
#define RA_HARD_8080_16	/* 硬件8080接口,16bit */

void RA8875_Delaly1us(void);
void RA8875_Delaly1ms(void);
uint16_t RA8875_ReadID(void);
void RA8875_WriteCmd(uint8_t _ucRegAddr);
void RA8875_WriteData(uint8_t _ucRegValue);
uint8_t RA8875_ReadData(void);
void RA8875_WriteData16(uint16_t _usRGB);
uint16_t RA8875_ReadData16(void);
uint8_t RA8875_ReadStatus(void);
uint32_t RA8875_GetDispMemAddr(void);

#ifdef RA_HARD_SPI	   /* 四线SPI接口模式 */
	void RA8875_InitSPI(void);
	void RA8875_HighSpeedSPI(void);
	void RA8875_LowSpeedSPI(void);
#endif

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
