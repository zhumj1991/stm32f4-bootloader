#ifndef __BSP_RTC_H
#define __BSP_RTC_H

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};


void bsp_InitRTC(void);
void rtc_get(struct rtc_time *tm);
void rtc_set(struct rtc_time *tm);
void rtc_reset(void);

#endif
