/*
 * Binarization of image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#ifndef OCRUS_BINARIZATION_WAP_BINARIZE_H_
#define OCRUS_BINARIZATION_WAP_BINARIZE_H_

#include <string>

#include <opencv2/opencv.hpp>
#include "dto/ocr_result_dto.h"

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
void NiblackSauvolaWolfJolion(cv::Mat im, cv::Mat output, NiblackVersion version,
                              int winx, int winy, double k, double dR);

/**
 * Binarize an image
 *
 * @param src: Image to binarize
 * @param dst: The binarized image
 */
void binarize(const cv::Mat& src, cv::Mat& dst, double k =0.1);

}  // namespace ocrus

#endif  // OCRUS_BINARIZATION_WAP_BINARIZE_H_
