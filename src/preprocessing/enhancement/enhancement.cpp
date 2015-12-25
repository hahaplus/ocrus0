/*
 * Enhancement.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: michael
 */

#include <preprocessing/enhancement/enhancement.h>
#include "binarization/wap_binarize.h"
#include "textDetect/simple_text_detect.h"
#include "preprocessing/denoise/denoise_line_point.h"
#include "util/general.h"


using namespace std;
using namespace cv;
Enhancement::Enhancement() {
  // TODO Auto-generated constructor stub

}

Enhancement::~Enhancement() {
  // TODO Auto-generated destructor stub
}
void Enhancement::imageStretchByHistogram(IplImage *src1, IplImage *dst1)
/*************************************************
 Function:      通过直方图变换进行图像增强，将图像灰度的域值拉伸到0-255
 src1:               单通道灰度图像
 dst1:              同样大小的单通道灰度图像
 *************************************************/
{
  assert(src1->width == dst1->width);
  double p[256], p1[256], num[256];

  memset(p, 0, sizeof(p));
  memset(p1, 0, sizeof(p1));
  memset(num, 0, sizeof(num));
  int height = src1->height;
  int width = src1->width;
  long wMulh = height * width;

  //statistics
  for (int x = 0; x < src1->width; x++) {
    for (int y = 0; y < src1->height; y++) {
      uchar v = ((uchar*) (src1->imageData + src1->widthStep * y))[x];
      num[v]++;
    }
  }
  //calculate probability
  for (int i = 0; i < 256; i++) {
    p[i] = num[i] / wMulh;
  }

  //p1[i]=sum(p[j]);  j<=i;
  for (int i = 0; i < 256; i++) {
    for (int k = 0; k <= i; k++)
      p1[i] += p[k];
  }

  // histogram transformation
  for (int x = 0; x < src1->width; x++) {
    for (int y = 0; y < src1->height; y++) {
      uchar v = ((uchar*) (src1->imageData + src1->widthStep * y))[x];
      ((uchar*) (dst1->imageData + dst1->widthStep * y))[x] = p1[v] * 255 + 0.5;
    }
  }


}

void Enhancement::getBBox(const cv::Mat &img, OcrDetailResult* odr) {
  odr->clear();
  vector<vector<pair<int, int> > > block_list = AlgorithmUtil::floodFillInMat<
      Vec<uchar, 1> >(img, 0, 0);
  vector<double> avg_height, avg_width;
  for (int i = 0; i < block_list.size(); i++) {
    vector<pair<int, int> > block = block_list[i];
    ResultUnit ru;
    for (auto b : block) {
      ru.bounding_box[0].x = min(ru.bounding_box[0].x, b.second);
      ru.bounding_box[1].x = max(ru.bounding_box[1].x, b.second);
      ru.bounding_box[0].y = min(ru.bounding_box[0].y, b.first);
      ru.bounding_box[1].y = max(ru.bounding_box[1].y, b.first);
    }
   // avg_height.push_back(ru.bounding_box[1].y - ru.bounding_box[0].y);
    //avg_width.push_back(ru.bounding_box[1].x - ru.bounding_box[0].x);
    odr->push_back_symbol(ru);
  }
 // double avg_h = AlgorithmUtil::getAverageValue<double>(avg_height);
 // double avg_w = AlgorithmUtil::getAverageValue<double>(avg_width);
  //odr->setResult(filted_result);
}
void Enhancement::enhancementAndBinarize(const cv::Mat &src, cv::Mat &dst, double k) {

  /*Mat out;
      int center = 100;
      double alpha = 1.1;
      double beta = (1 - alpha) * center;
      src.convertTo(src, src.type(), alpha, beta);
      ocrus::binarize(src, dst);
      // General::showImage(dst);
      return;*/
  Mat binarize_img, enhanced_binarize_img;
  OcrDetailResult boxes;

  //for (int i = 0; i < 2; i++)
  // smooth
  GaussianBlur(src, src, Size(3,3), 0, 0);
  // sharpen
  Mat kernel(3,3,CV_32F,Scalar(-1));
  kernel.at<float>(1,1) = 9;
  filter2D(src, src, src.depth(),kernel);
  //General::showImage(src);
  //======================get the box of character=======================
  ocrus::binarize(src, binarize_img);


  Rect text_area = SimpleTextDetect::simpleDetect(src);

  DenoiseLinePoint::removeNoise(binarize_img/*, &text_area*/);
  if ( abs(k) < 1e-6 )   // k is very low then do not need enhance
  {
      dst = binarize_img;
      return;
  }
  //General::showImage(binarize_img);
  //getBBox(binarize_img, &boxes);
  //=====================================================================

  //=======================get the enhanced binarize image===============
  IplImage gray_input(src);
  IplImage gray_output = *cvCreateImage(cvGetSize(&gray_input), 8, 1);
  imageStretchByHistogram(&gray_input, &gray_output);
  Mat gray_img = cv::cvarrToMat(&gray_output);
  for (int i =0; i < 3; i++)
  GaussianBlur(gray_img, gray_img, Size(3,3), 0, 0);
  //bilateralFilter(gray_img, dst, 10, 40, 40);
 // gray_img = dst;
  ocrus::binarize(gray_img, enhanced_binarize_img, 1.0 - k);
  //General::showImage(enhanced_binarize_img);
  vector<vector<pair<int, int> > > block_list = AlgorithmUtil::floodFillInMat<
        Vec<uchar, 1> >(enhanced_binarize_img, 0, 0);
  // draw



  for (auto block : block_list) {
    //rectangle(enhanced_binarize_img, left_top, block.bounding_box[1],0);
    int cnt = 0, sum = 0;
    for (auto pixel : block)
    {
        if (enhanced_binarize_img.at<uchar>(pixel.first, pixel.second) == 0)
        {
          sum++;
          if (binarize_img.at<uchar>(pixel.first, pixel.second) == 0)
            cnt++;
        }
     }
    if (cnt < sum * 0.3) // it means this is a noise so remove them
    {
       for (auto pixel : block) {
          enhanced_binarize_img.at<uchar>(pixel.first, pixel.second) = 255;
       }
    }
  }

  //for (int i = 0; i < 2; i++)
  //GaussianBlur(enhanced_binarize_img, enhanced_binarize_img, Size(3,3), 0, 0);
    //bilateralFilter(enhanced_binarize_img, dst, 10, 40, 40);
  dst = enhanced_binarize_img;
 // General::showImage(dst);
}

