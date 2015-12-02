/*
 * WapOcrApi.cpp
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#include "wap_ocr_api.h"

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "../util/string_util.h"

using namespace std;
using namespace tesseract;
using namespace cv;
WapOcrApi::WapOcrApi() {
	// TODO Auto-generated constructor stub
}

string WapOcrApi::recognitionToText(cv::Mat &src,const string lang, int cutLevel,OcrDetailResult* result)
{
	 if ( result == NULL )
	 {
		 result = new OcrDetailResult;
	 }
	 tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	 api->Init(NULL, lang.c_str());
	 api->SetVariable("save_blob_choices", "T");
	 //api->SetImage(image);
	 api->SetImage((uchar*) src.data, src.cols, src.rows, src.channels(), src.cols);
	 api->Recognize(0);
	 tesseract::ResultIterator* ri = api->GetIterator();
	 tesseract::PageIteratorLevel level = (cutLevel == 0) ? tesseract::RIL_SYMBOL : tesseract::RIL_TEXTLINE;
	 string res;
	 int cnt = 0, lineCnt = -1;
	 stringstream ss;
	 string strCnt;
	 CvFont font;
	 double hScale=1;
	 double vScale=1;
	 int lineWidth=2;// 相当于写字的线条
	 cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
	 if (ri != 0) {
	 do {
	      const char* word = ri->GetUTF8Text(level);
	      float conf = ri->Confidence(level);
	      int x1, y1, x2, y2;
	      ri->BoundingBox(level, &x1, &y1, &x2, &y2);
	      if (ri->IsAtBeginningOf(tesseract::RIL_TEXTLINE))
	    	  lineCnt++;
		      strCnt = StringUtil::toString(cnt++);
		      if (cutLevel == 0)
		      {
		    	  res += "("+StringUtil::toString(lineCnt)+" ";
		    	  res += strCnt + ") ";
		      }
		      res += word;
		      if (cutLevel == 0)
		      {
		    	  res += "：";
		      }
		      vector<cv::Point2i> corners;
		      // draw the boudingbox
		      corners.push_back(cv::Point2i(x1, y1));
          corners.push_back(cv::Point2i(x2, y2));

		      // the other choices of the character
		      tesseract::ChoiceIterator choiceIterator( *ri );
              // save the result
		      ResultUnit rt(corners, string(word));
		      rt.lineIndex = lineCnt;
		      rt.confidence = conf;
		      if (cutLevel == 0)
		      do
              {
            	  const char* txt = choiceIterator.GetUTF8Text();
                  if (txt != NULL && strlen(txt) != 0)
            	  {http://stackoverflow.com/questions/23740431/tesseract-remove-reference-ambiguous-symbol-in-project-on-visual-studio-2012
                	  res += txt;
                	  res += " ";
            	      res += StringUtil::toString(choiceIterator.Confidence()) + " ";
            	      rt.candidates.push_back(make_pair(txt, choiceIterator.Confidence()));
            	  }
                  //delete[] txt;
              }while(choiceIterator.Next());
		      res += "\n";
              delete[] word;
              result->push_back_symbol(rt);
		 } while (ri->Next(level));
        //optimize(result);
        //res = result->toString();
	 }
	 return res;
}
bool WapOcrApi::overlap(pair<int, int> a, pair<int, int> b)
{
	if (a.first > b.first)
	{
		swap(a, b);
	}
	return b.first <= a.second;
}
void WapOcrApi::mapToLine(vector<ResultUnit> &symbols)
{
	sort(symbols.begin(), symbols.end(),CompY());
	vector<pair<int,int> > intervals;
	for (int i = 0; i < symbols.size();i++)
	{
		ResultUnit &rt = symbols[ i ];
		pair<int,int> interval = make_pair(rt.boundingBox[0].y, rt.boundingBox[1].y);
		if (!intervals.empty() && overlap(interval, intervals.back()))
		{
			// update
			intervals.back().second = max(intervals.back().second, interval.second);
		}
		else
		{
			intervals.push_back(interval);
		}
		rt.lineIndex = intervals.size() - 1;
	}
}
void WapOcrApi::optimize(OcrDetailResult* result)
{
	if (result == NULL)
		return;
	vector<ResultUnit> symbols = result->getResult();
	mapToLine(symbols);
	vector<ResultUnit> line;
	result->clear();
	for (int i = 0;i < symbols.size();i++)
	{
		line.push_back(symbols[i]);
		if (i == symbols.size() - 1 || symbols[i].lineIndex != symbols[i+1].lineIndex)
		{
			mergeAndSplit(line);
			result->push_back_symbol(line);
			line.clear();
		}
	}
}
void WapOcrApi::mergeAndSplit(vector<ResultUnit> &line)
{
	// try to merge
	sort(line.begin(), line.end(), CompX());
	pair<int, int> intervalBack(-1,-1);
	vector<ResultUnit> segment;
	vector<ResultUnit> newLine;
	for (int i = 0; i < line.size(); i++)
	{
		ResultUnit &rt = line[i];
		pair<int,int> interval = make_pair(rt.boundingBox[0].x, rt.boundingBox[1].x);
		if (intervalBack.first == -1)
			intervalBack = interval;
		else
			intervalBack.second = max(intervalBack.second, interval.second);
		pair<int,int> nextInterval(1e6, 1e6);
		segment.push_back(line[i]);
		if (i != line.size() - 1)
		{
			nextInterval = make_pair(line[i+1].boundingBox[0].x, line[i+1].boundingBox[1].x);
		}
		if (!overlap(nextInterval, intervalBack))
		{
			intervalBack = make_pair(-1, -1);
			handle(segment);
			for (int j = 0; j < segment.size(); j++)
			{
				newLine.push_back(segment[j]);
			}
			segment.clear();
		}
	}
	line = newLine;
}
void WapOcrApi::handle(vector<ResultUnit> &segment)
{

}
WapOcrApi::~WapOcrApi() {
	// TODO Auto-generated destructor stub

}


