/*
 * ShadowRemove.cpp
 *
 *  Created on: Nov 26, 2015
 *      Author: michael
 */

#include "shadow_remove.h"
#include "util/algorithm_util.h"
ShadowRemove::ShadowRemove() {
  // TODO Auto-generated constructor stub

}

ShadowRemove::~ShadowRemove() {
  // TODO Auto-generated destructor stub
}
void ShadowRemove::removeShadow(Mat& mat) {
  Mat out;
  GaussianBlur(mat, out, Size(3, 3), 0, 0);
  mat = out;
  //namedWindow("xx",CV_WINDOW_NORMAL);
  //  imshow("xx", out);
  //  waitKey();
  Mat rgb_img = mat.clone();
  rgb2Ycrcb(mat);

  Mat ymat = getYFromYcrcb(mat);
  //Vec3b p = ymat.at<Vec3b>(Point(0,5));
  //printf("here");
  //printf("%d %d %d", p[0], p[1], p[2]);
  /*namedWindow("xx",CV_WINDOW_NORMAL);
   imshow("xx", ymat);
   waitKey();*/
  // get average intensity
  int cnt_iter = 3;
  while (cnt_iter--) {
    double sum = 0;
    for (int i = 0; i < ymat.rows; i++)
      for (int j = 0; j < ymat.cols; j++) {
        uchar val = ymat.at<uchar>(i, j);
        sum += val;
      }

    double avg = sum / (ymat.rows * ymat.cols);
    double nonshadow = 0;
    int nonshadow_cnt = 0;

    for (int i = 0; i < ymat.rows; i++)
      for (int j = 0; j < ymat.cols; j++) {
        uchar val = ymat.at<uchar>(i, j);
        if (!(val < 0.8 * avg && val > 0.5 * avg)) {
          nonshadow += val;
          nonshadow_cnt++;
        }
      }
    nonshadow /= nonshadow_cnt;
    double shadow_pixel = 0;
    int shadow_pixel_cnt = 0;
    for (int i = 0; i < ymat.rows; i++)
      for (int j = 0; j < ymat.cols; j++) {

        uchar &val = ymat.at<uchar>(i, j);
        if ((val < 0.8 * avg && val > 0.5 * avg)) {
          shadow_pixel += val;
          //mat.at<Vec3b>(i, j)[0] = val;
          shadow_pixel_cnt++;
        }
      }
    shadow_pixel /= shadow_pixel_cnt;

    vector<vector<pair<int, int> > > block_list = AlgorithmUtil::floodFillInMat<
        Vec<uchar, 1> >(ymat, (uchar) shadow_pixel, 10);
    for (int i = 0; i < block_list.size(); i++) {
      if (block_list[i].size() > 500) {
        vector<pair<int, int> > &block = block_list[i];
        for (int j = 0; j < block.size(); j++) {
          pair<int, int> pos = block[j];
          ymat.at<uchar>(pos.first, pos.second) = nonshadow;
          mat.at<Vec3b>(pos.first, pos.second)[0] = nonshadow;
        }
      }
    }

  }
  Vec3b v;
  v.channels;
  //mat = rgbImg;
  ycrcb2Rgb(mat);
  // namedWindow("xx",CV_WINDOW_NORMAL);

  // imshow("xx", mat);
  // waitKey();

}
Mat ShadowRemove::getYFromYcrcb(Mat& mat) {
  IplImage ycb = IplImage(mat);
  IplImage* y = cvCreateImage(cvGetSize(&ycb), 8, 1);
  IplImage* cr = cvCreateImage(cvGetSize(&ycb), 8, 1);
  IplImage* cb = cvCreateImage(cvGetSize(&ycb), 8, 1);
  cvSplit(&ycb, y, cr, cb, 0);
  return cvarrToMat(y);
}
void ShadowRemove::rgb2Ycrcb(Mat& mat) {
  IplImage src = IplImage(mat);
  IplImage* ycb = cvCreateImage(cvGetSize(&src), 8, 3);
  cvCvtColor(&src, ycb, CV_BGR2YCrCb);
  Mat ymat = cvarrToMat(ycb);
  //ymat.data = y->imageData;
  //namedWindow("xx",CV_WINDOW_NORMAL);
  //imshow("xx", ymat);
  //waitKey();
  //mat.release();
  mat = ymat;
}
void ShadowRemove::ycrcb2Rgb(Mat& mat) {
  IplImage src = IplImage(mat);
  IplImage* rgb = cvCreateImage(cvGetSize(&src), 8, 3);
  cvCvtColor(&src, rgb, CV_YCrCb2BGR);
  Mat ymat = cvarrToMat(rgb);
  //ymat.data = y->imageData;
  /* namedWindow("cb",CV_WINDOW_NORMAL);
   imshow("cb", ymat);
   waitKey();*/
  //mat.release();
  mat = ymat;
}
