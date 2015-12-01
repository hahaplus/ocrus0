/*
 * OcrResultDto.h
 *
 *  Created on: Dec 1, 2015
 *      Author: michael
 */

#ifndef SRC_DTO_OCRRESULTDTO_H_
#define SRC_DTO_OCRRESULTDTO_H_
#include <string>
#include <vector>
using namespace std;
using namespace cv;
class ResultUnit {
public:
	vector<cv::Point2i> boundingBox;  // every symbol's boundingBox
	string content;                   // symbol's content
	vector<pair<string,float> > candidates; // other choices for this symbol (content, possibility)
    int lineIndex;
	ResultUnit()
	{
		lineIndex = 0;
	}
	ResultUnit(vector<Point2i> &bbox, string content)
	{
    	this->boundingBox = bbox;
    	this->content = content;
    	this->lineIndex = 0;
	}
};
class OcrDetailResult {
private:
    vector<ResultUnit> symbols;
public:
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
    vector<ResultUnit> getResult()
	{
    	return symbols;
	}
    string toString()
    {
    	return "";
    }
};
//compare as x
class CompX
{
public:
    bool operator ()(const ResultUnit& a, const ResultUnit& b)
    {
      return a.boundingBox[0].x < b.boundingBox[0].x;
    }
};
// compare as y
class CompY
{
public:
    bool operator ()(const ResultUnit& a, const ResultUnit& b)
    {
      return a.boundingBox[0].y < b.boundingBox[0].y;
    }
};


#endif /* SRC_DTO_OCRRESULTDTO_H_ */
