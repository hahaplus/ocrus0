/*
 * Print the bounding boxes of an image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include "recognition/recognition.h"
#include "workflow/processor.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout
        << "Print the bounding boxes of an image " << std::endl
        << "Gray and binarized images are saved where the image is located"
        << std::endl << "Usage: ocrus_bounding_box word|symbol path_img"
        << std::endl;
    return 0;
  }

  std::string level(argv[1]);
  std::string path_img(argv[2]);

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
  ocrus::ocrPrintBoundingBox(binarize_img, level_, "eng+jpn");

  cv::imwrite(path_img + "_gray.png", gray_img);

  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(path_img + "_binarize.png", binarize_img);

  return 0;
}
