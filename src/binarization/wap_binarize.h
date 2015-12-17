/*
 * Binarization of image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#ifndef OCRUS_BINARIZATION_H_
#define OCRUS_BINARIZATION_H_

#include <string>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "dto/ocr_result_dto.h"

namespace ocrus {

enum NiblackVersion {
  NIBLACK = 0,
  SAUVOLA,
  WOLFJOLION,
};

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;

// *************************************************************
// glide a window across the image and
// create two maps: mean and standard deviation.
//
// Version patched by Thibault Yohan (using opencv integral images)
// *************************************************************
double calcLocalStats(Mat &im, Mat &map_m, Mat &map_s, int winx, int winy);

/**********************************************************
 * The binarization routine
 **********************************************************/
void NiblackSauvolaWolfJolion(Mat im, Mat output, NiblackVersion version,
                              int winx, int winy, double k, double dR);

void binarize(Mat& src, Mat& dst);

}  // namespace ocrus

#endif  // OCRUS_BINARIZATION_H_
