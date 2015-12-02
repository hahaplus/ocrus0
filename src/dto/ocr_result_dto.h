/*
 * OcrResultDto.h
 *
 *  Created on: Dec 1, 2015
 *      Author: michael
 */

#ifndef SRC_DTO_OCR_RESULT_DTO_H_
#define SRC_DTO_OCR_RESULT_DTO_H_

#include <string>
#include <vector>


class ResultUnit {
public:
	std::vector<cv::Point2i> boundingBox;  // every symbol's boundingBox
	std::string content;                   // symbol's content
	std::vector<std::pair<std::string,float> > candidates; // other choices for this symbol (content, possibility)
	float confidence;
    int lineIndex;
	ResultUnit()
	{
		lineIndex = 0;
	}
	ResultUnit(std::vector<cv::Point2i> &bbox, std::string content)
	{
    	this->boundingBox = bbox;
    	this->content = content;
    	this->lineIndex = 0;
	}
};
class OcrDetailResult {
private:
    std::vector<ResultUnit> symbols;
public:
    void setResult(std::vector<ResultUnit> &symbols)
    {
    	this->symbols = symbols;
    }
    void clear()
    {
    	symbols.clear();
    }
    void push_back_symbol(std::vector<ResultUnit> &a)
    {
    	for(int i = 0; i < a.size();i++)
        	symbols.push_back(a[i]);
    }
    void push_back_symbol(ResultUnit &a)
    {
    	symbols.push_back(a);
    }
    std::vector<ResultUnit> getResult() const
	{
    	return symbols;
	}
    std::string toString()
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


#endif /* SRC_DTO_OCR_RESULT_DTO_H_ */
