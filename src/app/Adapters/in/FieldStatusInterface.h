/*
 * FieldStatusInterface.h
 *
 *  Created on: Feb 2, 2026
 *      Author: alex
 */

#ifndef ADAPTERS_IN_FIELDSTATUSINTERFACE_H_
#define ADAPTERS_IN_FIELDSTATUSINTERFACE_H_

#include "../../app/Entities/Fields.h"

class FieldStatusInterface
{
public:
	virtual ~FieldStatusInterface() = default;
	virtual void fieldStatus(Fields &field) = 0;
};

#endif /* ADAPTERS_IN_FIELDSTATUSINTERFACE_H_ */
