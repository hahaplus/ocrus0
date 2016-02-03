/*
 * Binarization of image
 *
 * Copyright (C) 2016 Works Applications, all rights reserved
 */

#include "binarize.h"

#include <opencv2/opencv.hpp>

using namespace cv;

namespace ocrus {

/**
 * Glide a rect window across the image and create two stats images:
 * mean and standard deviation.
 *
 * @param im: Image mat for calculation
 * @param map_m: Mean stat image, coordinates are window center
 * @param map_s: Standard deviation stat image, coordinates are window center
 * @param winx: Width of the window
 * @param winy: height of the window
 * @return: Max value in map_s
 *
 * Adapted from version 2.4 in
 * http://liris.cnrs.fr/christian.wolf/software/binarize/index.html
 */
static double calcLocalStats(Mat &im, Mat &map_m, Mat &map_s, int winx,
                             int winy) {
  Mat im_sum, im_sum_sq;
  cv::integral(im, im_sum, im_sum_sq, CV_64F);

  double m, s, max_s, sum, sum_sq;

  int winx_half = winx / 2;
  int winy_half = winy / 2;

  int y_firstth = winy_half;
  int y_lastth = im.rows - winy_half - 1;

  double winarea = winx * winy;

  max_s = 0;
  for (int j = y_firstth; j <= y_lastth; j++) {
    sum = sum_sq = 0;

    // j : [winy_half, im.rows - winy_half - 1]
    // row: [j - winy_half, j - winy_half + winy)
    // col: [0, winx)
    // Suppose winy is even
    // When j is min, [0, winy) x [0, winx)
    // When j is max, [rows - 1 - winy, rows -1) x [0, winx)
    sum = im_sum.at<double>(j - winy_half + winy, winx)
        - im_sum.at<double>(j - winy_half, winx)
        - im_sum.at<double>(j - winy_half + winy, 0)
        + im_sum.at<double>(j - winy_half, 0);
    sum_sq = im_sum_sq.at<double>(j - winy_half + winy, winx)
        - im_sum_sq.at<double>(j - winy_half, winx)
        - im_sum_sq.at<double>(j - winy_half + winy, 0)
        + im_sum_sq.at<double>(j - winy_half, 0);

    m = sum / winarea;
    s = sqrt((sum_sq - m * sum) / winarea);
    if (s > max_s) {
      max_s = s;
    }
    // ----------------
    // | *
    // | *
    // | *
    // ----------------
    map_m.at<float>(j, winx_half) = m;
    map_s.at<float>(j, winx_half) = s;

    // Shift the window, add and remove new/old values to the histogram
    for (int i = 1; i <= im.cols - winx; i++) {

      // Remove the left old column and add the right new column
      // When j is min, i is min
      // sum = sum - [0, winy) x [0, 1) + [0, winy) x [winx, winx + 1)
      sum -= im_sum.at<double>(j - winy_half + winy, i)
          - im_sum.at<double>(j - winy_half, i)
          - im_sum.at<double>(j - winy_half + winy, i - 1)
          + im_sum.at<double>(j - winy_half, i - 1);
      sum += im_sum.at<double>(j - winy_half + winy, i + winx)
          - im_sum.at<double>(j - winy_half, i + winx)
          - im_sum.at<double>(j - winy_half + winy, i + winx - 1)
          + im_sum.at<double>(j - winy_half, i + winx - 1);

      sum_sq -= im_sum_sq.at<double>(j - winy_half + winy, i)
          - im_sum_sq.at<double>(j - winy_half, i)
          - im_sum_sq.at<double>(j - winy_half + winy, i - 1)
          + im_sum_sq.at<double>(j - winy_half, i - 1);
      sum_sq += im_sum_sq.at<double>(j - winy_half + winy, i + winx)
          - im_sum_sq.at<double>(j - winy_half, i + winx)
          - im_sum_sq.at<double>(j - winy_half + winy, i + winx - 1)
          + im_sum_sq.at<double>(j - winy_half, i + winx - 1);

      m = sum / winarea;
      s = sqrt((sum_sq - m * sum) / winarea);
      if (s > max_s) {
        max_s = s;
      }
      // ----------------
      // |   * ...
      // |   * ...
      // |   * ...
      // ----------------
      map_m.at<float>(j, winx_half + i) = m;
      map_s.at<float>(j, winx_half + i) = s;
    }
  }

  return max_s;
}

void NiblackSauvolaWolfJolion(Mat im, Mat output, NiblackVersion version,
                              int winx, int winy, double k, double dR) {

  double m, s, max_s;
  double th = 0;
  double min_I, max_I;

  int winx_half = winx / 2;
  int winy_half = winy / 2;

  int x_firstth = winx_half;
  int x_lastth = im.cols - winx_half - 1;

  int y_firstth = winy_half;
  int y_lastth = im.rows - winy_half - 1;

  int mx, my;

  // Create local statistics and store them in a double matrices
  Mat map_m = Mat::zeros(im.rows, im.cols, CV_32F);
  Mat map_s = Mat::zeros(im.rows, im.cols, CV_32F);
  max_s = calcLocalStats(im, map_m, map_s, winx, winy);

  minMaxLoc(im, &min_I, &max_I);

  Mat thsurf(im.rows, im.cols, CV_32F);

  // Create the threshold surface, including border processing
  //
  // 1 1 1 1 1 1 1|1 2
  // 1 1 1 1 1 1 1|1 2
  // 1 1 1 1 1 x_firstth
  //-------y_firstth-------------------
  // 1 1 1 1 1 1 1|1 2
  // 3 3 3 3 3 3 3|3
  // Digits are pixel values
  //
  // Comment by Chang Sun: The border processing might not be very adequate
  for (int j = y_firstth; j <= y_lastth; j++) {

    // NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
    for (int i = 0; i <= im.cols - winx; i++) {

      m = map_m.at<float>(j, i + winx_half);
      s = map_s.at<float>(j, i + winx_half);

      // Calculate the threshold
      switch (version) {

        case NIBLACK:
          th = m + k * s;
          break;

        case SAUVOLA:
          th = m * (1 + k * (s / dR - 1));
          break;

        case WOLFJOLION:
          th = m + k * (s / max_s - 1) * (m - min_I);
          break;

        default:
          std::cerr << "Unknown NiblackVersion value\n";
          exit(1);
      }

      thsurf.at<float>(j, i + winx_half) = th;

      if (i == 0) {
        // LEFT BORDER
        for (int ii = 0; ii <= x_firstth; ++ii) {
          thsurf.at<float>(j, ii) = th;
        }

        // LEFT-UPPER CORNER
        if (j == y_firstth) {
          for (int u = 0; u < y_firstth; ++u) {
            for (int ii = 0; ii <= x_firstth; ++ii) {
              thsurf.at<float>(u, ii) = th;
            }
          }
        }

        // LEFT-LOWER CORNER
        if (j == y_lastth) {
          for (int u = y_lastth + 1; u < im.rows; ++u) {
            for (int ii = 0; ii <= x_firstth; ++ii) {
              thsurf.at<float>(u, ii) = th;
            }
          }
        }
      }

      // UPPER BORDER
      if (j == y_firstth) {
        for (int u = 0; u < y_firstth; ++u) {
          thsurf.at<float>(u, i + winx_half) = th;
        }
      }

      // LOWER BORDER
      if (j == y_lastth) {
        for (int u = y_lastth + 1; u < im.rows; ++u) {
          thsurf.at<float>(u, i + winx_half) = th;
        }
      }
    }

    // RIGHT BORDER
    for (int i = x_lastth; i < im.cols; ++i) {
      thsurf.at<float>(j, i) = th;
    }

    // RIGHT-UPPER CORNER
    if (j == y_firstth) {
      for (int u = 0; u < y_firstth; ++u) {
        for (int i = x_lastth; i < im.cols; ++i) {
          thsurf.at<float>(u, i) = th;
        }
      }
    }

    // RIGHT-LOWER CORNER
    if (j == y_lastth) {
      for (int u = y_lastth + 1; u < im.rows; ++u) {
        for (int i = x_lastth; i < im.cols; ++i) {
          thsurf.at<float>(u, i) = th;
        }
      }
    }
  }

  for (int y = 0; y < im.rows; ++y) {
    for (int x = 0; x < im.cols; ++x) {
      if (im.at<unsigned char>(y, x) >= thsurf.at<float>(y, x)) {
        output.at<unsigned char>(y, x) = 255;
      } else {
        output.at<unsigned char>(y, x) = 0;
      }
    }
  }

}

void binarize(const Mat& img_gray, Mat& img_binary, double k) {
  CV_Assert(img_gray.channels() == 1);
  Mat tmp = img_gray.clone();
  int winx = 40;
  int winy = 40;
  img_binary.create(img_gray.size(), img_gray.type());
  NiblackSauvolaWolfJolion(tmp, img_binary, WOLFJOLION, winx, winy, k, 128);
}

}
// namespace ocrus
