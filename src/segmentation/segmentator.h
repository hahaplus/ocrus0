/*
 * Segmentator.h
 *
 *  Created on: Jan 20, 2016
 *      Author: michael
 */

#ifndef SRC_SEGMENTATION_SEGMENTATOR_H_
#define SRC_SEGMENTATION_SEGMENTATOR_H_
#include "dto/ocr_result_dto.h"

#include <set>
class Segmentator {
 public:
  Segmentator();
  static void segmentImg(cv::Mat &image, OcrDetailResult* result);
  virtual ~Segmentator();
 private:
  static vector<ResultUnit> result;
  static vector<vector<vector<int> > > result_map;   // map the (row, start, end) -> index in result
  static vector<vector<vector<float> > > confidence;
  static void getRow(cv::Mat &image, OcrDetailResult* result);
  static void filterBigBox(cv::Mat &img, OcrDetailResult* seg_result);
  static void mergeSmallBox(cv::Mat &img, OcrDetailResult* seg_result);
  static void handleEachRowInit(cv::Mat&, map<int, vector<ResultUnit> > &);
  static void handleEachRow(cv::Mat &img, int, vector<ResultUnit>&, vector<vector<float> > &);
  static void getAllTheSplitIndex(set<int>&, int, int, vector<vector<int>> &);
  /*
   * return the similarity between the img with the prototype image of the character
   */
  static double getSimilarity(cv::Mat& img, string charater);
};

#endif /* SRC_SEGMENTATION_SEGMENTATOR_H_ */
