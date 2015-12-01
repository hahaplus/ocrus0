/*
 * WapOcrApi.h
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#ifndef SRC_API_WAPOCRAPI_H_
#define SRC_API_WAPOCRAPI_H_

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include "../dto/OcrResultDto.h"
using namespace std;
using namespace cv;




class WapOcrApi {
public:
	WapOcrApi();
	static string recognitionToText(cv::Mat &img, const string lang = "eng+jpn+chi_sim", const int cutLevel = 0, OcrDetailResult* result = NULL);
	virtual ~WapOcrApi();
private:
	// merge and split
	static void optimize(OcrDetailResult*);
	static void mergeAndSplit(vector<ResultUnit>&);
	static void mapToLine(vector<ResultUnit> &symbols);
	static bool overlap(pair<int, int> a, pair<int, int> b);
	static void handle(vector<ResultUnit> &segment);
};

#endif /* SRC_API_WAPOCRAPI_H_ */
