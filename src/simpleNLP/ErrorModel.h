/*
 * ErrorModel.h
 *
 *  Created on: Dec 3, 2015
 *      Author: litton
 */

#ifndef ERRORMODEL_H_
#define ERRORMODEL_H_

#include <map>
#include <string>

using namespace std;
class ErrorModel {
public:
	map<string, string> numErrors;
	map<string, string> fontErrors;
	ErrorModel();
	virtual ~ErrorModel();
};

#endif /* ERRORMODEL_H_ */
