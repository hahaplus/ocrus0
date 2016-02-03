/*
 * A program that can binarize an image
 *
 * Copyright (C) 2016 Works Applications, all rights reserved
 */

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/opencv.hpp>

#include "preprocessing/binarize.h"

int main(int argc, char *argv[]) {

  if (argc != 4) {
    printf("Binarize an image\n"
           "Usage: ocrus_binarize_img path_img path_img_binary k\n"
           "    k: The parameter in WolfJolion algorithm\n");
    return 0;
  }

  std::string path_img(argv[1]);
  std::string path_img_binary(argv[2]);
  double k = atof(argv[3]);

  cv::Mat img, img_gray, img_binary;

  img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);
  cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);

  ocrus::binarize(img_gray, img_binary, k);

  cv::imwrite(path_img_binary, img_binary);

  return 0;
}
