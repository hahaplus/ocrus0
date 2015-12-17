/*
 * SimpleTextDetect.h
 *
 *  Created on: Dec 17, 2015
 *      Author: michael
 */

#ifndef SRC_TEXTDETECT_SIMPLE_TEXT_DETECT_H_
#define SRC_TEXTDETECT_SIMPLE_TEXT_DETECT_H_
#include <opencv2/opencv.hpp>
class SimpleTextDetect {
 public:
  SimpleTextDetect();
  virtual ~SimpleTextDetect();
  // return one rectangle area of the main part text
  static cv::Rect simpleDetect(cv::Mat);
};

#endif /* SRC_TEXTDETECT_SIMPLE_TEXT_DETECT_H_ */
