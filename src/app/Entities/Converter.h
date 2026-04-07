/*
 * Converter.h
 *
 *  Created on: Jan 30, 2026
 *      Author: alex
 */

#ifndef CONVERTER_H_
#define CONVERTER_H_

#include <string>

class Converter
{
public:
	static int convert(std::string coord)
	{
		if (coord.length() < 2) return -1;

		char file = coord.at(0);
		char rank = coord.at(1);
		int index = 'h' - file;
		index += (rank - '1') * 8;
		return index;
	}

	static std::string convert(int coord)
	{
		char file = 'a'+ (7 - coord % 8);
		char rank = '1' + coord / 8;
		std::string res;
		res.push_back(file);
		res.push_back(rank);
		return res;
	}
};

#endif /* CONVERTER_H_ */
