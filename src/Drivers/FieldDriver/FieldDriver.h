/*
 * FieldDriver.h
 *
 *  Created on: Nov 7, 2025
 *      Author: alex
 */

#ifndef FieldDriver_H_
#define FieldDriver_H_

#include "../../app/Entities/Fields.h"
#include "../../app/Adapters/in/FieldStatusInterface.h"
#include "../BluetoothDriver/Listener.h"
#include "../FlashDriver/FlashDriverInterface.h"

class FieldDriver : public Listener
{
private:
	unsigned int seed = 0;
	float voltage = 0;
	Fields *fields = nullptr;
	FieldStatusInterface *interface = nullptr;
	FlashDriverInterface *flash = nullptr;
	bool isCalibrate = false;
	void calibration();
	void readState();
	void loadFieldSettings();
	void saveFieldSettings();
	void printField();
public:
	FieldDriver(
			Fields& fields,
			FieldStatusInterface* interface,
			FlashDriverInterface* flash);
	virtual ~FieldDriver ();

	const Fields* getFilds() const;

	unsigned int getSeed() {return seed;}

	void run();

	float getVRef() const {return voltage;}

	virtual void messege(const std::string &message) override;

	void setForceCalibrate() { this->isCalibrate = true; }
};

#endif /* FieldDriver_H_ */
