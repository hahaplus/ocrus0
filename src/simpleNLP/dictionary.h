/*
 * Dictionary.h
 *
 *  Created on: Dec 1, 2015
 *      Author: litton
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <vector>
#include <string>

using namespace std;

class dictionary {
public:
	vector<string> locs;
	vector<string> companies;
	vector<string> companyFeats;
	vector<string> businesses;
	vector<string> allFeats;
	vector<string> charFeats;
	vector<string> moneyEnds;

	vector<string> words;

	dictionary();
	virtual ~dictionary();
};

#endif /* DICTIONARY_H_ */
