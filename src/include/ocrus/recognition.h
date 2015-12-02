/*
 * Recognition functionalities
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#ifndef OCRUS_RECOGNITION_H_
#define OCRUS_RECOGNITION_H_

#include <string>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

namespace ocrus {

void ocrPrintBoundingBox(const cv::Mat& src, tesseract::PageIteratorLevel level,
                         const std::string& lang);

}  // namespace ocrus

#endif  // OCRUS_RECOGNITION_H_
