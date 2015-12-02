/*
 * StringUtil.h
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#ifndef SRC_UTIL_STRING_UTIL_H_
#define SRC_UTIL_STRING_UTIL_H_
#include <string>
#include <sstream>
using namespace std;
class StringUtil {
public:
	StringUtil();
	virtual ~StringUtil();
	template <typename T>
	static string toString(T t)
	{
			stringstream ss;
			ss << t;
			string res;
		    ss >> res;
		    return res;
	}
};

#endif /* SRC_UTIL_STRING_UTIL_H_ */
