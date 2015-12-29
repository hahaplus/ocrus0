/*
 * Print the bounding boxes of an image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "binarization/wap_binarize.h"
#include "preprocessing/denoise/denoise_line_point.h"
#include "preprocessing/denoise/remove_line.h"
#include "preprocessing/enhancement/enhancement.h"
#include "recognition/recognition.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Print the bounding boxes of an image\n"
           "Gray and binarized images are saved where the image is located\n"
           "Usage: ocrus_bounding_box page_seg_mode word|symbol path_img\n"
           "page_seg_mode: An enum integer from Tesseract");
    return 0;
  }

  tesseract::PageSegMode page_seg_mode =
      static_cast<tesseract::PageSegMode>(atoi(argv[1]));
  std::string level(argv[2]);
  std::string path_img(argv[3]);

  tesseract::PageIteratorLevel level_ = tesseract::RIL_SYMBOL;
  if (level == "word") {
    level_ = tesseract::RIL_WORD;
  } else if (level == "symbol") {
    level_ = tesseract::RIL_SYMBOL;
  } else {
    std::cout << "level can only be word or symbol" << std::endl;
    return -1;
  }

  cv::Mat img, gray_img, binarize_img, box_img;

  img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);

  ocrus::removeRedLineFor406(img);

  cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

  ocrus::binarize(gray_img, binarize_img, 0.1);

  DenoiseLinePoint::removeNoise(binarize_img);

  //Enhancement::enhancementAndBinarize(gray_img, binarize_img, 0);

  ocrus::ocrPrintBoundingBox(binarize_img, page_seg_mode, level_, "jpn+jpnRSN");
  //ocrus::ocrPrintBoundingBox(binarize_img, page_seg_mode, level_, "chi_sim+eng");

  cv::imwrite(path_img + "_gray.png", gray_img);

  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(path_img + "_binarize.png", binarize_img);

  return 0;
}
