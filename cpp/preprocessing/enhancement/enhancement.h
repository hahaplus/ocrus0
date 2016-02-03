/*
 * Enhancement.h
 *
 *  Created on: Dec 21, 2015
 *      Author: michael
 */

#ifndef SRC_PREPROCESSING_ENHANCEMENT_ENHANCEMENT_H_
#define SRC_PREPROCESSING_ENHANCEMENT_ENHANCEMENT_H_
#include <opencv2/opencv.hpp>
class Enhancement {
 public:
  Enhancement();
  /**
   * input a gray image output a enhanced Binarize image
   * k ranges from [0,1] means the intention of the enhancement
   * the larger k gives a stronger enhancement
   */
  static void enhancementAndBinarize(const cv::Mat &src, cv::Mat &dst,
                                     double k = 0.1, double e = 0.0);
  virtual ~Enhancement();
 private:
  static void imageStretchByHistogram(IplImage *src1,IplImage *dst1);
};

#endif /* SRC_PREPROCESSING_ENHANCEMENT_ENHANCEMENT_H_ */