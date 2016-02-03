/*
 * Provide specific or general functionalities for removing all kinds of lines
 * in images
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 */

#ifndef OCRUS_PREPROCESSING_DENOISE_REMOVE_LINE_H_
#define OCRUS_PREPROCESSING_DENOISE_REMOVE_LINE_H_

#include <opencv2/opencv.hpp>

namespace ocrus {

/**
 * A specific function for removing red lines in the 406.zip images
 *
 * @param img: The image to be processed
 */
void removeRedLineFor406(cv::Mat& img);

}  // namespace ocrus

#endif /* OCRUS_PREPROCESSING_DENOISE_REMOVE_LINE_H_ */
