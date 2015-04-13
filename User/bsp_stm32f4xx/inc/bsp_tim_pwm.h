#ifndef __BSP_TIM_PWM_H
#define __BSP_TIM_PWM_H

void bsp_SetTIMOutPWM(void);

void bsp_SetTIMforTrig(void);

void bsp_SetTIMIntCaputer(void);
	
void TIM2_IRQHandler(void);

void bsp_tim_count_init(unsigned short period);

#endif


