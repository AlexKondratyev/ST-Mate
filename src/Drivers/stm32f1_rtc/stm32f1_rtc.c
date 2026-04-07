#include "stm32f1_rtc.h"
#include "timestamp.h"

//	������������������ ����
void rtc_Reset(void) {
	if (RTC->PRLL != 0x7FFF) { // ���� RTC ��������� �� ��������� (��� ����� ���� ��� ������ ��������� �������)
		RCC->BDCR &= ~RCC_BDCR_RTCEN; // ����� ���������
	}
}

// ������������� ����� ��������� �������. ������ 0 ���� ��� ��, ������ !0 - �����
uint8_t rtc_Init(void) {
	uint32_t cntr = 0; // ������� ������� ��������
	rtc_Reset();
	if (!(RCC->BDCR & RCC_BDCR_RTCEN)) { // ���� ���� ��������� ������� �� ����������������
		RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN; 
		PWR->CR |= PWR_CR_DBP;                                    //	��������� ������ � Backup �������
		RCC->BDCR |= RCC_BDCR_BDRST;                              // 	�������� Backup �������
		RCC->BDCR &= ~RCC_BDCR_BDRST;
		RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;        //	������� LSE �������� (����� 32768) � ������ ������������
		RCC->BDCR |= RCC_BDCR_LSEON;                              //	�������� LSE
		while ((RCC->BDCR & RCC_BDCR_LSEON) != RCC_BDCR_LSEON){		//	��������� ���������
			if (++cntr == 144000000) return 1;
		} 	
		BKP->RTCCR |= 3;                                          //	���������� RTC
		cntr = 0;
		while (!(RTC->CRL & RTC_CRL_RTOFF)) {                			//	��������� ��������� �� ��������� ��������� RTC
			if (++cntr == 144000000) return 2;
		}
		RTC->CRL  |=  RTC_CRL_CNF;                                //	��������� ������ � �������� RTC
		RTC->PRLL  = 0x7FFF;                                      //	�������� �������� �� 32768 (32767+1)
		RTC->CRL  &=  ~RTC_CRL_CNF;                               //	��������� ������ � �������� RTC
		cntr = 0;
		while (!(RTC->CRL & RTC_CRL_RTOFF)) {                     //	��������� ��������� ������
			if (++cntr == 144000000) return 3;
		}
		RTC->CRL &= (uint16_t)~RTC_CRL_RSF;                       //	���������������� RTC
		cntr = 0;
		while((RTC->CRL & RTC_CRL_RSF) != RTC_CRL_RSF) {          //	��������� �������������
			if (++cntr == 144000000) return 4;
		}
		PWR->CR &= ~PWR_CR_DBP;                                   //	��������� ������ � Backup �������
	}
	return 0;
}

//	�������� �������� ��������
uint32_t rtc_GetCounter(void) {
	return (uint32_t)((RTC->CNTH << 16) | RTC->CNTL); 
} 

//	�������� ����� �������� ��������
void rtc_SetCounter(uint32_t count)	{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;  //	�������� ������������ PWR � Backup
	PWR->CR |= PWR_CR_DBP;                  //	��������� ������ � Backup �������
	while (!(RTC->CRL & RTC_CRL_RTOFF));    //	��������� ��������� �� ��������� ��������� RTC
	RTC->CRL |= RTC_CRL_CNF;                //	��������� ������ � �������� RTC
	RTC->CNTH = count>>16;                  //	�������� ����� �������� �������� ��������
	RTC->CNTL = count;
	RTC->CRL &= ~RTC_CRL_CNF;               //	��������� ������ � �������� RTC
	while (!(RTC->CRL & RTC_CRL_RTOFF));    //	��������� ��������� ������
	PWR->CR &= ~PWR_CR_DBP;                	//	��������� ������ � Backup �������
}

RTC_UNIT rtc_GetTime(void) {
	RTC_UNIT dt;
	TS_GetDateTime(rtc_GetCounter(), &dt.year, &dt.month, &dt.date, &dt.hours, &dt.minutes, &dt.seconds);
	return dt;
}

void rtc_SetTime(RTC_UNIT dt) {
	uint32_t temp = TS_GetSeconds(dt.year, dt.month, dt.date, dt.hours, dt.minutes, dt.seconds);
	rtc_SetCounter(temp);
}

// ��������� ������ � ��������� ���, ����� � ��� ��������� ���������� �����
void ResetStructDateTime(RTC_UNIT * unit) {
	unit->year = 255;
	unit->month = 13;
	unit->date = 32;
	unit->hours = 24;
	unit->minutes = 60;
	unit->seconds = 60;
}
