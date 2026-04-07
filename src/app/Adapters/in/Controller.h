/*
 * Controller.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef ADAPTERS_CONTROLLER_H_
#define ADAPTERS_CONTROLLER_H_

#include "FieldStatusInterface.h"
#include "../../Interactor/InputBoundary.h"
#include <stdint.h>

class Controller :  public FieldStatusInterface
{
private:
	uint32_t lastMove = 0;

	bool isInit = false;
	Fields internFields;

	InputBoundary *input = nullptr;
public:
	Controller(InputBoundary *input);
	virtual ~Controller();
protected:
	virtual void fieldStatus(Fields &fields) override;
};

#endif /* ADAPTERS_CONTROLLER_H_ */
