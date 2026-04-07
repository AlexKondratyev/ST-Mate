/*
**		ipaSoft Electronics (c) 2024
**		https://ipasoft.info
**		�������������� RTC ��� STM32F10x
*/

#ifndef STM32F1_RTC_
#define STM32F1_RTC_

#include "main.h"

typedef struct {
	uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} RTC_UNIT;

uint8_t 	rtc_Init(void);
RTC_UNIT rtc_GetTime(void);
void rtc_SetTime(RTC_UNIT dt);
void ResetStructDateTime(RTC_UNIT * unit);

#endif
