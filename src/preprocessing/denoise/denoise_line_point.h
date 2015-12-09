/*
 * denoise_line_point.h
 *
 *  Created on: Dec 9, 2015
 *      Author: michael
 */

#ifndef SRC_PREPROCESSING_DENOISE_DENOISE_LINE_POINT_H_
#define SRC_PREPROCESSING_DENOISE_DENOISE_LINE_POINT_H_
#include <opencv2/opencv.hpp>
#include "util/algorithm_util.h"
class DenoiseLinePoint {
 public:
  DenoiseLinePoint();
  virtual ~DenoiseLinePoint();
  /*
   * @para src is the binary image
   */
  static void removeNoise(cv::Mat &src);
};

#endif /* SRC_PREPROCESSING_DENOISE_DENOISE_LINE_POINT_H_ */
