/*
 * Listener.h
 *
 *  Created on: Mar 23, 2026
 *      Author: alex
 */

#ifndef BLUETOOTHDRIVER_LISTENER_H_
#define BLUETOOTHDRIVER_LISTENER_H_

#include <string>

class Listener
{
public:
	virtual ~Listener(){};
	virtual void messege(const std::string &message) = 0;
};

#endif /* BLUETOOTHDRIVER_LISTENER_H_ */
