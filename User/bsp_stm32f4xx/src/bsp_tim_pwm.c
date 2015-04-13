/*
	¿ÉÒÔÊä³öµ½GPIOµÄTIMÍ¨µÀ:

	TIM1_CH1, PA8,	PE9,
	TIM1_CH2, PA9,	PE11
	TIM1_CH3, PA10,	PE13
	TIM1_CH4, PA11,	PE14

	TIM2_CH1,
	TIM2_CH2, PA1,	PB3
	TIM2_CH3, PA2,	PB10
	TIM2_CH4, PA3,	PB11

	TIM3_CH1, PA6,  PB4, PC6
	TIM3_CH2, PA7,	PB5, PC7
	TIM3_CH3, PB0,	PC8
	TIM3_CH4, PB1,	PC9

	TIM4_CH1, PB6,  PD12
	TIM4_CH2, PB7,	PD13
	TIM4_CH3, PB8,	PD14
	TIM4_CH4, PB9,	PD15

	TIM5_CH1, PA0,  PH10
	TIM5_CH2, PA1,	PH11
	TIM5_CH3, PA2,	PH12
	TIM5_CH4, PA3,	PI10

	TIM8_CH1, PC6,  PI5
	TIM8_CH2, PC7,	PI6
	TIM8_CH3, PC8,	PI7
	TIM8_CH4, PC9,	PI2

	TIM9_CH1, PA2,  PE5
	TIM9_CH2, PA3,	PE6

	TIM10_CH1, PB8,  PF6

	TIM11_CH1, PB9,  PF7

	TIM12_CH1, PB14,  PH6
	TIM12_CH2, PB15,  PH9

	TIM13_CH1, PA6,  PF8
	TIM14_CH1, PA7,  PF9

	APB1 ¶¨Ê±Æ÷ÓÐ TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
	APB2 ¶¨Ê±Æ÷ÓÐ TIM1, TIM8 ,TIM9, TIM10, TIM11
*/
#include "bsp.h"

uint32_t Freq_Value[4];
/*
*********************************************************************************************************
*	º¯ Êý Ãû: bsp_SetTIMOutPWM
*	¹¦ÄÜËµÃ÷: ÅäÖÃGPIOºÍTIMÊ±ÖÓ£¬ GPIOÁ¬½Óµ½TIMÊä³öÍ¨µÀ
*	ÐÎ    ²Î: GPIOx
*			 GPIO_PinX
*			 TIMx
*			 _ucChannel
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
//NVIC_InitTypeDef NVIC_InitStructure;
	
	uint16_t usPrescaler = 88;
	uint16_t usPeriod = 999;
	/* PWMÐÅºÅµçÆ½Ìø±äÖµ */
	uint16_t CCR1_Val = 500;        
	uint16_t CCR2_Val = 375;
	uint16_t CCR3_Val = 250;
	uint16_t CCR4_Val = 125;

	/* Ê¹ÄÜÊ±ÖÓ */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* Á¬½Óµ½AF¹¦ÄÜ */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);
//	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);
	
	/* ÅäÖÃGPIO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;// | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*-----------------------------------------------------------------------
		system_stm32f4xx.c ÎÄ¼þÖÐ void SetSysClock(void) º¯Êý¶ÔÊ±ÖÓµÄÅäÖÃÈçÏÂ£º

		HCLK = SYSCLK / 1     (AHB1Periph)	168MHz
		PCLK2 = HCLK / 2      (APB2Periph)	84MHz
		PCLK1 = HCLK / 4      (APB1Periph)	42MHz

		ÒòÎªAPB1 prescaler != 1, ËùÒÔ APB1ÉÏµÄTIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
		ÒòÎªAPB2 prescaler != 1, ËùÒÔ APB2ÉÏµÄTIMxCLK = PCLK2 x 2 = SystemCoreClock;

		APB1 ¶¨Ê±Æ÷ÓÐ TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
		APB2 ¶¨Ê±Æ÷ÓÐ TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	TIM_Cmd(TIM1, DISABLE);	
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);


	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
  /* PWM1 Mode configuration: Channel2 */	
//	TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
//	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
//	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
  /* PWM1 Mode configuration: Channel3 */
//	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;	
//	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
//	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	 /* PWM1 Mode configuration: Channel4 */
//	TIM_OCInitStructure.TIM_Pulse = CCR4_Val;	
//	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
//	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM1, ENABLE);

/*
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x04;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
*/
	/* TIMx enable counter */
	TIM_Cmd(TIM1, ENABLE);

	/* ÏÂÃæÕâ¾ä»°¶ÔÓÚTIM1ºÍTIM8ÊÇ±ØÐëµÄ£¬¶ÔÓÚTIM2-TIM6Ôò²»±ØÒª */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {        
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: bsp_SetTIMforTrig
*	¹¦ÄÜËµÃ÷: ÅäÖÃTIM£¬ÓÃÓÚ¼òµ¥Íâ²¿´¥·
*	ÐÎ    ²Î: TIMx : ¶¨Ê±Æ÷
*			  _ulFreq : ¶¨Ê±ÆµÂÊ £¨Hz£©¡£ 0 ±íÊ¾¹Ø±Õ¡£
*			  _PreemptionPriority : ÖÐ¶ÏÓÅÏÈ¼¶·Ö×é
*			  _SubPriority : ×ÓÓÅÏÈ¼¶
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
void bsp_SetTIMforTrig(void)
{
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
	uint16_t usPeriod = 999;
	uint16_t usPrescaler = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/*-----------------------------------------------------------------------
		system_stm32f4xx.c ÎÄ¼þÖÐ void SetSysClock(void) º¯Êý¶ÔÊ±ÖÓµÄÅäÖÃÈçÏÂ£º

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		ÒòÎªAPB1 prescaler != 1, ËùÒÔ APB1ÉÏµÄTIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
		ÒòÎªAPB2 prescaler != 1, ËùÒÔ APB2ÉÏµÄTIMxCLK = PCLK2 x 2 = SystemCoreClock;

		APB1 ¶¨Ê±Æ÷ÓÐ TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
		APB2 ¶¨Ê±Æ÷ÓÐ TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	TIM_Cmd(TIM2, DISABLE);		/* ¹Ø±Õ¶¨Ê±Êä³ö */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);//Ê¹ÓÃTIM2ÊÂ¼þ¸üÐÂ×÷ÎªADC´¥·¢
	TIM_ARRPreloadConfig(TIM2, ENABLE);	
	
	/* TIMx enable counter */
	TIM_Cmd(TIM2, ENABLE);
}
/*
*********************************************************************************************************
*	º¯ Êý Ãû: bsp_SetTIMIntCaputer
*	¹¦ÄÜËµÃ÷: ÅäÖÃTIMºÍNVIC£¬ÓÃÓÚ²¶»ñÆµÂÊ¡£ ÖÐ¶Ï·þÎñ³ÌÐòÓÉÓ¦ÓÃ³ÌÐòÊµÏÖ¡£
*	ÐÎ    ²Î: TIMx : ¶¨Ê±Æ÷
*			  _ulFreq : ¶¨Ê±ÆµÂÊ £¨Hz£©¡£ 0 ±íÊ¾¹Ø±Õ¡£
*			  _PreemptionPriority : ÖÐ¶ÏÓÅÏÈ¼¶·Ö×é
*			  _SubPriority : ×ÓÓÅÏÈ¼¶
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
void bsp_SetTIMIntCaputer(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_ICInitTypeDef					TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uint16_t usPeriod = 0xFFFF;
	uint16_t usPrescaler = 84;	
	
	/* Ê¹ÄÜÊ±ÖÓ */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/* Á¬½Óµ½AF¹¦ÄÜ */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);
	
	/* ÅäÖÃGPIO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*-----------------------------------------------------------------------
		system_stm32f4xx.c ÎÄ¼þÖÐ void SetSysClock(void) º¯Êý¶ÔÊ±ÖÓµÄÅäÖÃÈçÏÂ£º

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		ÒòÎªAPB1 prescaler != 1, ËùÒÔ APB1ÉÏµÄTIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
		ÒòÎªAPB2 prescaler != 1, ËùÒÔ APB2ÉÏµÄTIMxCLK = PCLK2 x 2 = SystemCoreClock;

		APB1 ¶¨Ê±Æ÷ÓÐ TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
		APB2 ¶¨Ê±Æ÷ÓÐ TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	TIM_Cmd(TIM3, DISABLE);		/* ¹Ø±Õ¶¨Ê±Êä³ö */
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	TIM_ARRPreloadConfig(TIM3, ENABLE);	
	
	/* TIM Interrupts enable */
	TIM_ITConfig(TIM3,TIM_IT_CC1, ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC2, ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC3, ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC4, ENABLE);
	
	/* TIMx enable counter */
	TIM_Cmd(TIM3, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM2_IRQHandler(void)
{

}

void TIM3_IRQHandler(void)
{
	static u16 this_time_CH1 = 0; 
	static u16 last_time_CH1 = 0; 
	static u8 capture_number_CH1 = 0;
	vu16 tmp16_CH1;
	
	static u16 this_time_CH2 = 0; 
	static u16 last_time_CH2 = 0; 
	static u8 capture_number_CH2 = 0;
	vu16 tmp16_CH2;
	
	static u16 this_time_CH3 = 0; 
	static u16 last_time_CH3 = 0; 
	static u8 capture_number_CH3 = 0;
	vu16 tmp16_CH3;
	
	static u16 this_time_CH4 = 0; 
	static u16 last_time_CH4 = 0; 
	static u8 capture_number_CH4 = 0;
	vu16 tmp16_CH4;

	if(TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET) 
	{         
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
		
		if(capture_number_CH1 == 0) {             
			capture_number_CH1 = 1;            
			last_time_CH1 = TIM_GetCapture1(TIM3); 
		} else if(capture_number_CH1 == 1) {           
							capture_number_CH1 = 0;            
							this_time_CH1 = TIM_GetCapture1(TIM3);         
							if(this_time_CH1 > last_time_CH1) { 
								tmp16_CH1 = (this_time_CH1 - last_time_CH1); 
							} else { 
								tmp16_CH1 = ((0xFFFF - last_time_CH1) + this_time_CH1); 
							}            
							Freq_Value[0] = 1000000L / tmp16_CH1; 
						} 
  }
	
	if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET) 
	{         
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
		
		if(capture_number_CH2 == 0) {             
			capture_number_CH2 = 1;            
			last_time_CH2 = TIM_GetCapture2(TIM3); 
		} else if(capture_number_CH2 == 1) {           
							capture_number_CH2 = 0;            
							this_time_CH2 = TIM_GetCapture2(TIM3);         
							if(this_time_CH2 > last_time_CH2) { 
								tmp16_CH2 = (this_time_CH2 - last_time_CH2); 
							} else { 
								tmp16_CH2 = ((0xFFFF - last_time_CH2) + this_time_CH2); 
							}            
							Freq_Value[1] = 1000000L / tmp16_CH2; 
						} 
  }
	
	if(TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET) 
	{         
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
		
		if(capture_number_CH3 == 0) {             
			capture_number_CH3 = 1;            
			last_time_CH3 = TIM_GetCapture3(TIM3); 
		} else if(capture_number_CH3 == 1) {           
							capture_number_CH3 = 0;            
							this_time_CH3 = TIM_GetCapture3(TIM3);         
							if(this_time_CH3 > last_time_CH3) { 
								tmp16_CH3 = (this_time_CH3 - last_time_CH3); 
							} else { 
								tmp16_CH3 = ((0xFFFF - last_time_CH3) + this_time_CH3); 
							}            
							Freq_Value[2] = 1000000L / tmp16_CH3; 
						} 
  }
	
	if(TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET) 
	{         
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
		
		if(capture_number_CH4 == 0) {             
			capture_number_CH4 = 1;            
			last_time_CH4 = TIM_GetCapture4(TIM3); 
		} else if(capture_number_CH4 == 1) {           
							capture_number_CH4 = 0;            
							this_time_CH4 = TIM_GetCapture4(TIM3);         
							if(this_time_CH4 > last_time_CH4) { 
								tmp16_CH4 = (this_time_CH4 - last_time_CH4); 
							} else { 
								tmp16_CH4 = ((0xFFFF - last_time_CH4) + this_time_CH4); 
							}            
							Freq_Value[3] = 1000000L / tmp16_CH4; 
						} 
  }
}

void bsp_tim_count_init(unsigned short period)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = period;
	TIM_TimeBaseStructure.TIM_Prescaler = 43308;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	/* Ê¹ÄÜTIM4ÖÐ¶ÏÍ¨µÀ */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	/* Ê¹ÄÜTIM4¼ÆÊý */
	TIM_Cmd(TIM4, ENABLE);
}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {        
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		iwdg_feed();
	}

}
