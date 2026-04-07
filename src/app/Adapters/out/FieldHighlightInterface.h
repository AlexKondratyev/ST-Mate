/*
 * FieldHighlightInterface.h
 *
 *  Created on: Feb 8, 2026
 *      Author: alex
 */

#ifndef ADAPTERS_OUT_FIELDHIGHLIGHTINTERFACE_H_
#define ADAPTERS_OUT_FIELDHIGHLIGHTINTERFACE_H_

class FieldHighlightInterface
{
public:
	virtual void setFieldHighlight(int index, unsigned char r, unsigned char g, unsigned char b) = 0;
	virtual void clearHighlight() = 0;

};

#endif /* ADAPTERS_OUT_FIELDHIGHLIGHTINTERFACE_H_ */
