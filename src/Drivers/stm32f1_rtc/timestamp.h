#ifndef TIMESTAMP_H_ 
#define TIMESTAMP_H_

#include "main.h"

// Разбить секунды штампа времени на дату и время
void TS_GetDateTime(uint32_t stamp, uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
// Получить штамп времени в секундах с 0-го по 136-й года.
uint32_t TS_GetSeconds(uint8_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds);
// Вычисляет день недели любой даты начиная с 2023. Дни 1 - 31, месяцы 1 - 12, год 23 - 99
// Вернёт значение с 0 по 6
uint8_t TS_calcDayOfWeek(uint8_t day, uint8_t month, uint8_t year);
// Вернет количество дней в месяце в зависисмости от номера месяца (1-12) и года (0-99)
uint8_t TS_GetDaysInMonth(uint8_t month, uint8_t year);

#endif