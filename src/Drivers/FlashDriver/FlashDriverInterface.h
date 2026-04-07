/*
 * FlashDriverInterface.h
 *
 *  Created on: Mar 26, 2026
 *      Author: alex
 */

#ifndef FLASHDRIVER_FLASHDRIVERINTERFACE_H_
#define FLASHDRIVER_FLASHDRIVERINTERFACE_H_

#include <stdint.h>

class FlashDriverInterface
{
public:
	~FlashDriverInterface() {};
	virtual bool isInit() = 0;
	virtual bool read(uint32_t addr, uint8_t* data, uint32_t len) = 0;
	virtual bool write(uint32_t addr, const uint8_t* data, uint32_t len) = 0;
};

#endif /* FLASHDRIVER_FLASHDRIVERINTERFACE_H_ */
