/*
 * Fields.h
 *
 *  Created on: Mar 2, 2026
 *      Author: alex
 */

#ifndef ENTITIES_FIELDS_H_
#define ENTITIES_FIELDS_H_


#include <memory.h>

class Fields {
public:
	enum FieldState {none, white, black};

private:
	FieldState fields[64] = {none};

public:
	Fields() {};
	bool operator!=(const Fields& rhs){return memcmp(this->fields, rhs.fields, 64) != 0;}
	virtual ~Fields() {};
	FieldState getField(int i) {return fields[i];}
	void setField(int i, FieldState state) {fields[i] = state;};
};
#endif /* ENTITIES_FIELDS_H_ */
