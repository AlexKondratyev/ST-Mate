/*
 * DriverLED.h
 *
 *  Created on: Nov 8, 2025
 *      Author: alex
 */

#ifndef DRIVERLED_H_
#define DRIVERLED_H_
#include "stm32f1xx_hal.h"

#include "../../app/Adapters/out/FieldHighlightInterface.h"
#include "../BluetoothDriver/Listener.h"
#include "../FlashDriver/FlashDriverInterface.h"

#define LED_COUNT 64

#pragma pack(push, 1)
struct RGB
{
	uint8_t green;
	uint8_t red;
	uint8_t bule;
};
#pragma pack(pop)

class DriverLED : public FieldHighlightInterface, public Listener
{
private:
	uint8_t brightness = 20; // must <= 25%

	RGB *colorsArray = nullptr;
	FlashDriverInterface *flash = nullptr;

	void animationPowerOn();
	void loadLedSettings();
	void saveLedSettings();
public:
	DriverLED(FlashDriverInterface* flash);
	virtual ~DriverLED();

	uint8_t getBrightness() const;
	void setBrightness(uint8_t brightness = 25);

	void clear();
	void fill(uint8_t r, uint8_t g, uint8_t b);
	void setPixel(uint16_t diodeId, uint8_t r, uint8_t g, uint8_t b);
	void refresh();

	virtual void messege(const std::string &message) override;
	virtual void setFieldHighlight(int index, unsigned char r, unsigned char g, unsigned char b) override;
	virtual void clearHighlight() override;
};

#endif /* DRIVERLED_H_ */
