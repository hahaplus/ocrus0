/*
 * RecognitionByCNN.h
 *
 *  Created on: Jan 22, 2016
 *      Author: michael
 */

#ifndef SRC_RECOGNITION_RECOGNITION_BY_CNN_H_
#define SRC_RECOGNITION_RECOGNITION_BY_CNN_H_
#include <vector>
#include <opencv2/opencv.hpp>
#include "dto/ocr_result_dto.h"
#include <python2.7/Python.h>
class RecognitionByCNN {
 public:
  RecognitionByCNN();
  // recognition with conventional network
   // param: img is a matrix of a single character
   // output-param: result is the recognition result of the single character
  static void recognition(const cv::Mat &img, ResultUnit &result);
  // param: img_list is a list of many single characters
  // output-param: result is the recognition result of the character list
  static void recognition(const std::vector<cv::Mat> &img_list,
                          std::vector<ResultUnit> &result);
  virtual ~RecognitionByCNN();
 private:
  static void init();
  static void release();
  // load dict
  static void loadDict();
  static vector<string> dict;
  // dict path
  static string dict_path;
  // for the deep learning network
  static PyObject* pMod;
  static PyObject* single_img_func;
  static PyObject* multi_img_func;
  static PyObject* pDict;
};

#endif /* SRC_RECOGNITION_RECOGNITION_BY_CNN_H_ */
