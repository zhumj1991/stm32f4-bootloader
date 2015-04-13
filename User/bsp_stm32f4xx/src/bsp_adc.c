#include "stm32f4xx.h"


/* define ---------------------------------------------------------------------*/
#define ADC1_DR_ADDRESS          ((uint32_t)0x4001204C)

/* 变量 ----------------------------------------------------------------------*/
#ifndef	SAMPDEPTH
	#define	SAMPDEPTH			256
#endif
extern	uint16_t	ADCConvertedValue[SAMPDEPTH];

/*
*********************************************************************************************************
*	函 数 名: bsp_InitADC
*	功能说明: ADC初始化
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitADC(void)
{
  ADC_InitTypeDef					ADC_InitStructure;
	ADC_CommonInitTypeDef		ADC_CommonInitStructure;
	DMA_InitTypeDef					DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef				GPIO_InitStructure;
	
	/* 使能外设时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* DMA2 Stream0 channel0 配置-------------------------------------------------- */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADCConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = SAMPDEPTH;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	
	/* DMA2_Stream0 使能 */
	DMA_Cmd(DMA2_Stream0, ENABLE);
	
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);				//使能DMA2 Stream0中断
	/* NVIC configuration*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
    
	/****************************************************************************   
	  PPCLK2 = HCLK / 2 
	  下面选择的是2分频
	  ADCCLK = PCLK2 /8 = HCLK / 8 = 168 / 8 = 21M
    ADC采样频率： Sampling Time + Conversion Time = 480 + 12 cycles = 492cyc
                  Conversion Time = 21MHz / 492cyc = 42.6ksps. 
	*****************************************************************************/
    
	/* ADC Common 配置 ----------------------------------------------------------*/
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 规则通道配置 ---------------------------------------------------------*/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 规则通道16配置 -----------------------------------------------------------*/
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 2, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles);  
	
	//ADC_ExternalTrigInjectedConvConfig(ADC1,ADC_ExternalTrigInjecConv_T2_TRGO);						//打开ADC1外部触发
	/* ADC1 注入通道10配置 -----------------------------------------------------------*/
	//ADC_InjectedChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles); 
	/* 使能温度检测 */
	//ADC_TempSensorVrefintCmd(ENABLE); 
    
	/* 使能 ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
    
	/* 完成最后一个ADC传输后使能DMA请求(单ADC模式) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	
	/* 使能 ADC1 */
	ADC_Cmd(ADC1, ENABLE);
    
	/* 软件启动ADC1转换 */
	//ADC_SoftwareStartConv(ADC1);
}

/*
*************************************************************************************************
  VSENSE：温度传感器的当前输出电压，与ADC_DR 寄存器中的结果ADC_ConvertedValue之间的转换关系为： 
            ADC_ConvertedValue * Vdd
  VSENSE = --------------------------
            Vdd_convert_value(0xFFF)  
    ADC转换结束以后，读取ADC_DR寄存器中的结果，转换温度值计算公式如下：
          V25 - VSENSE
  T(℃) = ------------  + 25
           Avg_Slope
  V25：  温度传感器在25℃时 的输出电压，典型值0.76 V。
  Avg_Slope：温度传感器输出电压和温度的关联参数，典型值2.5 mV/℃。
************************************************************************************************
*/
float GetTemp(uint16_t advalue)
{
    float Vtemp_sensor;
    float Current_Temp;
    
    Vtemp_sensor = advalue * 3.3/ 4095;  				           
    Current_Temp = (Vtemp_sensor - 0.76)/0.0025 + 25;  
    
    return Current_Temp;
}

/*-----------------------------------------
 函数说明:ADC传输DMA2通道0中断服务程序
 		  DMA把AD值传输到缓冲区完成后关闭定
		  时器3(作为触发AD转换的定时器)同时
		  置更新完成标志位为1,开定时器3在应
		  用中开启. 
------------------------------------------*/
void DMA2_Stream0_IRQHandler(void)
{ 
      DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0);	//清除DMA传输完成中断
      TIM_Cmd(TIM2,DISABLE);		//关闭TIM3
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
