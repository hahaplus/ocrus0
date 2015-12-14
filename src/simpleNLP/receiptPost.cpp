#include<string>
#include<vector>
#include<map>
#include<set>
#include<iostream>
#include<fstream>
#include<sstream>
#include<algorithm>
#include <stdlib.h>
#include "dictionary.h"
#include "ErrorModel.h"
#include "receiptNBC.h"
#include "dirent.h"

using namespace std;

map<int, map<string, float> > candMap0;
map<int, vector<int> > lines0;
map<int, map<string, float> > candMap;
map<int, vector<int> > lines;
dictionary dict;
ErrorModel emodel;

const int DATE_ERROR_TOLERANT = 4;

void makeMockData(string path) {

	ifstream fin(path.c_str());
	std::locale::global(std::locale("ja_JP.UTF-8"));
	string s;

	while (getline(fin, s)) {
		size_t bracketRight = s.find(")");
		if (bracketRight != string::npos) {
			string posMark = s.substr(1, bracketRight - 1);
			size_t posMarkSpace = posMark.find(" ");

			int lineMark = atoi(posMark.substr(0, posMarkSpace).c_str());
			int charMark = atoi(posMark.substr(posMarkSpace + 1).c_str());

			if (lines0.find(lineMark) == lines0.end()) {
				vector<int> v;
				lines0[lineMark] = v;
			}
			lines0[lineMark].push_back(charMark);

			map<string, float> mp;
			size_t colon = s.find("：");

			string candstr = s.substr(colon + 3);
			istringstream iss(candstr);
			string substr;
			int count = 0;
			string cand;
			while (iss >> substr) {

				if (count % 2 == 0 && substr.length() < 4) {
					cand = substr;
					//					cout << "[cand] " << substr << " ";
				} else {
					float db = atof(substr.c_str());
					if (count % 2 == 1) {
						mp[cand] = db;
						//						cout << "[score] " << db << endl;
					} else {
						mp[" "] = db;
						//						cout << "[cand] [space]" << "[score] " << db << endl;
					}
				}
				count++;
			}
			candMap0[charMark] = mp;
		}
	}
	cout << candMap0.size() << " " << lines0.size() << endl;
}

string mysubhead(int curSeq, string seq) {
	if (seq.empty() || curSeq >= seq.length())
		return "";

	char c = seq[curSeq];
	if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9'
			|| c == '.' || c == '(' || c == ')' || c == ']' || c == ']' || c
			== '\\' || c == '{' || c == '}' || c == '+')
		return seq.substr(curSeq, 1);
	return seq.substr(curSeq, 3);
}

bool recursiveMatchSeq(int curChar, int curSeq, vector<int> chars,
		vector<string> seq, map<int, map<string, float> > candMap) {

	//	cout << seq << " " << curChar << " " << curSeq << endl;
	if (seq.size() - curSeq <= chars.size() - curChar) {
		map<string, float> mp = candMap[chars[curChar]];
		string letter = seq[curSeq];
		//		cout << "[letter] " << letter << endl;
		//		for(map<string, float>::iterator it = mp.begin();it!=mp.end();it++){
		//			cout<<"  "<<it->first<<endl;
		//		}

		if (mp.find(letter) != mp.end()) {

			if (curSeq + 1 >= seq.size()) {
				//				cout << "curSeq full " << curSeq << " " << seq << " "
				//						<< seq.length() << endl;
				return true;
			} else if (recursiveMatchSeq(curChar + 1, curSeq + 1, chars, seq,
					candMap))
				return true;
		}
	}
	/*
	 string backup = seq;
	 for (map<string, string>::iterator it = emodel.fontErrors.begin(); it
	 != emodel.fontErrors.end(); it++) {
	 string correct = it->second;
	 if (seq.substr(curSeq, correct.length()).compare(correct) == 0) {
	 if (curSeq <= 1) {
	 seq = it->first + seq.substr(curSeq + correct.length());
	 map<int, map<string, float> > candMap0;
	 } else {
	 seq = seq.substr(0, curSeq - 1) + it->first + seq.substr(
	 curSeq + correct.length());
	 }
	 if (recursiveMatchSeq(curChar, curSeq, chars, seq, candMap)) {
	 seq = backup;
	 return true;
	 }
	 }
	 seq = backup;
	 }*/
	return false;
}

bool myPrefixMatch(string tar, string str) {
	int curStr = 0;
	int curTar = 0;
	string subhead = mysubhead(curStr, str);

	//	cout << "match prefix [" << tar << "] [" << str << "]" << endl;
	while (!subhead.empty()) {
		if (subhead.length() == 1 && (tar[curTar] == '+' || tar[curTar]
				== subhead[0])) {
			curStr++;
			curTar++;
			subhead = mysubhead(curStr, str);
			continue;
		}
		if (subhead.length() == 3 && (tar[curTar] == '+') && subhead.compare(
				"株") != 0 && subhead.compare("円") != 0) {
			curStr += 3;
			curTar++;
			subhead = mysubhead(curStr, str);
			continue;
		}
		if (subhead.length() == 3 && tar.substr(curTar, 3).compare(subhead)
				== 0) {
			curStr += 3;
			curTar += 3;
			subhead = mysubhead(curStr, str);
			continue;
		}
		//		cout << "prefix not match! " << endl;
		return false;
	}
	//	cout << "prefix match!" << endl;	map<int, map<string, float> > candMap0;
	return true;
}
int stringLen(string s) {
	int rst = 0;
	int cur = 0;
	string letter = mysubhead(cur, s);
	while (!letter.empty()) {
		rst++;
		cur += letter.length();
		letter = mysubhead(cur, s);
	}
	return rst;
}
int matchError(int i, vector<int> chars0,
		map<int, map<string, float> > candMap0, string wrong, string str,
		int len) {

	//	cout << "wrong " << wrong << " string " << str << endl;
	//	cout << "[string len] " << wrong.length() << " " << stringLen(wrong)
	//			<< endl;
	if (wrong.length() != stringLen(wrong)) {
		int k = 0;
	}
	if (len == stringLen(wrong))
		return len;

	map<string, float> mp = candMap0[chars0[i + len]];
	if (mp.find("株") == mp.end() && mp.find("円") == mp.end())
		for (map<string, float>::iterator it = mp.begin(); it != mp.end(); it++) {
			if (myPrefixMatch(wrong, str + (it->first))) {
				int ret = matchError(i, chars0, candMap0, wrong,
						str + (it->first), len + 1);
				if (ret > 0)
					return ret;
			}
		}
	return 0;
}

bool isDigi(map<string, float> mp) {
	for (map<string, float>::iterator it = mp.begin(); it != mp.end(); it++) {
		char c = it->first[0];
		if ((c >= '0' && c <= '9') || c == '.')
			return true;
		//		if (emodel.numErrors.find(it->first) != emodel.numErrors.end())
		//			return true;
	}
	return false;
}

bool isErrorDigi(map<string, float> mp) {
	for (map<string, float>::iterator it = mp.begin(); it != mp.end(); it++) {
		if (emodel.numErrors.find(it->first) != emodel.numErrors.end())
			return true;
	}
	return false;
}

int matchChar(int& i, int& cur, vector<int> chars0,
		map<int, map<string, float> > candMap0,
		map<int, map<string, float> >& rst, bool num) {

	if (num && (!isDigi(candMap0[chars0[i]])))
		for (map<string, string>::iterator itr = emodel.numErrors.begin(); itr
				!= emodel.numErrors.end(); itr++) {

			string wrong = itr->first;
			string right = itr->second;

			int len = matchError(i, chars0, candMap0, wrong, "", 0);
			if (len > 0) {
				map<string, float> mp;
				mp[right] = 1;
				if (len == 1) {
					mp.insert(candMap0[chars0[i]].begin(),
							candMap0[chars0[i]].end());
				}
				//			cout << "wrong to right " << wrong << " " << right << endl;
				rst[chars0[cur++]] = mp;
				return len;
			}
		}
	if (!num && !isDigi(candMap0[chars0[i]]))
		for (map<string, string>::iterator itr = emodel.fontErrors.begin(); itr
				!= emodel.fontErrors.end(); itr++) {

			string wrong = itr->first;
			string right = itr->second;
			cout << i << " " << wrong << " " << right << endl;
			int len = matchError(i, chars0, candMap0, wrong, "", 0);
			if (len > 0) {
				map<string, float> mp;
				mp[right] = 1;
				if (len == 1) {
					mp.insert(candMap0[chars0[i]].begin(),
							candMap0[chars0[i]].end());
				}
				//			cout << "wrong to right " << wrong << " " << right << endl;
				rst[chars0[cur++]] = mp;
				return len;
			}
		}
	rst[chars0[cur++]] = candMap0[chars0[i]];
	return 1;
}

map<int, map<string, float> > correctChars(vector<int> chars0,
		map<int, map<string, float> > candMap0, bool num) {
	map<int, map<string, float> > rst;
	int cur = 0;
	for (int i = 0; i < chars0.size();) {
		if (chars0[i] == 169) {
			int k = 9;
		}
		i += matchChar(i, cur, chars0, candMap0, rst, num);
	}
	for (map<int, map<string, float> >::iterator it = rst.begin(); it
			!= rst.end(); it++) {
		cout << "MAP FOR CHAR: " << it->first << endl;
		map<string, float> mp = it->second;
		for (map<string, float>::iterator it2 = mp.begin(); it2 != mp.end(); it2++) {
			cout << "  " << it2->first << endl;
		}
	}
	return rst;
}

void correctAll(map<int, map<string, float> > candMap0, bool num) {

	for (map<int, vector<int> >::iterator it = lines0.begin(); it
			!= lines0.end(); it++) {
		cout << "CORRECT LINE " << it->first << endl;
		map<int, map<string, float> > candMap2 = correctChars(
				lines0[it->first], candMap0, num);
		vector<int> chars;

		for (map<int, map<string, float> >::iterator it2 = candMap2.begin(); it2
				!= candMap2.end(); it2++) {
			chars.push_back(it2->first);
		}
		sort(chars.begin(), chars.end());
		lines[it->first] = chars;
		cout << "LINE CHARS ";
		for (int i = 0; i < chars.size(); i++) {
			cout << chars[i] << " ";
			candMap[chars[i]] = candMap2[chars[i]];
		}
		cout << endl << endl;
	}
	cout << candMap.size() << " " << lines.size() << endl;
}

bool matchSeq(int i, vector<int> chars, map<int, map<string, float> > candMap,
		vector<string> seqs) {

	for (unsigned int j = 0; j < seqs.size(); j++) {
		string seq = seqs[j];
		//		cout<<seq<<endl;
		vector<string> seqvec;
		int cur = 0;
		string letter = mysubhead(cur, seq);
		while (!letter.empty()) {
			seqvec.push_back(letter);
			cur += letter.length();
			letter = mysubhead(cur, seq);
			//				cout<<"letter "<<letter<<endl;
		}
		if (recursiveMatchSeq(i, 0, chars, seqvec, candMap)) {
			return true;
		}
	}
	return false;
}

bool findCorrectedChar(map<string, float> mp, string charFeat,
		ErrorModel emodel) {
	for (map<string, float>::iterator it = mp.begin(); it != mp.end(); it++) {
		if (emodel.fontErrors.find(it->first) != emodel.fontErrors.end()
				&& emodel.fontErrors[it->first].compare(charFeat) == 0) {
			return true;
		}
	}
	return false;
}
int I = 0;
int SCAN = 0;
set<string> scanFeats(vector<int> chars, map<int, map<string, float> > candMap,
		dictionary dict) {
	SCAN++;
	set<string> rt;
	int c_numLen = 0;
	int f_numLen = 0;

	for (int i = 0; i < chars.size(); i++) {
		I = i;
		int charId = chars[i];
		map<string, float> mp = candMap[charId];
		for (int j = 0; j < dict.charFeats.size(); j++) {
			string charFeat = dict.charFeats[j];
			if (rt.find(charFeat) == rt.end() && (mp.find(charFeat) != mp.end()
					|| findCorrectedChar(mp, charFeat, emodel))) {
				rt.insert(charFeat);
			}
		}
		if (isDigi(mp)) {
			c_numLen++;
			rt.insert("NB");
			continue;
		} else {
			if (c_numLen > f_numLen) {
				f_numLen = c_numLen;
			}
			c_numLen = 0;

			if (matchSeq(i, chars, candMap, dict.locs)) {
				rt.insert("LOC");
				continue;
			}
			if (matchSeq(i, chars, candMap, dict.businesses)) {
				rt.insert("BZFT");
				continue;
			}
			if (matchSeq(i, chars, candMap, dict.companyFeats)) {
				rt.insert("CPFT");
			}
		}
	}
	if (f_numLen > 0 && f_numLen < 5) {
		rt.insert("NBST");
	}
	for (set<string>::iterator it = rt.begin(); it != rt.end(); it++) {
		cout << " " << (*it) << endl;
	}
	return rt;
}

map<int, string> scanForTypes(map<int, map<string, float> > candMap,
		map<int, vector<int> > lines, dictionary dict) {
	map<int, string> rst;
	receiptNBC nbc(dict);

	for (map<int, vector<int> >::iterator it = lines.begin(); it != lines.end(); it++) {
		rst[it->first] = nbc.classify(scanFeats(it->second, candMap, dict));
		if (it->second.size() > 19 && (rst[it->first].find("money")
				!= rst[it->first].npos || rst[it->first].find("date")
				!= rst[it->first].npos)) {
			rst[it->first] = "other";
		}
		cout << it->first << " " << rst[it->first] << endl;
	}
	return rst;
}

string findFirstDigi(map<string, float> mp) {
	for (map<string, float>::iterator it = mp.begin(); it != mp.end(); it++) {
		char c = it->first[0];
		if (it->second > 0.2) {
			if (c >= '0' && c <= '9')
				return it->first;
			//			if (emodel.numErrors.find(it->first) != emodel.numErrors.end())
			//				return emodel.numErrors[it->first];
		}
	}
	return "";
}

string correctMoney(map<int, map<string, float> > candMap, int before, int end) {
	string rt = "";
	for (int i = before; i <= end; i++) {
		rt += findFirstDigi(candMap[i]);
	}
	//	cout<<"money "+rt<<endl;
	return rt;
}

string correctAndExtractMoney(vector<int> chars,
		map<int, map<string, float> > candMap, dictionary dict) {
	int maxLen = 0;
	int nm_begin = -1;
	int nm_end = -1;
	string rt = "";

	int tolerate = 0;
	for (int i = 0; i < chars.size(); i++) {
		int charIdx = chars[i];
		map<string, float> mp = candMap[charIdx];
		if (mp.find(")") != mp.end())
			mp["9"] = 1;

		if (isDigi(mp)) {
			if (nm_begin == -1) {
				nm_begin = i;
			}
			nm_end = i;
			tolerate = 0;
		} else {
			if (nm_begin != -1 && mp.find(" ") == mp.end()) {
				for (int j = 0; j < dict.moneyEnds.size(); j++) {
					if (mp.find(dict.moneyEnds[j]) != mp.end()) {
						//						cout<<"find!"<<endl;
						return correctMoney(candMap, chars[nm_begin],
								chars[nm_end]) + dict.moneyEnds[j];
					}
				}
				if (1 + nm_end - nm_begin > maxLen) {
					maxLen = 1 + nm_end - nm_begin;
					rt = correctMoney(candMap, chars[nm_begin], chars[nm_end]);
				}

			} else {
				if (nm_begin != -1) {
					nm_end = i;
				}
			}
			tolerate++;
			if (tolerate > 1||mp.find("\\") != mp.end()) {
				nm_begin = -1;
				nm_end = -1;
				tolerate = 0;
			}
		}
	}
	//	cout<<"return correct money "+rt;
	return rt;
}

string correctDate(vector<string> pattern, int matchLen) {
	if (pattern[matchLen].compare("*") != 0) {
		return pattern[matchLen];
	}
	if (matchLen == 0)
		return "年";
	if (matchLen == 1)
		return "月";
	return "日";
}

string correctDateDigi(string digis) {
	if (digis.length() == 1)
		return "0" + digis;
	return digis;
}

string matchDatePattern(vector<string> pattern, vector<int> chars,
		map<int, map<string, float> > candMap) {

	//	cout << "MDP" << endl;

	string rt = "";
	string digis = "";

	int matchLen = -1;
	int tolerant = 0;

	for (int i = 0; i < chars.size(); i++) {
		int charIdx = chars[i];
		if(charIdx==188){
			int k=188;
		}
		map<string, float> mp = candMap[charIdx];
		if (mp.find(")") != mp.end()) {
			mp["9"] = 1;
		}
		if (matchLen >= 0 && !isDigi(candMap[charIdx]) && !isErrorDigi(
				candMap[charIdx])) {

			if ((candMap[charIdx].find(pattern[matchLen])
					!= candMap[charIdx].end() || pattern[matchLen].compare("*")
					== 0) && digis.length() >= 1 && digis.length() < 5) {
				//				cout<<"match "<<pattern[matchLen]<<" digit "<<digis<<endl;
				rt += correctDateDigi(digis);
				//				cout<<"add1 "<<rt<<" "<<matchLen<<endl;
				rt += correctDate(pattern, matchLen++);
				//				cout<<"add2 "<<matchLen<<" "<<pattern.size()<<endl;
				if (matchLen >= pattern.size()) {
					return rt;
				}
				digis = "";
			} else {
				if (tolerant < DATE_ERROR_TOLERANT) {
					tolerant++;
				} else {
					rt = "";
					digis = "";
					matchLen = -1;
					tolerant = 0;
				}
			}
		} else {
			if (isDigi(candMap[charIdx]) || isErrorDigi(candMap[charIdx])) {
				if (matchLen == -1)
					matchLen = 0;
				digis += findFirstDigi(candMap[charIdx]);
				tolerant = 0;
			}
		}
	}
	return "";
}

string findDatePattern(vector<int> chars, map<int, map<string, float> > candMap) {

	string rt = "";

	//match pattern one: 年月日
	vector<string> pattern1;
	pattern1.push_back("年");
	pattern1.push_back("月");
	pattern1.push_back("日");
	rt = matchDatePattern(pattern1, chars, candMap);
	if (!rt.empty())
		return rt;

	pattern1.clear();
	pattern1.push_back("*");
	pattern1.push_back("*");
	pattern1.push_back("日");
	rt = matchDatePattern(pattern1, chars, candMap);
	if (!rt.empty())
		return rt;

	pattern1.clear();
	pattern1.push_back("年");
	pattern1.push_back("*");
	pattern1.push_back("*");
	rt = matchDatePattern(pattern1, chars, candMap);
	if (!rt.empty())
		return rt;

	pattern1.clear();
	pattern1.push_back("*");
	pattern1.push_back("月");
	pattern1.push_back("*");
	rt = matchDatePattern(pattern1, chars, candMap);
	if (!rt.empty())
		return rt;

	return rt;
}

string correctAndExtractDT(vector<int> chars,
		map<int, map<string, float> > candMap, dictionary dict) {

	string found = findDatePattern(chars, candMap);
	if (found.size() == 0) {
		return "PARSE ERROR";
	}
	return found;
}

string charToStr(char c) {
	stringstream ss;
	string s;
	ss << c;
	ss >> s;
	return s;
}

double min(double x, double y, double z) {
	return x < y ? min(x, z) : min(y, z);
}
//edit distance
double editDist(vector<int> chars, map<int, map<string, float> > candMap,
		vector<string> bvec) {
	//	cout<<"B: "<<B<<endl;
	size_t NA = chars.size();
	size_t NB = bvec.size();
	//	cout<<B<<" "<<NB<<endl;

	vector<vector<double> > M(NA + 1, vector<double> (NB + 1));

	for (size_t a = 0; a <= NA; ++a)
		M[a][0] = (int) a;

	for (size_t b = 0; b <= NB; ++b)
		M[0][b] = (int) b;

	for (size_t a = 1; a <= NA; ++a)
		for (size_t b = 1; b <= NB; ++b) {
			double x = M[a - 1][b] + 1;
			double y = M[a][b - 1] + 1;
			map<string, float> mp = candMap[chars[a - 1]];
			string bstr = bvec[b - 1];
			//			cout<<"bstr "<<bstr<<endl;
			double z = M[a - 1][b - 1] + (mp.find(bstr) != mp.end() ? 0 : 1);
			M[a][b] = min(x, y, z);
		}

	return M[NA][NB];
}

void replaceAll(std::string& str, const std::string& from,
		const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

string correctAndExtractCompany(vector<int> chars,
		map<int, map<string, float> > candMap, dictionary dict) {

	double minDist = 99999;
	string fname = "UNKNOWN COMPANY";
	bool found = false;
	for (int i = 0; i < dict.companies.size(); i++) {
		string name = dict.companies[i];
		//		cout<<name<<endl;
		vector<string> bvec;
		int cur = 0;
		string letter = mysubhead(cur, name);
		while (!letter.empty()) {
			bvec.push_back(letter);
			cur += letter.length();
			letter = mysubhead(cur, name);
			//						cout<<"letter "<<letter<<endl;
		}
		double dist = editDist(chars, candMap, bvec);
		if (dist < bvec.size() / 2 && dist < minDist) {
			fname = name;
			minDist = dist;
			found = true;
		}
	}
	//	if (found) {
	//		string fnameHead = fname;
	//		for (int i = 0; i < dict.companyFeats.size(); i++) {
	//			replaceAll(fnameHead, dict.companyFeats[i], "");
	//		}
	//
	//		vector<int> charHead;
	//		for (int i = 0; i < fnameHead.size() + minDist; i++)
	//			charHead.push_back(chars[i]);
	//
	//		double dist = editDist(charHead, candMap, fnameHead);
	//		if (dist - minDist > fname.size() / 2)
	//			fname = "UNKNOWN COMPANY";
	//	}
	return fname;
}

bool validDate(string& date) {
	int yIdx = date.find("年");
	int mIdx = date.find("月");
	int dIdx = date.find("日");
	//	cout<<date<<" "<<yIdx<<" "<<mIdx<<" "<<dIdx<<endl;
	int yDigi = atoi(date.substr(0, yIdx).c_str());
	int mDigi = atoi(date.substr(yIdx + 3, mIdx - yIdx - 3).c_str());
	int dDigi = atoi(date.substr(mIdx + 3, dIdx - mIdx - 3).c_str());

	if (yDigi > 2200 || yDigi < 2000 && yDigi > 99)
		return false;
	if (mDigi > 12 || mDigi < 1 || dDigi > 31 || dDigi < 1)
		return false;
	return true;
}

map<string, vector<string> > correctAndExtract(map<int, vector<int> > lines,
		map<int, string> lineToClass, map<int, map<string, float> > candMap,
		dictionary dict) {

	map<string, vector<string> > rt;
	for (map<int, vector<int> >::iterator it = lines.begin(); it != lines.end(); it++) {
		if (lineToClass[it->first].compare("other") != 0) {
			string lineClass = lineToClass[it->first];
			vector<int> chars = it->second;
			if (rt.find(lineClass) == rt.end()) {
				vector<string> v;
				rt[lineClass] = v;
			}

			if (lineToClass[it->first].compare("money_sum") == 0
					|| lineToClass[it->first].compare("money_any") == 0) {
				string rst = correctAndExtractMoney(chars, candMap, dict);
				if (!rst.empty()) {
					rt[lineClass].push_back(rst);
					//					cout<<"add "<<lineClass<<" "<<rst<<endl;
				}
			}
			/**/
			if (lineToClass[it->first].compare("company_name") == 0) {
				string rst = correctAndExtractCompany(chars, candMap, dict);
				if (!rst.empty() && rst.compare("UNKNOWN COMPANY") != 0) {
					rt[lineClass].push_back(rst);
					//					cout<<"add "<<lineClass<<" "<<rst<<endl;
				}
			}
			/**/
			if (lineToClass[it->first].compare("date_time") == 0) {
				string rst = correctAndExtractDT(chars, candMap, dict);
				if (validDate(rst) && !rst.empty()
						&& rst.compare("PARSE ERROR") != 0) {
					rt[lineClass].push_back(rst);
					//					cout<<"add "<<lineClass<<" "<<rst<<endl;
				}
			}
		}
	}
	return rt;
}

int main() {

	//	DIR *dir;
	//	struct dirent *ent;
	//
	//	if ((dir = opendir("/home/litton/Downloads/0643/")) != NULL) {
	//
	//		while ((ent = readdir(dir)) != NULL) {
	//			string dname(ent->d_name);
	//			if (dname.find("txt") != dname.npos && dname.find("whole")
	//					== dname.npos) {

	makeMockData("/home/litton/Downloads/0643/Photo-0003.txt");
	correctAll(candMap0, false);
	candMap0 = candMap;
	lines0 = lines;
	map<int, string> lineToClass = scanForTypes(candMap0, lines0, dict);
	correctAll(candMap0, true);
	map<string, vector<string> > extractedInfo = correctAndExtract(lines,
			lineToClass, candMap, dict);
	//				cout<<endl<<"INFORMATION OF "<<dname<<endl;
	for (map<string, vector<string> >::iterator it = extractedInfo.begin(); it
			!= extractedInfo.end(); it++) {
		if (!it->second.empty()) {
			cout << "FOUND: " + it->first << endl;
			for (int i = 0; i < it->second.size(); i++) {
				cout << "  " << it->second[i] << endl;
			}
			cout << endl;
		}
	}
	//			}
	//		}
	//		closedir(dir);
	//	}

	return 0;
}
