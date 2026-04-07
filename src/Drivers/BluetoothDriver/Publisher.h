/*
 * Publisher.h
 *
 *  Created on: Mar 23, 2026
 *      Author: alex
 */

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include "Listener.h"
#include <list>

class Publisher
{
public:
	std::list<Listener*> listListeners;
	virtual ~Publisher(){};
	virtual void attach(Listener *listener) = 0;
	virtual void notify(const std::string &message) = 0;
};



#endif /* PUBLISHER_H_ */
