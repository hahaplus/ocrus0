/*
 * namecardPost.h
 *
 *  Created on: May 22, 2015
 *      Author: litton
 */

#ifndef NAMECARDPOST_H_
#define NAMECARDPOST_H_

#include<string>
#include<vector>
#include<map>
#include<set>
#include"viterbi.hpp"

using namespace std;
using namespace Viterbi;

//dictionaries
string dicNoteEng[] = {"M:","T:","F:","E:","W:","A:","P.O."};
int lenNoteEng = 7;

string familyNameEng[] = {"Chen"};
int lenFamNames = 1;
string givenNameEng[] = {"Reggie"};
int lenSurNames = 1;
string careerEng[] = {"Manager"};
int lenCareers = 1;

string dicCnptEng[] = {"PH","MH","AH","PB","MB","AB","WH","WB"};

//generation model
vector<string> states = {"N","J","P","E","W","A"};
Map_State_Probability startProbs;
State_Map_State_Probability transProbs;

map<char,set<char> > genChar;
map<string,string> genStr;
map<string,string> genNc;

//useful data structures
map<string,set<string> > genWord;

double min(double x, double y, double z) {
	return x < y ? min(x, z) : min(y, z);
}

bool isSign(char c){
	if(c==':'||c==';'||c=='.')
		return true;
	return false;
}

double editDist(char c1, char c2){
	if(c1==':'&&c2==';')
		return 0.1;
	if(c2==':'&&c1==';')
		return 0.1;
	if(c2=='P'&&c1=='R')
		return 0.1;
	if(c2=='R'&&c1=='P')
		return 0.1;
	return 1;
}

//edit distance
double editDist(string A,string B){
	size_t NA = A.size();
	size_t NB = B.size();

	vector<vector<double>> M(NA + 1, vector<double>(NB + 1));

	for (size_t a = 0; a <= NA; ++a)
		M[a][0] = (int)a;

	for (size_t b = 0; b <= NB; ++b)
		M[0][b] = (int)b;

	for (size_t a = 1; a <= NA; ++a)
		for (size_t b = 1; b <= NB; ++b)
		{
			double x = M[a-1][b] + (isSign(A[a-1])?0.7:1);
			double y = M[a][b-1] + (isSign(B[b-1])?0.7:1);
			double z = M[a-1][b-1] + (A[a-1] == B[b-1] ? 0 : editDist(A[a-1],B[b-1]));
			M[a][b] = min(x,y,z);
		}

	return M[A.size()][B.size()];
}


//mocked probability models
double charProb(char ch1, char ch2, int size){
	return 1.0/size;
}

double wordProb(string word1, string word2){
	return 30 - editDist(word1,word2);
}

double strProb(string type, string str){
//	emissProbs[type][str] = 1;
	//...
}

double ncProb(string preType, string type){

}

map<string, map<string,double> > veryNaiveProb;

double conditionProb(string state, vector<string> ob){
	double rst = 0.05;//smoothing factor
	for(int i=0;i<ob.size();i++){
		string feat = ob[i];
		rst+=veryNaiveProb[feat][state];
	}
	return rst;
}

vector<string> markovGetStates(vector<vector<string> > obs){
	Tracking track;
	/*
	 * const Seq_States &obsLabels,
	   const Seq_States &states,
	   Map_State_Probability &start_p,
	   State_Map_State_Probability &trans_p,
	   State_Map_State_Probability &emit_p,
	   Tracking &final_tracker
	 */

	Seq_States states;
	states.push_back("<NOISE>");
	states.push_back("<MTF>");
	states.push_back("<EML>");
	states.push_back("<URL>");
	states.push_back("<ADS>");
	states.push_back("<BOX>");
	states.push_back("<NAME>");
	states.push_back("<TITLE>");

	//priorier probability
	Map_State_Probability startProbs;
	startProbs["<NOISE>"]=0.1;
	startProbs["<MTF>"]=0.2;
	startProbs["<EML>"]=0.1;
	startProbs["<URL>"]=0.1;
	startProbs["<ADS>"]=0.2;
	startProbs["<BOX>"]=0.1;
	startProbs["<NAME>"]=0.1;
	startProbs["<TITLE>"]=0.1;

	Seq_States obsLabels;
	State_Map_State_Probability emissProbs;
	for(int i=0;i<obs.size();i++){
		string ob = to_string(i);
		obsLabels.push_back(ob);

		//Equally treat all observations
		for(int j=0;j<states.size();j++)
			emissProbs[states[j]][ob]=conditionProb(states[j],obs[i]);
	}

	forward_viterbi(obsLabels,
				  states,
				  startProbs,
				  transProbs,
				  emissProbs,
				  track);

	cout<<"structure of name card: "<<track.v_prob<<endl;
	for(int i=0;i<track.v_path.size();i++){
		cout<<track.v_path[i]<<endl;
	}
	return track.v_path;
}

//vector<string> markovGetStates(vector<vector<string> > types, vector<vector<double> >  probs){
//
//	return NULL;
//
//}

void setUpDataStructures(){

	startProbs["N"]=1;
	//...

	transProbs["N"]["J"]=0.5;
	//...

	genStr["M:"]="<MTF>";
	genStr["T:"]="<MTF>";
	genStr["F:"]="<MTF>";
	genStr["E:"]="<EML>";
	genStr["W:"]="<URL>";
	genStr["A:"]="<ADS>";
	genStr["P.O."]="<BOX>";

	set<char> s;
	for(int i=0;i<10;i++){
		s.clear();
		s.insert('0'+i);
		genChar['0'+i]=s;
	}

	s.clear();
	s.insert('+');
	genChar['+']=s;

	s.clear();
	s.insert('-');
	genChar['-']=s;

	s.clear();
	s.insert('(');
	genChar['(']=s;

	s.clear();
	s.insert(')');
	genChar[')']=s;

	veryNaiveProb["<MTF>"]["<NOISE>"]=0.01;
	veryNaiveProb["<MTF>"]["<MTF>"]=0.9;
	veryNaiveProb["<MTF>"]["<EML>"]=0.01;
	veryNaiveProb["<MTF>"]["<URL>"]=0.01;
	veryNaiveProb["<MTF>"]["<ADS>"]=0.01;
	veryNaiveProb["<MTF>"]["<BOX>"]=0.01;
	veryNaiveProb["<MTF>"]["<NAME>"]=0.01;
	veryNaiveProb["<MTF>"]["<TITLE>"]=0.01;

	veryNaiveProb["<EML>"]["<NOISE>"]=0.01;
	veryNaiveProb["<EML>"]["<MTF>"]=0.01;
	veryNaiveProb["<EML>"]["<EML>"]=0.9;
	veryNaiveProb["<EML>"]["<URL>"]=0.01;
	veryNaiveProb["<EML>"]["<ADS>"]=0.01;
	veryNaiveProb["<EML>"]["<BOX>"]=0.01;
	veryNaiveProb["<EML>"]["<NAME>"]=0.01;
	veryNaiveProb["<EML>"]["<TITLE>"]=0.01;

	veryNaiveProb["<URL>"]["<NOISE>"]=0.01;
	veryNaiveProb["<URL>"]["<MTF>"]=0.01;
	veryNaiveProb["<URL>"]["<EML>"]=0.01;
	veryNaiveProb["<URL>"]["<URL>"]=0.9;
	veryNaiveProb["<URL>"]["<ADS>"]=0.01;
	veryNaiveProb["<URL>"]["<BOX>"]=0.01;
	veryNaiveProb["<URL>"]["<NAME>"]=0.01;
	veryNaiveProb["<URL>"]["<TITLE>"]=0.01;

	veryNaiveProb["<ADS>"]["<NOISE>"]=0.01;
	veryNaiveProb["<ADS>"]["<MTF>"]=0.01;
	veryNaiveProb["<ADS>"]["<EML>"]=0.01;
	veryNaiveProb["<ADS>"]["<URL>"]=0.01;
	veryNaiveProb["<ADS>"]["<ADS>"]=0.9;
	veryNaiveProb["<ADS>"]["<BOX>"]=0.01;
	veryNaiveProb["<ADS>"]["<NAME>"]=0.01;
	veryNaiveProb["<ADS>"]["<TITLE>"]=0.01;

	veryNaiveProb["<BOX>"]["<NOISE>"]=0.01;
	veryNaiveProb["<BOX>"]["<MTF>"]=0.01;
	veryNaiveProb["<BOX>"]["<EML>"]=0.01;
	veryNaiveProb["<BOX>"]["<URL>"]=0.01;
	veryNaiveProb["<BOX>"]["<ADS>"]=0.01;
	veryNaiveProb["<BOX>"]["<BOX>"]=1.9;
	veryNaiveProb["<BOX>"]["<NAME>"]=0.01;
	veryNaiveProb["<BOX>"]["<TITLE>"]=0.01;

	veryNaiveProb["<CAREER>"]["<NOISE>"]=0.01;
	veryNaiveProb["<CAREER>"]["<MTF>"]=0.01;
	veryNaiveProb["<CAREER>"]["<EML>"]=0.01;
	veryNaiveProb["<CAREER>"]["<URL>"]=0.01;
	veryNaiveProb["<CAREER>"]["<ADS>"]=0.01;
	veryNaiveProb["<CAREER>"]["<BOX>"]=0.01;
	veryNaiveProb["<CAREER>"]["<NAME>"]=0.01;
	veryNaiveProb["<CAREER>"]["<TITLE>"]=0.9;

	veryNaiveProb["<NUM>"]["<NOISE>"]=0.01;
	veryNaiveProb["<NUM>"]["<MTF>"]=0.3;
	veryNaiveProb["<NUM>"]["<EML>"]=0.01;
	veryNaiveProb["<NUM>"]["<URL>"]=0.01;
	veryNaiveProb["<NUM>"]["<ADS>"]=0.3;
	veryNaiveProb["<NUM>"]["<BOX>"]=0.3;
	veryNaiveProb["<NUM>"]["<NAME>"]=0.01;
	veryNaiveProb["<NUM>"]["<TITLE>"]=0.01;

	veryNaiveProb["<SURNAME>"]["<NOISE>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<MTF>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<EML>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<URL>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<ADS>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<BOX>"]=0.01;
	veryNaiveProb["<SURNAME>"]["<NAME>"]=0.7;
	veryNaiveProb["<SURNAME>"]["<TITLE>"]=0.2;

	veryNaiveProb["<FAMNAME>"]["<NOISE>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<MTF>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<EML>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<URL>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<ADS>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<BOX>"]=0.01;
	veryNaiveProb["<FAMNAME>"]["<NAME>"]=0.7;
	veryNaiveProb["<FAMNAME>"]["<TITLE>"]=0.2;

	veryNaiveProb["<UNKNOWN>"]["<NOISE>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<MTF>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<EML>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<URL>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<ADS>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<BOX>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<NAME>"]=0.1;
	veryNaiveProb["<UNKNOWN>"]["<TITLE>"]=0.1;

	transProbs["<NOISE>"]["<NOISE>"]=0.1;
	transProbs["<NOISE>"]["<MTF>"]=0.1;
	transProbs["<NOISE>"]["<EML>"]=0.1;
	transProbs["<NOISE>"]["<URL>"]=0.1;
	transProbs["<NOISE>"]["<ADS>"]=0.1;
	transProbs["<NOISE>"]["<BOX>"]=0.1;
	transProbs["<NOISE>"]["<NAME>"]=0.1;
	transProbs["<NOISE>"]["<TITLE>"]=0.1;

	transProbs["<MTF>"]["<NOISE>"]=0.01;
	transProbs["<MTF>"]["<MTF>"]=0.3;
	transProbs["<MTF>"]["<EML>"]=0.2;
	transProbs["<MTF>"]["<URL>"]=0.1;
	transProbs["<MTF>"]["<ADS>"]=0.1;
	transProbs["<MTF>"]["<BOX>"]=0.1;
	transProbs["<MTF>"]["<NAME>"]=0.01;
	transProbs["<MTF>"]["<TITLE>"]=0.01;

	transProbs["<EML>"]["<NOISE>"]=0.01;
	transProbs["<EML>"]["<MTF>"]=0.01;
	transProbs["<EML>"]["<EML>"]=0.01;
	transProbs["<EML>"]["<URL>"]=0.3;
	transProbs["<EML>"]["<ADS>"]=0.2;
	transProbs["<EML>"]["<BOX>"]=0.1;
	transProbs["<EML>"]["<NAME>"]=0.01;
	transProbs["<EML>"]["<TITLE>"]=0.01;

	transProbs["<URL>"]["<NOISE>"]=0.01;
	transProbs["<URL>"]["<MTF>"]=0.01;
	transProbs["<URL>"]["<EML>"]=0.3;
	transProbs["<URL>"]["<URL>"]=0.01;
	transProbs["<URL>"]["<ADS>"]=0.3;
	transProbs["<URL>"]["<BOX>"]=0.3;
	transProbs["<URL>"]["<NAME>"]=0.01;
	transProbs["<URL>"]["<TITLE>"]=0.01;

	transProbs["<ADS>"]["<NOISE>"]=0.1;
	transProbs["<ADS>"]["<MTF>"]=0.01;
	transProbs["<ADS>"]["<EML>"]=0.01;
	transProbs["<ADS>"]["<URL>"]=0.01;
	transProbs["<ADS>"]["<ADS>"]=0.4;
	transProbs["<ADS>"]["<BOX>"]=0.4;
	transProbs["<ADS>"]["<NAME>"]=0.01;
	transProbs["<ADS>"]["<TITLE>"]=0.01;

	transProbs["<BOX>"]["<NOISE>"]=0.1;
	transProbs["<BOX>"]["<MTF>"]=0.01;
	transProbs["<BOX>"]["<EML>"]=0.01;
	transProbs["<BOX>"]["<URL>"]=0.01;
	transProbs["<BOX>"]["<ADS>"]=0.01;
	transProbs["<BOX>"]["<BOX>"]=0.01;
	transProbs["<BOX>"]["<NAME>"]=0.01;
	transProbs["<BOX>"]["<TITLE>"]=0.01;

	transProbs["<NAME>"]["<NOISE>"]=0.1;
	transProbs["<NAME>"]["<MTF>"]=0.2;
	transProbs["<NAME>"]["<EML>"]=0.2;
	transProbs["<NAME>"]["<URL>"]=0.01;
	transProbs["<NAME>"]["<ADS>"]=0.1;
	transProbs["<NAME>"]["<BOX>"]=0.01;
	transProbs["<NAME>"]["<NAME>"]=0.01;
	transProbs["<NAME>"]["<TITLE>"]=0.3;

	transProbs["<TITLE>"]["<NOISE>"]=0.1;
	transProbs["<TITLE>"]["<MTF>"]=0.2;
	transProbs["<TITLE>"]["<EML>"]=0.2;
	transProbs["<TITLE>"]["<URL>"]=0.2;
	transProbs["<TITLE>"]["<ADS>"]=0.1;
	transProbs["<TITLE>"]["<BOX>"]=0.01;
	transProbs["<TITLE>"]["<NAME>"]=0.01;
	transProbs["<TITLE>"]["<TITLE>"]=0.2;
}

double concatProb(string last, string now){
	if(last=="<MTF>"&&now=="<NUM>")
		return 1;
	if(last=="<NUM>"&&now=="<NUM>")
		return 0.7;
	if(last=="<UNKNOWN>"||now=="<UNKNOWN>")
		return 0.6;
	if(last=="<SURNAME>"&&now=="<FAMNAME>")
		return 0.9;
	if(last=="<FAMNAME>"&&now=="<SURNAME>")
		return 0.9;
	return 0;
}

//Transform a graph to lists of paths!
double MERGETHRESH = 0.5;
void dfsMerge(int idx, double lastProb, vector<vector<string> > &concepts, vector<vector<double> > &wordProbs,
		vector<double> &probs, vector<int> &history, vector<vector<string> > &corrects){

	if(idx == concepts.size()){

		probs.push_back(lastProb);
		vector<string> correctLine;
		for(int i=0;i<history.size();i++)
			correctLine.push_back(concepts[i][history[i]]);
		corrects.push_back(correctLine);
	}
	else{
		if(idx==0)
			history.clear();

		for(int i=0;i<concepts[idx].size();i++){
			string concept = concepts[idx][i];
			double possible = history.empty()?1:concatProb(concepts[idx-1][history[idx-1]],concept);//concatProb should consider the total probability!
			if(possible>=MERGETHRESH){
				history.push_back(i);
				dfsMerge(idx+1,possible, concepts,wordProbs, probs,history,corrects);
				history.pop_back();
			}
		}
	}
}

void matchNote(string word, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	for(int i=0;i<lenNoteEng;i++){
		string note = dicNoteEng[i];
		double dist = editDist(note,word);
		double prob = 1.0-dist/max(word.size(),note.size());
//		cout<<"Distance: "<<note<<" "<<word<<" "<<dist<<" "<<prob<<endl;
		if(prob>0.5){
			reals.push_back(note);
			probs.push_back(prob);
			cnpts.push_back(genStr[note]);
			cout<<"MATCHED: "<<genStr[note]<<" "<<prob<<endl;
		}
	}
}

void genName(string word, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	for(int i=0;i<lenFamNames;i++){
		string famName = familyNameEng[i];
		double dist = editDist(famName,word);
		double prob = 1.0-dist/max(word.size(),famName.size());
		if(prob>0.3){
			reals.push_back(famName);
			probs.push_back(prob);
			cnpts.push_back("<FAMNAME>");
			cout<<"MATCHED: <FAMNAME> "<<prob<<endl;
		}
	}

	for(int i=0;i<lenSurNames;i++){
		string surName = givenNameEng[i];
		double dist = editDist(surName,word);
		double prob = 1.0-dist/max(word.size(),surName.size());
		if(prob>0.3){
			reals.push_back(surName);
			probs.push_back(prob);
			cnpts.push_back("<SURNAME>");
			cout<<"MATCHED: <SURNAME> "<<prob<<endl;
		}
	}
}

void genCareer(string word, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	for(int i=0;i<lenCareers;i++){
		string career = careerEng[i];
		double dist = editDist(career,word);
		double prob = 1.0-dist/max(word.size(),career.size());
		if(prob>0.3){
			reals.push_back(career);
			probs.push_back(prob);
			cnpts.push_back("<CAREER>");
			cout<<"MATCHED: <CAREER> "<<prob<<endl;
		}
	}
}

double digiProb(char d, char c){
	if(d==c)
		return 1.0;
	return 1.0;
}

double isDigi(char c){
	//TODO use regex to match
	if(c>='0'&&c<='9')
		return true;
	if(c=='('||c==')'||c=='+'||c=='-')
		return true;
	return false;
}

double DIGITHRESH = 0.5;

//TODO speed up with DP
void dfsNumGen(string word, int idx,vector<char> &history, double lastProb, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	if(idx==word.length()){
		string s = "";
		for(int i=0;i<idx;i++){
			s+=history[i];
		}
		reals.push_back(s);
		probs.push_back(lastProb);
		cnpts.push_back("<NUM>");
		cout<<"MATCHED: <NUM> "<<lastProb<<endl;
	}
	else{
		char c = word[idx];
		if(genChar.find(c)!=genChar.end()){
			set<char> digiCands = genChar[c];
			for(set<char>::iterator itor = digiCands.begin();itor!=digiCands.end();itor++){
				char cand = (*itor);
				double prob = digiProb(c,cand);//TODO look how many digits changed
				if(prob>=DIGITHRESH){
					history.push_back(cand);
					dfsNumGen(word,idx+1,history,prob,cnpts, reals,probs);
					history.pop_back();
				}
			}
		}
	}
}

void genNum(string word, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	vector<char> history;
	dfsNumGen(word, 0,history,1,cnpts,reals,probs);

}

map<string, double> cnptProb;

void matchCnpt(string word, vector<string> &cnpts, vector<string> &reals, vector<double> &probs){

	genName(word,cnpts,reals,probs);
	genCareer(word,cnpts,reals,probs);
	genNum(word,cnpts,reals,probs);

	for(int i=0;i<cnpts.size();i++){
		string cnpt = cnpts[i];
		if(cnptProb.find(cnpt)==cnptProb.end())
			cnptProb[cnpt]=probs[i];
		else
			cnptProb[cnpt]+=probs[i];
	}
}

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

void correct(string line, vector<vector<string> > &corrects, vector<double> &probs){
	vector<string> words = split(line,2);
	vector<vector<string> > correctList1;//1: real, 2: abstract/concept
	vector<vector<string> > correctList2;
	vector<vector<double> > wordProbs;

	for(int i=0;i<words.size();i++){
		string word = words[i];
		cout<<"[correting word]:: "<<word<<endl;

		vector<string> matched1;
		vector<string> matched2;
		vector<double> probs;

		matchNote(word,matched1,matched2,probs);
		matchCnpt(word,matched1,matched2,probs);

		if(matched1.empty()&&matched2.empty()){
			cout<<"MATCHED NOTHING"<<endl;
			matched2.push_back(word);
			matched1.push_back("<UNKNOWN>");
			probs.push_back(1);
		}

		correctList1.push_back(matched1);
		correctList2.push_back(matched2);
		wordProbs.push_back(probs);
	}

	vector<int> history;

	dfsMerge(0,1,correctList1,wordProbs,probs, history,corrects);
	cout<<"CANDIDATE CORRECTS OF LINE!"<<endl;
	for(int i=0;i<corrects.size();i++){
		vector<string> correctStrs = corrects[i];
		for(int j=0;j<correctStrs.size();j++)
			cout<<correctStrs[j];
		cout<<endl;
	}
}

void clearDataStructures(){}

vector<vector<string> > findMaxProbCorretion(vector<vector<vector<string> > > obvCorrects,vector<vector<double> > obvProbs){

	vector<vector<string> > rst;
	for(int i=0;i<obvCorrects.size();i++){
		double maxProb = -10;
		vector<string> maxStr;

		for(int j=0;j<obvProbs[i].size();j++){
			if(obvProbs[i][j]>maxProb){
				maxProb = obvProbs[i][j];
				maxStr = obvCorrects[i][j];
			}
		}

		rst.push_back(maxStr);
	}
	return rst;
}

//main flow
void correctNameCard(string input){
	setUpDataStructures();
	vector<string> lines = split(input,1);
/**/
	vector<vector<string> > obvTypes;
	vector<vector<vector<string> > > obvCorrects;
	vector<vector<double> > obvProbs;

	for(int i=0;i<lines.size();i++){
		string line = lines[i];
		cout<<"[correting line]:: "<<line<<endl;
//		vector<string> myTypes;
		vector<vector<string> > corrects;
		vector<double> probs;

//		correct(line, myTypes, corrects, probs);
		correct(line, corrects, probs);
//		obvTypes.push_back(myTypes);
		obvCorrects.push_back(corrects);
		obvProbs.push_back(probs);
		cout<<endl;
	}

	vector<vector<string> > observes = findMaxProbCorretion(obvCorrects,obvProbs);
	vector<string> states = markovGetStates(observes);

	clearDataStructures();

}

#endif /* NAMECARDPOST_H_ */
