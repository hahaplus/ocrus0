/*
 * OCR an image and draw OCR results to the binarized copy of this image
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include "recognition/recognition.h"
#include "workflow/processor.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "OCR and draw OCR results to the binary image" << std::endl
              << "Usage: ocrus_ocr_draw word|symbol path_img path_out_img"
              << std::endl;
    return 0;
  }

  std::string level(argv[1]);
  std::string path_img(argv[2]);
  std::string path_out_img(argv[3]);

  int cut_level = 0;
  if (level == "word") {
    cut_level = 1;
  } else if (level == "symbol") {
    cut_level = 0;
  } else {
    std::cout << "level can only be word or symbol" << std::endl;
    return -1;
  }

  cv::Mat gray_img, binarize_img, out_img;

  gray_img = cv::imread(path_img, CV_LOAD_IMAGE_GRAYSCALE);
  Binarize::binarize(gray_img, binarize_img);

  cv::Mat in_img = binarize_img.clone();

  OcrDetailResult result;
  WapOcrApi::recognitionToText(binarize_img, "eng+jpn", cut_level, &result);

  ocrus::drawOcrResult(in_img, result, &out_img);
  cv::imwrite(path_out_img, out_img);

  return 0;
}
