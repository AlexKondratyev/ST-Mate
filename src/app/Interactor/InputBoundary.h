/*
 * InputBoundary.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef INTERACTOR_INPUTBOUNDARY_H_
#define INTERACTOR_INPUTBOUNDARY_H_

#include <string>
#include "../../app/Entities/Fields.h"

class InputBoundary
{
public:
	virtual void setFields(Fields &filds) = 0;
	virtual void figureMove(const std::string& position, int index, bool isSet) = 0;
};

#endif /* INTERACTOR_INPUTBOUNDARY_H_ */
