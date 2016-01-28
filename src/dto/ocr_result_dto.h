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
#include <opencv2/opencv.hpp>
#include "util/string_util.h"
using namespace std;

class ResultUnit {
 public:
  vector<cv::Point2i> bounding_box;  // every symbol's bounding_box
  string content;                   // symbol's content
  vector<pair<string, float> > candidates;  // other choices for this symbol (content, possibility)
  float confidence;
  int line_index;
  int property;  // can be any property you want to store
  ResultUnit() {
    line_index = 0;
    confidence = 0;
    bounding_box.resize(2);
    bounding_box[0] = cv::Point2i(1e6, 1e6);
    bounding_box[1] = cv::Point2i(-1e6, -1e6);
  }
  ResultUnit(vector<cv::Point2i> &bbox, string content,
             vector<pair<string, float> > candidates, float confi,
             int line_index) {
    this->bounding_box = bbox;
    this->content = content;
    this->candidates = candidates;
    this->confidence = confi;
    this->line_index = line_index;
  }

  ResultUnit(vector<cv::Point2i> &bbox, string content) {
    this->bounding_box = bbox;
    this->content = content;
    this->line_index = 0;
    this->confidence = 0;
  }
  int getWidth() {
    return bounding_box[1].x - bounding_box[0].x + 1;
  }
  int getHeight() {
    return bounding_box[1].y - bounding_box[0].y + 1;
  }
  static bool isOverlap(ResultUnit a, ResultUnit b) {
    return isIntervalOverlap(
        make_pair(a.bounding_box[0].x, a.bounding_box[1].x),
        make_pair(b.bounding_box[0].x, b.bounding_box[1].x))
        && isIntervalOverlap(
            make_pair(a.bounding_box[0].y, a.bounding_box[1].y),
            make_pair(b.bounding_box[0].y, b.bounding_box[1].y));
  }
  static bool isIntervalOverlap(pair<int, int> a, pair<int, int> b) {
    if (a.first > b.first) {
      swap(a, b);
    }
    return b.first <= a.second;
  }
  static double getOverlapArea(ResultUnit a, ResultUnit b)
  {
     pair<int, int> ax(a.bounding_box[0].x, a.bounding_box[1].x);
     pair<int, int> ay(a.bounding_box[0].y, a.bounding_box[1].y);
     pair<int, int> bx(b.bounding_box[0].x, b.bounding_box[1].x);
     pair<int, int> by(b.bounding_box[0].y, b.bounding_box[1].y);

     pair<int, int> overlapx, overlapy;
     overlapx.first = max(ax.first, bx.first);
     overlapx.second = min(ax.second, bx.second);

     overlapy.first = max(ay.first, by.first);
     overlapy.second = min(ay.second, by.second);

     if (overlapx.first > overlapx.second || overlapy.first > overlapy.second)
       return 0;
     return (overlapx.second - overlapx.first + 1)*(overlapy.second - overlapy.first + 1);
  }
  static ResultUnit getMergeUnit(ResultUnit a, ResultUnit b)
  {
     vector<ResultUnit> unit_list;
     unit_list.push_back(a), unit_list.push_back(b);
     return getMergeUnit(unit_list);
  }
  static ResultUnit getMergeUnit(vector<ResultUnit> &segment_list)
  {
      ResultUnit res;
      for (auto ru : segment_list)
      {
          res.bounding_box[0].x = min(res.bounding_box[0].x, ru.bounding_box[0].x);
          res.bounding_box[0].y = min(res.bounding_box[0].y, ru.bounding_box[0].y);
          res.bounding_box[1].x = max(res.bounding_box[1].x, ru.bounding_box[1].x);
          res.bounding_box[1].y = max(res.bounding_box[1].y, ru.bounding_box[1].y);
      }
      return res;
  }
};
class OcrDetailResult {
 private:
  vector<ResultUnit> symbols;
 public:
  const static int SYMBOL;
  const static int TEXT;
  void setResult(vector<ResultUnit> symbols) {
    this->symbols.resize(symbols.size());
    this->symbols = symbols;
  }
  void clear() {
    symbols.clear();
  }
  void push_back_symbol(vector<ResultUnit> &a) {
    for (int i = 0; i < a.size(); i++)
      symbols.push_back(a[i]);
  }
  void push_back_symbol(ResultUnit &a) {
    symbols.push_back(a);
  }
  vector<ResultUnit> getResult() const {
    return symbols;
  }
  ResultUnit getResultAt(int index) {
    assert(index >= 0 && index < symbols.size());
    return symbols[index];
  }
  void setResultAt(int index, ResultUnit &ru){
    symbols[index] = ru;
  }
  int getResultSize() {
    return symbols.size();
  }
  string toString(int cut_level = 0) {
    string res;
    switch (cut_level) {
      case 0:
        for (int i = 0; i < symbols.size(); i++) {
          res += "(" + StringUtil::toString(symbols[i].line_index) + " "
              + StringUtil::toString(i) + ") ";
          res += symbols[i].content + " "
              + StringUtil::toString(symbols[i].confidence) + " ";
          vector<pair<string, float> > candidates = symbols[i].candidates;
          for (int j = 0; j < candidates.size(); j++) {
            res += candidates[j].first + " "
                + StringUtil::toString(candidates[j].second) + " ";
          }
          res += "\n";
        }
        break;
      case 1:
        break;
    }
    return res;
  }
  static void mergeOcrResult(cv::Mat &main_img, cv::Mat assit_img,
                             OcrDetailResult* main_result,
                             OcrDetailResult* assit_result);
};

//compare as x
class CompX {
 public:
  bool operator ()(const ResultUnit& a, const ResultUnit& b) {
    return a.bounding_box[0].x < b.bounding_box[0].x;
  }
};
// compare as y
class CompY {
 public:
  bool operator ()(const ResultUnit& a, const ResultUnit& b) {
    return a.bounding_box[0].y < b.bounding_box[0].y;
  }

};

#endif /* SRC_DTO_OCR_RESULT_DTO_H_ */
