/*
 * FiedlDriver.cpp
 *
 *  Created on: Nov 7, 2025
 *      Author: alex
 */

#include "FieldDriver.h"

#include <memory.h>
#include "stm32f1xx_hal.h"
#include "adc.h"
#include <stdio.h>
#include <math.h>

#define COUNT_ROW 8
#define CHANNEL_NUM 4
// Resistor divider K ~ 0.47. R1 = 5.1k, R2 = 10k

#pragma pack(push, 1)
struct FieldsSettings {
	uint16_t TRIGGER_LEVEL = 25;
	float alpha = 0.125f;  // Коэффициент (чем меньше, тем сильнее сглаживание)
	int32_t calibration_value[COUNT_ROW*COUNT_ROW] = {
		2062, 1972, 2033, 1975, 1972, 1993, 2005, 2013,
		2013, 1952, 1950, 1971, 2000, 1993, 1931, 2063,
		2042, 1936, 1996, 2013, 1940, 2053, 1946, 1973,
		2020, 2040, 2035, 2047, 2035, 1947, 1986, 2004,
		1948, 1992, 1984, 2037, 1976, 1960, 2017, 1937,
		1945, 1940, 1955, 2040, 2027, 2024, 1982, 1969,
		1974, 1963, 1943, 1977, 1980, 1972, 1986, 2035,
		2063, 1960, 1957, 1958, 1987, 2043, 2027, 1929,
	};
} fieldsSetting;
#pragma pack(pop)

uint16_t value[COUNT_ROW*COUNT_ROW] = {0};
uint16_t filtered_values[COUNT_ROW*COUNT_ROW] = {0}; // Храним текущее среднее
uint8_t selector = 0;
uint16_t adcValue[CHANNEL_NUM+1] = {0};
uint16_t vRef = 0;

void setChannel(uint8_t ch)
{
	HAL_GPIO_WritePin(GPIOA, ADC_S0_Pin, (GPIO_PinState)((ch >> 0) & 1u));
	HAL_GPIO_WritePin(GPIOA, ADC_S1_Pin, (GPIO_PinState)((ch >> 1) & 1u));
	HAL_GPIO_WritePin(GPIOA, ADC_S2_Pin, (GPIO_PinState)((ch >> 2) & 1u));
	HAL_GPIO_WritePin(GPIOA, ADC_S3_Pin, (GPIO_PinState)((ch >> 3) & 1u));
}
extern "C"
{
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance==ADC1)
    {
		if (selector < 16)
		{
            for(int mux = 0; mux < CHANNEL_NUM; mux++)
            {
                int sensorIndex = mux * 16 + (15 - selector);
                value[sensorIndex] = adcValue[mux];
                vRef = adcValue[4];
            }
            selector++;
            setChannel(selector);
            HAL_ADC_Start_DMA(hadc, (uint32_t*)adcValue, CHANNEL_NUM+1);
		}
		else
		{
			HAL_ADC_Stop_DMA(hadc);
			selector = 0;
			setChannel(selector);
		}
    }
}
}

FieldDriver::FieldDriver(
		Fields& fields,
		FieldStatusInterface* interface,
		FlashDriverInterface* flash)
{
	this->fields = &fields;
	this->interface = interface;
	this->flash = flash;

	loadFieldSettings();

	HAL_ADCEx_Calibration_Start(&hadc1);
	run();
	seed = (value[3]&0x0F) << 16 | (value[2]&0x0F) << 8 | (value[1]&0x0F) << 4 | (value[0]&0x0F);

	bool isEmpty = true;
	for (int i = 0; i < COUNT_ROW*COUNT_ROW; i++)
		if (abs(value[i]-fieldsSetting.calibration_value[i]) > fieldsSetting.TRIGGER_LEVEL*5)
		{
			isEmpty = false;
			break;
		}
	if (isEmpty)
		isCalibrate = true;
}

void FieldDriver::calibration()
{
	printf("Start calibration...\n");
	uint32_t start = HAL_GetTick();
	const int CYCLE_COUNT = 2000;
	for (int cycle = 0; cycle < CYCLE_COUNT; ++cycle)
	{
		readState();
		for (int i = 0; i<64; i++)
		{
			fieldsSetting.calibration_value[i] += value[i];
		}
	}
	for (int i = 0; i<64; i++)
	{
		fieldsSetting.calibration_value[i] /= CYCLE_COUNT;
		filtered_values[i] = (float)fieldsSetting.calibration_value[i];
	}
	saveFieldSettings();

	printf("Calibration complete in: %ld ms\n", HAL_GetTick()-start);
	printf("int32_t calibration_value[COUNT_ROW*COUNT_ROW] = {\n\t");
	for (int i = 0; i < 64; i++)
	{
		printf("%ld, ",fieldsSetting.calibration_value[i]);
        if (i % 8 == 7)
        	printf("\n\t");
	}
	printf("};\n");
	isCalibrate = false;
}

FieldDriver::~FieldDriver()
{

}

void FieldDriver::readState()
{
	if (selector != 0)
		return;

	selector = 0;
	setChannel(selector);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcValue, CHANNEL_NUM+1);
	while(selector <= 15) { __NOP(); };
}

const Fields* FieldDriver::getFilds() const
{
	return fields;
}

void FieldDriver::run()
{
	if (isCalibrate)
	{
		calibration();
		return;
	}

	readState();

	for (int i = 0; i < 64; i++)
	{
		// 1. Применяем фильтр
		filtered_values[i] = (fieldsSetting.alpha * (float)value[i]) + ((1.0f - fieldsSetting.alpha) * filtered_values[i]);

	    int32_t diff = (int32_t)filtered_values[i] - fieldsSetting.calibration_value[i];
	    Fields::FieldState currentState = fields->getField(i);

	    // Пороги (можно вынести в константы)
	    int32_t threshold_set = fieldsSetting.TRIGGER_LEVEL;       // 25: порог появления фигуры
	    int32_t threshold_release = fieldsSetting.TRIGGER_LEVEL - 10; // 15: порог исчезновения

	    if (currentState == Fields::none) {
	        // Если было пусто, ждем уверенного сигнала (> 25)
	        if (abs(diff) > threshold_set) {
	            fields->setField(i, (diff > 0) ? Fields::white : Fields::black);
	        }
	    } else {
	        // Если фигура уже стоит, отпускаем только если сигнал сильно упал (< 15)
	        if (abs(diff) < threshold_release) {
	            fields->setField(i, Fields::none);
	        } else {
	            // Защита от смены полярности на лету (например, провели другим магнитом)
	            fields->setField(i, (diff > 0) ? Fields::white : Fields::black);
	        }
	    }
	}
	this->voltage = 4095/(float)vRef*1.2;

	if (interface != nullptr)
		interface->fieldStatus(*fields);
}

void FieldDriver::messege(const std::string &message)
{
	int param = 0;
	if (message.find("CALIBRATE") != std::string::npos)
	{
		isCalibrate = true;
	}
	if (message.find("TRIGLV?") != std::string::npos)
	{
		printf("Trigger level:%d\n",fieldsSetting.TRIGGER_LEVEL);
	}
	if (message.find("TRIGLV:") != std::string::npos)
	{
	    if (sscanf(message.c_str(), "TRIGLV:%d", &param) == 1)
	    {
	    	if (param >= 0)
	    	{
	    		fieldsSetting.TRIGGER_LEVEL = param;
	    		saveFieldSettings();
				printf("Trigger level set:%d\n",param);
	    	}
	    }
	}
	if (message.find("ALPHA?") != std::string::npos)
	{
		printf("Alpha:1/%d\n", (int)((float)1/fieldsSetting.alpha));
	}
	if (message.find("ALPHA:") != std::string::npos)
	{
	    if (sscanf(message.c_str(), "ALPHA:%d", &param) == 1)
	    {
	    	if (param > 0)
	    	{
	    		fieldsSetting.alpha = 1/float(param);
	    		saveFieldSettings();
				printf("Alpha:1/%d\n",param);
	    	}
	    }
	}
}

void FieldDriver::loadFieldSettings()
{
	if(flash->isInit())
	{
		flash->read(0, (uint8_t*)&fieldsSetting, sizeof(FieldsSettings));
	}
}

void FieldDriver::saveFieldSettings()
{
	if(flash->isInit())
	{
		flash->write(0, (uint8_t*)&fieldsSetting, sizeof(FieldsSettings));
	}
}

void FieldDriver::printField()
{
    for (int i = 7; i >= 0; --i)
    {
        for (int j = 7; j >= 0; --j)
        {
        	printf("%d\t\t",  fields->getField(i * 8 + j));
        }
        printf("\n");
    }
    printf("\n");
}
