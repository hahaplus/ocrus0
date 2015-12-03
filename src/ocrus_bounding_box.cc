/*
 * Print the bounding boxes of an image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include <stdio.h>

#include "tesseract/baseapi.h"

#include "recognition/recognition.h"
#include "workflow/processor.h"

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

  cv::Mat gray_img, binarize_img, box_img;

  gray_img = cv::imread(path_img, CV_LOAD_IMAGE_GRAYSCALE);
  Binarize::binarize(gray_img, binarize_img);
  ocrus::ocrPrintBoundingBox(binarize_img, page_seg_mode, level_, "eng+jpn");

  cv::imwrite(path_img + "_gray.png", gray_img);

  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(path_img + "_binarize.png", binarize_img);

  return 0;
}
