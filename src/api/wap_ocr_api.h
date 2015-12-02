/*
 * WapOcrApi.h
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#ifndef SRC_API_WAP_OCR_API_H_
#define SRC_API_WAP_OCR_API_H_

#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "../dto/ocr_result_dto.h"

class WapOcrApi {
public:
	WapOcrApi();
	static std::string recognitionToText(cv::Mat &img, const std::string lang = "eng+jpn+chi_sim", const int cutLevel = 0, OcrDetailResult* result = NULL);
	virtual ~WapOcrApi();
private:
	// merge and split
	static void optimize(OcrDetailResult*);
	static void mergeAndSplit(std::vector<ResultUnit>&);
	static void mapToLine(std::vector<ResultUnit> &symbols);
	static bool overlap(std::pair<int, int> a, std::pair<int, int> b);
	static void handle(std::vector<ResultUnit> &segment);
};

#endif /* SRC_API_WAP_OCR_API_H_ */
