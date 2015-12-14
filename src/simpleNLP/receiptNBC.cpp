/*
 * receiptNBC.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: litton
 */

#include "receiptNBC.h"
#include <math.h>
#include <iostream>

using namespace std;

receiptNBC::receiptNBC(dictionary dict) {
	//TODO should be loaded from db or file
	this->dict = dict;
	typeProb["money_sum"] = 0.05;
	typeProb["money_any"] = 0.1;
	typeProb["company_name"] = 0.1;
	typeProb["date_time"] = 0.1;
	typeProb["other"] = 0.6;

	map<string, float> mp1;
	mp1["\\"] = 0.9;
	mp1["円"] = 0.9;
	mp1["NB"] = 0.999999;
	mp1["合"] = 0.9;
	mp1["計"] = 0.9;
	mp1["NBST"] = 0.8;
	mp1["金"] = 0.1;

	map<string, float> mp2;
	mp2["\\"] = 0.9;
	mp2["円"] = 0.9;
	mp2["NB"] = 0.999999;
	mp2["賃"] = 0.1;
	mp2["NBST"] = 0.8;
	mp2["金"] = 0.1;

	map<string, float> mp3;
	mp3["CPFT"] = 0.9;
	mp3["BZFT"] = 0.7;
	mp3["LOC"] = 0.8;

	map<string, float> mp4;
	mp4["年"] = 0.8;
	mp4["月"] = 0.8;
	mp4["日"] = 0.8;
	mp4["NB"] = 0.999999;
	mp4["NBST"] = 0.999999;

	map<string, float> mp5;
	mp5["NB"] = 0.15;
	mp5["NBST"] = 0.15;
	mp5["計"] = 0.1;
	mp5["賃"] = 0.05;
	mp5["合"] = 0.1;

	typeFeatProb["money_sum"] = mp1;
	typeFeatProb["money_any"] = mp2;
	typeFeatProb["company_name"] = mp3;
	typeFeatProb["date_time"] = mp4;
	typeFeatProb["other"] = mp5;
}

receiptNBC::~receiptNBC() {
}

float receiptNBC::getEventProb(float probIfExist, set<string> feats,
		string feat) {
	set<string>::iterator it = feats.find(feat);
	if (it != feats.end()) {
		return log(probIfExist);
	} else {
		return log(1 - probIfExist);
	}
}

float receiptNBC::calcProb(string type, set<string> feats) {
	float rt = log(typeProb[type]);

	map<string, float> mp = typeFeatProb[type];
	for (int i = 0; i < dict.allFeats.size(); i++) {
		string feat = dict.allFeats[i];
		if (mp.find(feat) != mp.end()) {
			rt += getEventProb(mp[feat], feats, feat);
		} else {
			rt += getEventProb(smoothing, feats, feat);
		}
	}
	return rt;
}

string receiptNBC::classify(set<string> feats) {
	string types[] = { "money_sum", "money_any", "company_name", "date_time",
			"other" };
		cout<<"[feats]"<<endl;
		for(set<string>::iterator it=feats.begin();it!=feats.end();it++)
			cout<<(*it)<<endl;
		cout<<endl;

	float prob = -99999999;
	string selected = "";

	for (int i = 0; i < 5; i++) {
		string type = types[i];
		float probNow = calcProb(type, feats);
		cout<<type<<" "<<probNow<<endl;
		if (probNow > prob) {
			prob = probNow;
			selected = type;
		}
	}
	return selected;
}
