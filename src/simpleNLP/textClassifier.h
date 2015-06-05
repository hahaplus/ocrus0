/*
 * textClassifier.h
 *
 *  Created on: May 29, 2015
 *      Author: litton
 */

#ifndef TEXTCLASSIFIER_H_
#define TEXTCLASSIFIER_H_

#include <vector>
#include <string>
#include <armadillo>
#include <methods/logistic_regression/logistic_regression.hpp>

using namespace std;
using namespace arma;
using namespace mlpack::regression;

vector<string> split(string str, int mode){
	vector<string> rst;
	string delim = "\n";
	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0';

	if(mode==2)
		delim = " ";

	char* pch = strtok(writable,delim.c_str());
	while(pch!=NULL){
		string sub(pch);
		rst.push_back(sub);
//		cout<<sub<<endl;
		pch = strtok(NULL,delim.c_str());
	}
	return rst;
}

bool junkChar(char c){
	if(c<='z'&&c>='a')
		return false;
	if(c<='Z'&&c>='A')
		return false;
	if(c<='9'&&c>='0')
		return false;
	if(c=='('||c==')')
		return false;
	return true;
}

bool rubbishWord(string word){
	if(word.length()<3){
		for(int i=0;i<word.length();i++){
			char c = word[i];
			if(junkChar(c)){
//				cout<<"rubbish "<<word<<endl;
				return true;
			}
		}
	}
	return false;
}

mat predictor(2,14);
vec response(14);
int dataCount = 0;

void prepareVector(string input, int type){

	vector<string> splited = split(input,1);
	int countLines = 0;
	int totalWords = 0;
	int totalLenth = 0;

	for(int i=0;i<splited.size();i++){

		string sub = splited[i];
//		cout<<"LINE: "<<sub<<endl;
		vector<string> words = split(sub,2);
		int delWords = 0;
		for(int j=0;j<words.size();j++){

//			cout<<"WORD: "<<words[j]<<endl;
			if(rubbishWord(words[j])){
				delWords ++;
			}
			else{
				totalLenth += words[j].length();
			}
		}
//		cout<<"deleted: "<<delWords<<", total: "<<words.size()<<endl;
		//not rubbish line
		if((0.0+delWords)/words.size()<0.5){
			countLines++;
			totalWords+=(words.size()-delWords);

		}

	}
//	cout<<countLines<<" "<<(0.0+totalWords)/countLines<<" "<<(0.0+totalLenth)/totalWords<<" "<<type<<endl;
	predictor(0,dataCount) = countLines;
	predictor(1,dataCount) = (0.0+totalWords)/countLines;
//	predictor(2,dataCount) = (0.0+totalLenth)/totalWords;
	response[dataCount++] = type;
//	cout<<predictor(0,dataCount-1)<<" "<<predictor(1,dataCount-1)<<" "<<predictor(2,dataCount-1)<<" "<<type<<endl;
}

LogisticRegression<> trainClassifier(){
	LogisticRegression<> classifier(predictor,response);
	return classifier;
}

mat testMat(2,6);
vec testRes(6);
int testCount = 0;

void prepareTest(string input, int type){

	vector<string> splited = split(input,1);
	int countLines = 0;
	int totalWords = 0;
	int totalLenth = 0;

	for(int i=0;i<splited.size();i++){

		string sub = splited[i];
		vector<string> words = split(sub,2);
		int delWords = 0;
		for(int j=0;j<words.size();j++){
			if(rubbishWord(words[j])){
				delWords ++;
			}
			else{
				totalLenth += words[j].length();
			}
		}
		//not rubbish line
		if((0.0+delWords)/words.size()<0.5){
			countLines++;
			totalWords+=(words.size()-delWords);

		}

	}
	testMat(0,testCount) = countLines;
	testMat(1,testCount) = (0.0+totalWords)/countLines;
//	testMat(2,testCount) = (0.0+totalLenth)/totalWords;
	testRes[testCount++] = type;
//	cout<<testMat(0,testCount-1)<<" "<<testMat(1,testCount-1)<<" "<<testMat(2,testCount-1)<<endl;
}

#endif /* TEXTCLASSIFIER_H_ */
