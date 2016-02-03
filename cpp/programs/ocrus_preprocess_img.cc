/*
 * A program that can preprocess an image
 *
 * Copyright (C) 2016 Works Applications, all rights reserved
 */

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include "preprocessing/binarize/binarize.h"
#include "preprocessing/denoise/remove_line.h"
#include "preprocessing/enhancement/enhancement.h"

int main(int argc, char *argv[]) {

  if (argc != 5) {
    printf("Preprocess a color image (remove noise, binarize and enhance)\n"
           "Usage: ocrus_preprocess_img input_path_img output_path_img k e\n"
           "    k: 0~1.0, parameter in WolfJolion binarization algorithm\n"
           "    e: 0~1.0, larger value gives stronger enhancement, 0: no enhancement\n");
    return 0;
  }

  std::string path_img(argv[1]);
  std::string path_img_binary(argv[2]);
  double k = atof(argv[3]);
  double e = atof(argv[4]);

  cv::Mat img, img_gray, img_binary;

  img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);

  ocrus::removeRedLineFor406(img);
  cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  Enhancement::enhancementAndBinarize(img_gray, img_binary, k, e);

  cv::imwrite(path_img_binary, img_binary);

  return 0;
}
