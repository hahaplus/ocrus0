/*
 * ImageSimilarity.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: michael
 */

#include <image_similarity/ImageSimilarity.h>

using namespace std;
using namespace cv;
ImageSimilarity::ImageSimilarity() {
  // TODO Auto-generated constructor stub

}

ImageSimilarity::~ImageSimilarity() {
  // TODO Auto-generated destructor stub
}
double ImageSimilarity::getSimilarity(cv::Mat &img_a, cv::Mat &img_b) {
  Mat src1, src2;
  src1 = normalize(img_a);
  src2 = normalize(img_b);
  vector<Point2i> pt1, pt2;
  getContourPoints(src1, pt1);
  getContourPoints(src2, pt2);

  if (pt1.size() < 3 || pt2.size() < 3)  // too simple
    return 0;

  vector<MYPOINT> my_p1, my_p2;

  cvpoint2MyPoint(pt1, my_p1);
  cvpoint2MyPoint(pt2, my_p2);

  PointMatchOutlier(&(my_p1[0]), my_p1.size(), &(my_p2[0]), my_p2.size());
  double dis = 0;
  int cnt = 0;
  for (auto p : my_p1) {
    if (p.nMatch != -1) {
      dis += getDis(Point2f(p.x, p.y),
                    Point2f(my_p2[p.nMatch].x, my_p2[p.nMatch].y));

    }
    else
    {
      dis += 4.0;
    }
  }
  for (auto p : my_p2) {
    if (p.nMatch != -1) {
      dis += getDis(Point2f(p.x, p.y),
                    Point2f(my_p1[p.nMatch].x, my_p1[p.nMatch].y));
      cnt++;
    }
    else
    {
      dis += 4.0;
    }
  }
  dis /= (my_p1.size() + my_p2.size());
  return (1.0 - min(4.0, dis) / 4.0) * 100;
}
void ImageSimilarity::cvpoint2MyPoint(vector<Point2i> &cv_points,
                                      vector<MYPOINT> &my_points) {
  for (auto pt : cv_points) {
    MYPOINT my_pt;
    my_pt.x = pt.x;
    my_pt.y = pt.y;
    my_points.push_back(my_pt);
  }
}
bool ImageSimilarity::isInRect(Point2i &pt, Rect2i &rt) {
  return pt.x >= rt.x && pt.x < rt.x + rt.width && pt.y >= rt.y
      && pt.y < rt.y + rt.height;
}
void ImageSimilarity::getContourPoints(Mat &img, vector<Point2i> &pts) {

  Mat edges;
  Canny(img, img, 100, 300);
  pts.clear();
  vector<Point2i> edgePts;
  for (int i = 0; i < img.rows; i++)
    for (int j = 0; j < img.cols; j++) {
      if (img.at<uchar>(i, j) > 100) {
        edgePts.push_back(Point2i(j, i));
      }
    }
  pts = edgePts;
  return;
  Rect2i rect = boundingRect(edgePts);
  vector<vector<Point> > contours;
  findContours(img, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
  for (int i = 0; i < contours.size(); i++) {
    for (int j = 0; j < contours[i].size(); j++) {
      if (isInRect(contours[i][j], rect)) {
        pts.push_back(contours[i][j]);
      }
    }
  }
}
Mat ImageSimilarity::normalize(Mat &img) {
  vector<Point2i> blackPts;
  for (int i = 0; i < img.rows; i++)
    for (int j = 0; j < img.cols; j++) {
      if (img.at<uchar>(i, j) < 100) {
        blackPts.push_back(Point2i(j, i));
      }
    }
  Rect2i rect = boundingRect(blackPts);
  Mat crop_img = Mat(img, rect);
  int square_size = max(crop_img.rows, crop_img.cols) + 10;
  Mat square(square_size, square_size, CV_8UC1, 255);
  int offset_x = (square_size - crop_img.cols) / 2;
  int offset_y = (square_size - crop_img.rows) / 2;
  crop_img.copyTo(
      square(cv::Rect(offset_x, offset_y, crop_img.cols, crop_img.rows)));

  cv::resize(square, square, Size(30, 30));

  //for (int i = 0; i < 3; i++)
  //GaussianBlur(square, square, Size(3, 3), 0, 0);
 // Mat kernel(3, 3, CV_32F, Scalar(-1)), sharp_result;
  //kernel.at<float>(1, 1) = 9;
  //for (int i = 0; i < 3; i++)
  //filter2D(square, square, square.depth(), kernel);
  //ocrus::binarize(square, square);
  return square;
}
double ImageSimilarity::getDis(Point2f a, Point2f b) {
  return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}
