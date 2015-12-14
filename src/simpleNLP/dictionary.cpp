/*
 * Dictionary.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: litton
 */

#include "dictionary.h"
#include <string>
dictionary::dictionary() {
	//TODO load from data sources
	locs.push_back("東京");
	locs.push_back("京都");
	locs.push_back("日本");
	locs.push_back("板橋");
	locs.push_back("国際");
	locs.push_back("帝都");

	companies.push_back("東京都個人タクシ一協同組合");
	companies.push_back("東京都個人タクシ一協会");
	companies.push_back("東京タクシ一センタ一");
	companies.push_back("国際自勤車株式会社");
	companies.push_back("kmグループ");
	companies.push_back("日本交通グループ");
	companies.push_back("日本交通");
	companies.push_back("板橋営業所");
	companies.push_back("グリ一ンキャブ");
	companies.push_back("日立自勤車交通第二(株)");
	companies.push_back("本所タクシ一株式会社");
	companies.push_back("第一交通株式会社");
	companies.push_back("本州自動車株式会社");
	companies.push_back("美松交通");
	companies.push_back("帝都自動車交通");
	companies.push_back("飛鳥交通第二株式会社");
	companies.push_back("新進タクシー株式会社");
	companies.push_back("日立自動車交通第三");
	companies.push_back("扇橋交通株式会社");
	companies.push_back("大栄交通株式会社");

	companyFeats.push_back("営業所");
	companyFeats.push_back("グル一プ");
	companyFeats.push_back("グルプ");
	companyFeats.push_back("会社");
	companyFeats.push_back("株式");
	companyFeats.push_back("協会");
	companyFeats.push_back("組合");
	companyFeats.push_back("カンパニ一");
	companyFeats.push_back("公司");
	companyFeats.push_back("センタ一");
	companyFeats.push_back("(株)");

	businesses.push_back("タクシ一");
	businesses.push_back("交通");
	businesses.push_back("自勤車");

	allFeats.push_back("円");
	allFeats.push_back("\\");
	allFeats.push_back("合");
	allFeats.push_back("計");
	allFeats.push_back("賃");
	allFeats.push_back("金");
	allFeats.push_back("年");
	allFeats.push_back("月");
	allFeats.push_back("日");
	charFeats = allFeats;

	allFeats.push_back("NB");
	allFeats.push_back("NBST");
	allFeats.push_back("LOC");
	allFeats.push_back("CPFT");
	allFeats.push_back("BZFT");

	moneyEnds.push_back("円");
	moneyEnds.push_back("元");

	words.insert(words.end(),locs.begin(),locs.end());
	words.insert(words.end(),businesses.begin(),businesses.end());
	words.insert(words.end(),companyFeats.begin(),companyFeats.end());
}

dictionary::~dictionary() {

}
