/*
 * Controller.cpp
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#include "Controller.h"

#include <memory.h>
#include "stm32f1xx_hal.h"
#include "../../Entities/Converter.h"

Controller::Controller(InputBoundary *input)
{
	this->input = input;
}

Controller::~Controller()
{

}

void Controller::fieldStatus(Fields &fields)
{
	if (!isInit)
	{
		internFields = fields;
		input->setFields(internFields);
		isInit = true;
		return;
	}

	const uint32_t curTic = HAL_GetTick();

	if (internFields != fields)
	{
		if (curTic - lastMove  >= 100) // ms
		{
			for (int i = 0; i < 64; i++)
				if (internFields.getField(i) != fields.getField(i))
				{
					std::string s = Converter::convert(i);
//					fields.getField(i) != Fields::none ? printf("%s is down\n", s.c_str()) : printf("%s is up\n", s.c_str());
					input->figureMove(s, i, fields.getField(i) != Fields::none);
					internFields.setField(i, fields.getField(i));
				}
			lastMove = curTic;
		}
	}
}
