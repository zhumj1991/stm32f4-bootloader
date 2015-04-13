/*
*********************************************************************************************************
*
*	模块名称 : 电阻式触摸板驱动模块
*	文件名称 : bsp_touch.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期         作者    说明
*		v1.0    2012-12-17  Eric2013  ST固件库V1.0.2版本。
*	
*   QQ超级群：216681322
*   UCOS&UCGUI论坛：http://bbs.armfly.com/thread.php?fid=12
*   BLOG: http://blog.sina.com.cn/u/2565749395
*
*********************************************************************************************************
*/

#ifndef __BSP_TOUCH_H
#define __BSP_TOUCH_H

typedef struct
{
	uint16_t XYChange;	/* X, Y 是否交换  */

	uint16_t usMaxAdc;	/* 触摸板最大ADC值，用于有效点判断. 最小ADC = 0  */
	int16_t usAdcNowX;
	int16_t usAdcNowY;

	uint8_t Enable;		/* 触摸检测使能标志 */

}TOUCH_T;

extern TOUCH_T g_tTP;

/* 触摸事件 */
enum
{
	TOUCH_NONE = 0,		/* 无触摸 */
	TOUCH_DOWN = 1,		/* 按下 */
	TOUCH_MOVE = 2,		/* 移动 */
	TOUCH_RELEASE = 3	/* 释放 */
};

/* 供外部调用的函数声明 */
void TOUCH_InitHard(void);

void TOUCH_SCAN(void);

#endif


