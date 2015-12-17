/*
 * general.h
 *
 *  Created on: Feb 4, 2015
 *      Author: fc
 */

#ifndef UTIL_GENERAL_H_
#define UTIL_GENERAL_H_

#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class General{
public:
  General(Mat image):
    _image(image){

  }
    pair<Vec3b, double> meanVariance(){
      int rows = _image.rows;
      int cols = _image.cols;
      if(_image.isContinuous()){
        cols *= rows;
        rows = 1;
      }
      int b = 0;
      int g = 0;
      int r = 0;
      for(int y = 0; y < rows; ++y){
        Vec3b *row = _image.ptr<Vec3b>(y);
        for(int x = 0; x < cols; ++x, ++row){
          b += (*row)[0];
          g += (*row)[1];
          r += (*row)[2];
        }
      }
      b /= (rows * cols);
      g /= (rows * cols);
      r /= (rows * cols);
      double variance = 0;
      for(int y = 0; y < rows; ++y){
        Vec3b *row = _image.ptr<Vec3b>(y);
        for(int x = 0; x < cols; ++x, ++row){
          variance += (b - (*row)[0]) * (b - (*row)[0]) + (g - (*row)[1]) * (g - (*row)[1])  + (r - (*row)[2]) * (r - (*row)[2]);
        }
      }
      variance /= (rows * cols);
      variance = sqrt(variance);
      return make_pair(Vec3b(b,g,r), variance);
    }
	static void showImage(Mat src, string window_name = "xx")
	{
	  namedWindow(window_name, CV_WINDOW_NORMAL);
	  imshow(window_name, src);
	  waitKey();
	}
private:
	Mat _image;
};





#endif /* UTIL_GENERAL_H_ */
