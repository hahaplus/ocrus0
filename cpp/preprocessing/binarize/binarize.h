/*
 * Binarization of image
 *
 * Copyright (C) 2016 Works Applications, all rights reserved
 */

#ifndef OCRUS_PREPROCESSING_BINARIZE_H_
#define OCRUS_PREPROCESSING_BINARIZE_H_

#include <string>

#include <opencv2/opencv.hpp>

namespace ocrus {

enum NiblackVersion {
  NIBLACK = 0,
  SAUVOLA,
  WOLFJOLION,
};

/**
 * The Niblack, Sauvola, and WolfJolion binarization algorithms
 *
 * @param im: Image to binarize
 * @param Mat: The binarized image
 * @param version: Indicate which algorithm to use
 * @param winx: Width of window
 * @param winy: Width of window
 * @param k: Algorithm parameter
 * @param dR: Algorithm parameter
 *
 * Adapted from version 2.4 in
 * http://liris.cnrs.fr/christian.wolf/software/binarize/index.html
 */
void NiblackSauvolaWolfJolion(cv::Mat im, cv::Mat output,
                              NiblackVersion version, int winx, int winy,
                              double k, double dR);

/**
 * Binarize an image by WolfJolion algorithm
 *
 * @param img_gray: A gray image to binarize
 * @param img_binary: The binarized image
 * @param k: The WolfJolion algorithm parameter, smaller k gets more black
 */
void binarize(const cv::Mat& img_gray, cv::Mat& img_binary, double k = 0.1);

}  // namespace ocrus

#endif  // OCRUS_PREPROCESSING_BINARIZE_H_
