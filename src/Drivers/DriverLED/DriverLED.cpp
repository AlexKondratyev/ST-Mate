/*
 * DriverLED.cpp
 *
 *  Created on: Nov 8, 2025
 *      Author: alex
 */

#include "DriverLED.h"
#include <math.h>
#include <memory.h>
#include "tim.h"

//brightness ~ current
#define BRIGHTNES(a) 53.23545*powf(a,3) - 46.50462*powf(a,2) + 128.0851*a - 9.06596

#define TIMER_FREQ 72.0

#define T0H 0.250
#define T1H 1.50
#define T0L 1.50
#define T1L 0.300

#define TRESET 300


uint16_t pos;
uint8_t mask = 0B10000000;
uint8_t lastbit;

long double period;
uint16_t lowCCR1, lowARR, highCCR1, highARR, tresetARR;

uint8_t *LED_data = nullptr;

DriverLED::DriverLED(FlashDriverInterface* flash)
{
	this->flash = flash;
	colorsArray = new RGB[LED_COUNT];
	LED_data = (uint8_t*)colorsArray;

	period = 1 / TIMER_FREQ;

	lowCCR1 = round(T0H / period);
	lowARR = round((T0H + T0L) / period);
	highCCR1 = round(T1H / period);
	highARR = round((T1H + T1L) / period);
	tresetARR = ceil(TRESET / period);

	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; //enable the timer4 clock
	TIM4->PSC = 0;   //set prescale to zero as timer has to go as fast as posible
	TIM4->CCMR1 = (TIM4->CCMR1 & ~(0b110<<4)) | (0b110<<4); //set PWM mode 110
	TIM4->CCR1 = 0; //set to zero so that the pin stay low until transmission
	TIM4->ARR = tresetARR; //set to timing for reset LEDs
	TIM4->CCER |= TIM_CCER_CC1E; //enable output to pin.
	TIM4->CR1 |= TIM_CR1_CEN; //Disable channel 1. This bit is used to start and stop transmission.
	TIM4->CR1 |= TIM_CR1_ARPE; //buffer ARR
	TIM4->CCMR1 |= TIM_CCMR1_OC1PE; //buffer CCR1
	TIM4->DIER &= ~TIM_DIER_UIE; // ensure we are not enabling interrupt flag to be generated this bit is used to start/stop transmission
	TIM4->CR1 |= TIM_CR1_CEN; //enable channel 1

	NVIC_EnableIRQ(TIM4_IRQn);

	animationPowerOn();
	loadLedSettings();
}

void DriverLED::animationPowerOn()
{
	const uint8_t maxBrightness = 100;
    uint8_t row, col, i;
    uint16_t ledIndex;

    for(i = 0; i < maxBrightness; i += 2) {
        fill(i, i, i);
        refresh();
        HAL_Delay(20);
    }

    for(i = 100; i > 0; i -= 2) {
        for(row = 0; row < 8; row++) {
            for(col = 0; col < 8; col++) {
                ledIndex = row * 8 + (7 - col);
                if((row + col) % 2 == 0) {
                    setPixel(ledIndex, i/3, i/3, i/3); // Затухающие черные
                } else {
                    setPixel(ledIndex, maxBrightness, maxBrightness, maxBrightness); // Белые
                }
            }
        }
        refresh();
        HAL_Delay(20);
    }

    HAL_Delay(200);
    for(row = 0; row < 8; row++) {
        for(col = 0; col < 8; col++) {
            ledIndex = row * 8 + (7 - col);
            if((row + col) % 2 == 0) {
                setPixel(ledIndex, 0, 0, 0);
            } else {
                setPixel(ledIndex, maxBrightness, maxBrightness, maxBrightness);
            }
        }
    }
    fill(0, 0, 0);
    refresh();
}

DriverLED::~DriverLED()
{
	delete colorsArray;
}

uint8_t DriverLED::getBrightness() const
{
	return brightness;
}

void DriverLED::setBrightness(uint8_t brightness)
{
	if (brightness > 100) return;
	this->brightness = brightness;
}

void DriverLED::clear()
{
	for (int i = 0; i < LED_COUNT; i++)
	{
		this->setPixel(i, 0, 0, 0);
	}
}

void DriverLED::fill(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < LED_COUNT; i++)
	{
		this->setPixel(i, r, g, b);
	}
}

void DriverLED::setPixel(uint16_t diodeId, uint8_t r, uint8_t g, uint8_t b)
{
	colorsArray[diodeId].red = r * this->brightness / 200;
	colorsArray[diodeId].green = g * this->brightness / 200;
	colorsArray[diodeId].bule = b * this->brightness / 200;
}

void DriverLED::refresh()
{
	pos = 0; //set the interupt to start at first byte
	lastbit = 0;
	mask = 0B10000000; //set the interupt to start at second bit

	TIM4->SR &= ~TIM_SR_UIF; // clear UIF flag
	TIM4->DIER |= TIM_DIER_UIE; //enable interupt flag to be generated to start transmission
	while (pos < LED_COUNT*sizeof(RGB)) { __NOP(); } // wait end of update
}

extern "C"
{
void TIM4_IRQHandler_Custom()
{
	TIM4->SR &= ~TIM_SR_UIF; // clear UIF flag
	if (pos < LED_COUNT*sizeof(RGB))
	{
		if (LED_data[pos] & mask)
		{
			TIM4->CCR1 = highCCR1;
			TIM4->ARR = highARR;
		}
		else
		{
			TIM4->CCR1 = lowCCR1;
			TIM4->ARR = lowARR;
		}
		if (mask == 1)
		{
			mask = 0B10000000;
			pos += 1;
		}
		else
			mask = mask >> 1;
	}
	else
	{
		TIM4->CCR1 = 0; //set to zero so that pin stays low
		TIM4->ARR = tresetARR; //set to timing for reset LEDs
		TIM4->DIER &= ~TIM_DIER_UIE; //disable interrupt flag to end transmission.
	}
}

}

void DriverLED::messege(const std::string &message)
{
	// get brightness
	if (message.find("BRIGHT?") != std::string::npos)
	{
		printf("Brightness:%d\n",this->brightness);
	}
	// set brightness
	if (message.find("BRIGHT:") != std::string::npos)
	{
		int param = 0;
	    if (sscanf(message.c_str(), "BRIGHT:%d", &param) == 1)
	    {
	    	if (param >= 0 && param <= 100)
	    	{
				setBrightness(param);
				saveLedSettings();
				printf("Brightness set:%d\n",param);
	    	}
	    }
	}
}

void DriverLED::setFieldHighlight(int index, unsigned char r, unsigned char g, unsigned char b)
{
	this->setPixel(index, r, g, b);
}

void DriverLED::clearHighlight()
{
	clear();
}

void DriverLED::loadLedSettings() {
	if(flash->isInit())
	{
		flash->read(512, (uint8_t*)&this->brightness, sizeof(this->brightness));
	}
}

void DriverLED::saveLedSettings() {
	if(flash->isInit())
	{
		flash->write(512, (uint8_t*)&this->brightness, sizeof(this->brightness));
	}
}
