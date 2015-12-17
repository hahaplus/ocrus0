/*
 * SimpleTextDetect.cpp
 *
 *  Created on: Dec 17, 2015
 *      Author: michael
 */

#include <textDetect/simple_text_detect.h>
using namespace cv;
using namespace std;
SimpleTextDetect::SimpleTextDetect() {
  // TODO Auto-generated constructor stub

}

SimpleTextDetect::~SimpleTextDetect() {
  // TODO Auto-generated destructor stub
}

cv::Rect SimpleTextDetect::simpleDetect(cv::Mat src)
{
  // convert to grey image
  Mat grey_img;
  if (src.channels() != 1)
  {
     cv::cvtColor(src, grey_img, cv::COLOR_BGR2GRAY);
  }
  else
  {
    grey_img = src.clone();
  }
  std::vector< std::vector< cv::Point> > contours;
  std::vector< cv::Rect> boxes;
  cv::Ptr< cv::MSER> mser =cv::MSER::create( 8, 10, 2000, 0.25, 0.1, 100, 1.01, 0.03, 5 );
  mser->detectRegions(grey_img, contours, boxes);
  Rect rt;
  Point left_top(1e6, 1e6), right_bottom(-1e6, -1e6);
  for (auto character : contours)
   for (auto point : character)
   {
       left_top.x = min(left_top.x, point.x);
       left_top.y = min(left_top.y, point.y);
       right_bottom.x = max(right_bottom.x, point.x);
       right_bottom.y = max(right_bottom.y, point.y);
   }
  return Rect(left_top.x, left_top.y, right_bottom.x - left_top.x, right_bottom.y - left_top.y );
}
