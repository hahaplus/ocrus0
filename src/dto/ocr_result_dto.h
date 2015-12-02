/*
 * ocr_result_dto.h
 *
 *  Created on: Dec 1, 2015
 *      Author: michael
 */

#ifndef SRC_DTO_OCR_RESULT_DTO_H_
#define SRC_DTO_OCR_RESULT_DTO_H_
#include <string>
#include <vector>

#include "../util/string_util.h"
using namespace std;
using namespace cv;
class ResultUnit {
public:
	vector<cv::Point2i> bounding_box;  // every symbol's bounding_box
	string content;                   // symbol's content
	vector<pair<string,float> > candidates; // other choices for this symbol (content, possibility)
    float confidence;
	int line_index;
	ResultUnit()
	{
		line_index = 0;
		confidence = 0;
	}
	ResultUnit(vector<Point2i> &bbox, string content)
	{
    	this->bounding_box = bbox;
    	this->content = content;
    	this->line_index = 0;
    	this->confidence = 0;
	}
};
class OcrDetailResult {
private:
    vector<ResultUnit> symbols;
public:
    const static int SYMBOL;
    const static int TEXT;
    void setResult(vector<ResultUnit> &symbols)
    {
    	this->symbols = symbols;
    }
    void clear()
    {
    	symbols.clear();
    }
    void push_back_symbol(vector<ResultUnit> &a)
    {
    	for(int i = 0; i < a.size();i++)
        	symbols.push_back(a[i]);
    }
    void push_back_symbol(ResultUnit &a)
    {
    	symbols.push_back(a);
    }
    vector<ResultUnit> getResult() const
	{
    	return symbols;
	}
    string toString(int cut_level)
    {
    	string res;
    	switch(cut_level)
    	{
    		case 0:
    			for (int i = 0; i < symbols.size(); i++)
    			{
    				res += "("+StringUtil::toString(symbols[i].line_index)+" "+StringUtil::toString(i)+") ";
    				res += 	symbols[i].content + " " + StringUtil::toString(symbols[i].confidence) + " ";
    				vector<pair<string,float> > candidates = symbols[i].candidates;
    				for (int j = 0; j < candidates.size(); j++)
    				{
    					res += candidates[j].first + " " + StringUtil::toString(candidates[j].second) + " ";
    				}
    				res += "\n";
    			}
    			break;
    		case 1:
    			break;
    	}
    	return res;
    }
};

//compare as x
class CompX
{
public:
    bool operator ()(const ResultUnit& a, const ResultUnit& b)
    {
      return a.bounding_box[0].x < b.bounding_box[0].x;
    }
};
// compare as y
class CompY
{
public:
    bool operator ()(const ResultUnit& a, const ResultUnit& b)
    {
      return a.bounding_box[0].y < b.bounding_box[0].y;
    }
};


#endif /* SRC_DTO_OCR_RESULT_DTO_H_ */