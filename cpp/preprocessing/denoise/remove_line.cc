/*
 * Provide specific or general functionalities for removing all kinds of lines
 * in images
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 */

#include <opencv2/opencv.hpp>

namespace ocrus {

/**
 * Return true if a pixel is similar to red in the 406.zip images
 *
 * @param intensity: The pixel's intensity
 * @return: true if similar otherwise false
 */
static inline bool similarToRedFor406(cv::Vec3b intensity) {
  uchar blue = intensity.val[0];
  uchar green = intensity.val[1];
  uchar red = intensity.val[2];
  return (red > 100 && red > green * 1.2 && red > blue * 1.1)
      || (red > 100 && red > green * 1.1 && red > blue * 1.2);
}

void removeRedLineFor406(cv::Mat& img) {
  cv::Mat similar_mat(img.rows, img.cols, CV_8UC1);
  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols; j++) {
      if (similarToRedFor406(img.at<cv::Vec3b>(i, j))) {
        similar_mat.at<uchar>(i, j) = 1;
      } else {
        similar_mat.at<uchar>(i, j) = 0;
      }
    }
  }
  cv::Mat im_sum;
  cv::integral(similar_mat, im_sum, CV_64F);

  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols; j++) {
      cv::Vec3b intensity = img.at<cv::Vec3b>(i, j);

      int x1 = std::max(i - 2, 0);
      int y1 = std::max(j - 2, 0);
      int x2 = std::min(i + 2, img.rows - 1);
      int y2 = std::min(j + 2, img.cols - 1);
      double count_similar = im_sum.at<double>(x2, y2)
          - im_sum.at<double>(x1, y2) - im_sum.at<double>(x2, y1)
          + im_sum.at<double>(x1, y1);

      if (count_similar > 3) {
        uchar blue = intensity.val[0];
        uchar green = intensity.val[1];
        uchar red = intensity.val[2];

        int count_bg = 0;
        int sum_blue = 0, sum_green = 0, sum_red = 0;
        for (int ii = i - 50; ii < i + 50; ii++) {
          for (int jj = j - 50; jj < j + 50; jj++) {
            uchar nearby_blue = img.at<cv::Vec3b>(ii, jj).val[0];
            uchar nearby_green = img.at<cv::Vec3b>(ii, jj).val[1];
            uchar nearby_red = img.at<cv::Vec3b>(ii, jj).val[2];
            if (nearby_blue > 150 && nearby_green > 150 && nearby_red > 150) {
              sum_blue += nearby_blue;
              sum_green += nearby_green;
              sum_red += nearby_red;
              count_bg++;
            }
          }
        }
        int ave_blue = 150, ave_green = 150, ave_red = 150;
        if (count_bg > 0) {
          ave_blue = sum_blue / count_bg;
          ave_green = sum_green / count_bg;
          ave_red = sum_red / count_bg;
        }
        if (ave_blue > 255) {
          ave_blue = 255;
        }
        if (ave_green > 255) {
          ave_green = 255;
        }
        if (ave_red > 255) {
          ave_red = 255;
        }

        img.at<cv::Vec3b>(i, j) = cv::Vec3b(ave_blue, ave_green, ave_red);
      }
    }
  }
}

}  // namespace ocrus
