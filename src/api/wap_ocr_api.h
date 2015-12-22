/*
 * wap_ocr_api.h
 *
 *  Created on: Nov 30, 2015
 *      Author: michael chen
 */

#ifndef SRC_API_WAP_OCR_API_H_
#define SRC_API_WAP_OCR_API_H_

#include <tesseract/baseapi.h>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "dto/ocr_result_dto.h"
class WapOcrApi {
public:
	WapOcrApi();
	static std::string recognitionToText(const cv::Mat &img, const std::string lang = "eng+jpn+chi_sim", const int cutLevel = 0, OcrDetailResult* result = NULL);
	static void release();
private:
	/*
	 * overlap ratio (0.0 ~ 1.0)to be judged as overlap
	 * */
	static double epsX, epsY;
	static tesseract::TessBaseAPI *api;
	//static cv::Mat img;
	// merge and split
	static void getBBox(const cv::Mat &img, OcrDetailResult* odr); // get The bounding box of the character
	static void optimize(OcrDetailResult*);
	static void mergeAndSplit(vector<ResultUnit>&);
	static void mapToLine(vector<ResultUnit> &symbols);
	// judge if interval a overlap with interval b, eps means if the distance between a and b is less or equal than eps, i will be judged overlap
	static bool overlap(pair<int, int> a, pair<int, int> b, int eps = 0);
	static void handle(vector<ResultUnit> &segment);
	static void recognizeUnit(ResultUnit &u);
	static void cutImage(const cv::Mat &src, cv::Mat &dst, int x, int y, int width, int height);
	static void scaleImage(cv::Mat &src, double new_width, double new_height);
	// format the image to be more neat
	static void formatImage(const cv::Mat &src, cv::Mat &dst, OcrDetailResult *result, std::map<pair<int,int>, ResultUnit> &pos_map);
  // write the character from src to dst at (x, y)
	static void writeCharacter(const cv::Mat &src, cv::Mat &dst, ResultUnit unit, int x, int y);
	// recognition with tesseract
  static void recognitionWithTesseract(const cv::Mat &img, const std::string lang = "eng+jpn+chi_sim", const int cutLevel = 0, OcrDetailResult* result = NULL);

};

#endif /* SRC_API_WAP_OCR_API_H_ */
