#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f4xx.h"


void bsp_InitADC(void);
float GetTemp(uint16_t advalue);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
