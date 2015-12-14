/*
 * receiptNBC.h
 *
 *  Created on: Dec 1, 2015
 *      Author: litton
 */

#ifndef RECEIPTNBC_H_
#define RECEIPTNBC_H_
#include <set>
#include <string>
#include <map>
#include "dictionary.h"

using namespace std;

class receiptNBC {

public:
	const float smoothing = 0.0001;
	vector<string> allFeats;
	map<string, float> typeProb;
	map<string, map<string, float> > typeFeatProb;
	dictionary dict;

	receiptNBC(dictionary dict);
	virtual ~receiptNBC();
	string classify(set<string> feats);
	float calcProb(string type, set<string> feats);
	float getEventProb(float probIfExist, set<string> feats, string feat);
};

#endif /* RECEIPTNBC_H_ */
