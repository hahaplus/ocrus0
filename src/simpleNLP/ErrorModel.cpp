/*
 * ErrorModel.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: litton
 */

#include "ErrorModel.h"

ErrorModel::ErrorModel() {
	numErrors["(+)"] = "0";
	numErrors["〔+)"] = "0";
	numErrors["(+]"] = "0";
	numErrors["〔+)"] = "0";
	numErrors["〔+〕"] = "0";
	numErrors["(+}"] = "0";
	numErrors["()"] = "0";
	numErrors["〔)"] = "0";
	numErrors["(]"] = "0";
	numErrors["〔)"] = "0";
	numErrors["〔〕"] = "0";
	numErrors["(}"] = "0";
	numErrors["Q"] = "0";
	numErrors["S)"] = "9";
	numErrors["…)"] = "9";
	numErrors["璽)"] = "9";
	numErrors["g"] = "9";
	numErrors["ー"] = "1";

	fontErrors["フ・"] = "プ";
	fontErrors["丿し"] = "ル";
	fontErrors["'丿"] = "リ";
	fontErrors["ク〝"] = "グ";
	fontErrors["(掛"] = "(株)";
	fontErrors["主朱"] = "株";
	fontErrors["キ朱"] = "株";

	fontErrors["欄"] = "(株)";
	fontErrors["{欄"] = "(株)";
	fontErrors["~"] = "一";
	fontErrors["ソ"] = "シ";
	fontErrors["…"] = "一";
	fontErrors["f土"] = "社";
	fontErrors["‡土"] = "社";
	fontErrors["ネ土"] = "社";
	fontErrors["宇土"] = "社";
	fontErrors["ム=ロ"] = "合";
	fontErrors["註"] = "計";
}

ErrorModel::~ErrorModel() {
	// TODO Auto-generated destructor stub
}
