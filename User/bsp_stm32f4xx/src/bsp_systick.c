#include "bsp.h"

static __IO u32 TimingDelay;


void bsp_systick_init(void)
{
	/* SystemFrequency / 1000    1ms中断一次
	 * SystemFrequency / 100000	 10us中断一次
	 * SystemFrequency / 1000000 1us中断一次
	 */
	if (SysTick_Config(SystemCoreClock / 1000000))	// ST3.5.0库版本
	{ 
		/* Capture error */ 
		while (1);
	}
	
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

void udelay(__IO u32 nTime)
{ 
#if 0
	TimingDelay = nTime;	

	// 使能滴答定时器  
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;

	while(TimingDelay != 0);
#else
	uint8_t i;
	
	while(nTime--)
		for(i = 32; i >0; i--);
	
	
#endif
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
void SysTick_Handler(void)
{
	if (TimingDelay != 0x00)
	{ 
	TimingDelay--;
	}	
}
