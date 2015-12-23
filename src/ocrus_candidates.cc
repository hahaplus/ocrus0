/*
 * Print the OCR candidates of an image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include <stdio.h>

#include "tesseract/baseapi.h"

#include "binarization/wap_binarize.h"
#include "preprocessing/denoise/denoise_line_point.h"
#include "recognition/recognition.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Print the OCR candidates of an image\n"
           "Gray and binarized images are saved where the image is located\n"
           "Usage: ocrus_bounding_box path_img\n");
    return 0;
  }

  std::string path_img(argv[1]);

  cv::Mat img, gray_img, binarize_img, box_img;

  img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);

  cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

  ocrus::binarize(gray_img, binarize_img);

  // DenoiseLinePoint::removeNoise(binarize_img);

  ocrus::ocrPrintCandidates(binarize_img, "jpn+jpnRSN");

  cv::imwrite(path_img + "_gray.png", gray_img);

  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(path_img + "_binarize.png", binarize_img);

  return 0;
}
