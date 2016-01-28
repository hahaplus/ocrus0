/*
 * ImageSimilarity.h
 *
 *  Created on: Jan 27, 2016
 *      Author: michael
 */

#ifndef SRC_IMAGE_SIMILARITY_IMAGESIMILARITY_H_
#define SRC_IMAGE_SIMILARITY_IMAGESIMILARITY_H_
#include <opencv2/opencv.hpp>
#include <vector>
#include "image_similarity/PointMatch.h"

class ImageSimilarity {
 public:
  ImageSimilarity();
  /*
   * @param: img_a, img_b are two image
   * @return: return the similairty between the two shape, range (0~100),
   *          the bigger the value is, the more similar the two shapes are.
   */
  static double getSimilarity(cv::Mat &img_a, cv::Mat &img_b);
  virtual ~ImageSimilarity();
 private:
  static void cvpoint2MyPoint(std::vector<cv::Point2i> &cv_points, std::vector<MYPOINT> &my_points);
  static bool isInRect(cv::Point2i &pt, cv::Rect2i &rt);
  static void getContourPoints(cv::Mat &img, std::vector<cv::Point2i> &pts);
  static cv::Mat normalize(cv::Mat &img);
  static double getDis(cv::Point2f a, cv::Point2f b);
};

#endif /* SRC_IMAGE_SIMILARITY_IMAGESIMILARITY_H_ */
