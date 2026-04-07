/* 	Работа со штампом времени 
		(c) ipaSoft Electronics 2023-2024
*/

#include "timestamp.h"

static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Получить штамп времени в секундах с 0-го по 136-й года.
uint32_t TS_GetSeconds(uint8_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds) {
	uint8_t i;
	uint32_t result = 0;
	
	for (i = 0; i < year; i++) { // Добавлять секунды по прошедшим годам
		if ((i % 4) == 0) result += 366 * 24 * 60 * 60; // Високосный
		else result += 365 * 24 * 60 * 60; // Невисокосный
	}
	for (i = 1; i < month; i++) { // Добавлять секунды по прошедшим месяцам указанного года
		if (((year % 4) == 0) && (i == 2)) result += 29 * 24 * 60 * 60; // Високосный год февраль
		else result += (uint32_t)daysInMonth[i - 1] * 24 * 60 * 60; // Другие года и/или месяцы
	}
	result += (uint32_t)(day - 1) * 24 * 60 * 60; // Добавить секунды по прошедшим дням
	result += (uint32_t)hours * 60 * 60;	// Добавить секунды по прошедшим часам
	result += (uint32_t)minutes * 60;			// Добавить секунды по прошедшим минутам
	result += seconds;
	return result;
}

// Разбить секунды штампа времени на дату и время
void TS_GetDateTime(uint32_t stamp, uint8_t *year, uint8_t *month, uint8_t *day, uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
	uint32_t tempStamp = stamp;
	*year = 0; *month = 1; *day = 1; *hours = 0; *minutes = 0; *seconds = 0;
	while(1) { // Разбить секунды на года
		if ((*year % 4) == 0) { // Если висоскосный год обрабатывается
			if (tempStamp > 366 * 24 * 60 * 60) { // Год тут есть точно
				tempStamp -= 366 * 24 * 60 * 60;
				*year = *year + 1; // Прирастить год и убавить из штампа кол-во секунд в этом году
			}
			else break; // Года не наберется
		}
		else { // Если обрабатывается невисокосный год
			if (tempStamp > 365 * 24 * 60 * 60) { // Год тут есть точно
				tempStamp -= 365 * 24 * 60 * 60;
				*year = *year + 1; // Прирастить год и убавить из штампа кол-во секунд в этом году
			}
			else break; // Года не наберется
		}
	}
	while(1) { // Разбить секунды на месяцы
		if (((*year % 4) == 0) && *month == 2) { // Если февраль високосного года
			if (tempStamp > 29 * 24 * 60 * 60) { // Месяц тут точно есть
				tempStamp -= 29 * 24 * 60 * 60;
				*month = *month + 1;
			}
			else break; // Нет месяца
		}
		else { // Другие месяцы високосных и невисокосных лет
			if (tempStamp > (uint32_t)daysInMonth[*month - 1] * 24 * 60 * 60) { // Есть месяц
				tempStamp -= (uint32_t)daysInMonth[*month - 1] * 24 * 60 * 60;
				*month = *month + 1;
			}
			else break; // Не будет месяца
		}
	}
	// Разбить секунды на дни
	while(1) {
		if (tempStamp > 24 * 60 * 60) { // секунд хватает на день
			*day = *day + 1;
			tempStamp -= 24 * 60 * 60;
		}
		else break; // Секунд меньше чем в сутках
	}
	// Разбить секунды на часы
	while(1) {
		if (tempStamp > 60 * 60) {
			*hours = *hours + 1;
			tempStamp -= 60 * 60;
		}
		else break;
	}
	// Разбить секунды на минуты
	while(1) {
		if (tempStamp > 60) {
			tempStamp -= 60;
			*minutes = *minutes + 1;
		}
		else break;
	}
	// Оставшиеся секунды
	*seconds = tempStamp -1;
}

// Вычисляет день недели любой даты начиная с 2023. Дни 1 - 31, месяцы 1 - 12, год 23 - 99
// Вернёт значение с 0 по 6
uint8_t TS_calcDayOfWeek(uint8_t day, uint8_t month, uint8_t year) {
	uint16_t firstDayOfWeekInYear = 6; // Первый день недели в текущем году. 0 - понедельник, 1 - втор..
	uint8_t i;
	uint8_t leapYear = 0; // Признак високосного года
	// Вычислить первый день недели в указанном году.
	// 2023 году 1 января - воскресенье, день 6
	for (i = 23; i < year; i++) {
		firstDayOfWeekInYear++;
		if (((i - 1) % 4) == 0) { // Если предыдущий год был високосным то
				firstDayOfWeekInYear++;
		}
	}
	if (((year - 1) % 4) == 0) { // Если предыдущий год был високосным то
		firstDayOfWeekInYear++;
	}
	firstDayOfWeekInYear = firstDayOfWeekInYear % 7;
	// Понять, високосный ли год текущий
	if ((year % 4) == 0) leapYear = 1;
	// Сложить дни прошедших месяцев
	for (i = 0; i < (month - 1); i++) {
		firstDayOfWeekInYear += daysInMonth[i];
		if (i == 1) if (leapYear) firstDayOfWeekInYear++; // Если февраль високосного года
	}
	// Прибавить искомый день
	firstDayOfWeekInYear += (day - 1);
	firstDayOfWeekInYear = firstDayOfWeekInYear % 7;
	return (uint8_t)(firstDayOfWeekInYear);
}

// Вернет количество дней в месяце в зависисмости от номера месяца (1-12) и года (0-99)
uint8_t TS_GetDaysInMonth(uint8_t month, uint8_t year) {
	if ((!month) || (month > 12)) return 0;
	uint8_t tempDaysInMonth;
	tempDaysInMonth = daysInMonth[month-1];
	if ((month == 2) && (!(year % 4))) tempDaysInMonth++;
	return tempDaysInMonth;
}