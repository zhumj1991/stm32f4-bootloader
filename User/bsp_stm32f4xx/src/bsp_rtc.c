#include "bsp.h"

/* 选择RTC的时钟源 */
#define RTC_CLOCK_SOURCE_LSE       /* LSE */
//#define RTC_CLOCK_SOURCE_LSI     /* LSI */ 




/*
*********************************************************************************************************
*	函 数 名: RTC_Config
*	功能说明: 用于配置时间戳功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;	
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	/* 用于设置RTC分频 */
	__IO uint32_t uwAsynchPrediv = 0;
	__IO uint32_t uwSynchPrediv = 0;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	/* 使能PWR时钟 */	
	PWR_BackupAccessCmd(ENABLE);							/* 允许访问备份寄存器 */

#if defined (RTC_CLOCK_SOURCE_LSI)					/* 选择LSI作为时钟源 */	 
	RCC_LSICmd(ENABLE);												/* Enable the LSI OSC */	
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);	/* Wait till LSI is ready */

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		/* 选择RTC时钟源 */
	RCC_RTCCLKCmd(ENABLE);										/* 使能RTC时钟 */	
	RTC_WaitForSynchro();											/* 等待RTC APB寄存器同步 */
	
#elif defined (RTC_CLOCK_SOURCE_LSE)				/* 选择LSE作为RTC时钟 */
	RCC_LSEConfig(RCC_LSE_ON);								/* 使能LSE振荡器  */	  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	/* 等待就绪 */
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		/* 选择RTC时钟源 */	
	RCC_RTCCLKCmd(ENABLE);										/* 使能RTC时钟 */
	RTC_WaitForSynchro();											/* 等待RTC APB寄存器同步 */

#else
	#error Please select the RTC Clock source inside the main.c file
#endif 

	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x9527) {
		RTC_WriteProtectionCmd(DISABLE);
    RTC_EnterInitMode();
		
		/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		uwSynchPrediv = 0xFF;
		uwAsynchPrediv = 0x7F;

//		RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);  /* 使能时间戳 */
//		RTC_TimeStampPinSelection(RTC_TamperPin_PC13);	

		/* 配置RTC数据寄存器和分频器  */
		RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
		RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	
		RTC_Init(&RTC_InitStructure);						/* RTC初始化 */

		/* 设置年月日和星期 */
		RTC_DateStructure.RTC_Year = 0x14;
		RTC_DateStructure.RTC_Month = RTC_Month_December;
		RTC_DateStructure.RTC_Date = 0x23;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

		/* 设置时分秒，以及显示格式 */
		RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
		RTC_TimeStructure.RTC_Hours   = 0x00;
		RTC_TimeStructure.RTC_Minutes = 0x00;
		RTC_TimeStructure.RTC_Seconds = 0x00; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

		RTC_ExitInitMode();
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WriteBackupRegister(RTC_BKP_DR0,0x9527);
	}
	PWR_BackupAccessCmd(DISABLE);
}


void rtc_get(struct rtc_time *tm)
{
	unsigned int have_retried = 0;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

retry_get_time:	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);/* 得到时间 */
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);/* 得到日期 */
	
	tm->tm_sec = RTC_TimeStructure.RTC_Seconds;
	tm->tm_min = RTC_TimeStructure.RTC_Minutes;
	tm->tm_hour = RTC_TimeStructure.RTC_Hours;

	/* the only way to work out wether the system was mid-update
	 * when we read it is to check the second counter, and if it
	 * is zero, then we re-try the entire read
	 */
	if (tm->tm_sec == 0 && !have_retried) {
		have_retried = 1;
		goto retry_get_time;
	}	
	
	tm->tm_wday = RTC_DateStructure.RTC_WeekDay;
	tm->tm_mday = RTC_DateStructure.RTC_Date;
	tm->tm_mon = RTC_DateStructure.RTC_Month;
	tm->tm_year = RTC_DateStructure.RTC_Year;

}

void rtc_set(struct rtc_time *tm)
{
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;

	RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
	RTC_TimeStructure.RTC_Seconds = tm->tm_sec;
	RTC_TimeStructure.RTC_Minutes = tm->tm_min;
	RTC_TimeStructure.RTC_Hours = tm->tm_hour;
	
	RTC_DateStructure.RTC_WeekDay = tm->tm_wday;
	RTC_DateStructure.RTC_Date = tm->tm_mday;
	RTC_DateStructure.RTC_Month = tm->tm_mon;
	RTC_DateStructure.RTC_Year = (tm->tm_year % 100);

	PWR_BackupAccessCmd(ENABLE);	
	RTC_WriteProtectionCmd(DISABLE);
	RTC_EnterInitMode();
	
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	
	RTC_ExitInitMode();
	RTC_WriteProtectionCmd(ENABLE);
	PWR_BackupAccessCmd(DISABLE);
}

void rtc_reset(void)
{
	return;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitRTC
*	功能说明: 初始化RTC
*	形    参：无
*	返 回 值: 无		        
*********************************************************************************************************
*/
void bsp_InitRTC(void)
{
	/* RTC 配置  */
	RTC_Config();
}

