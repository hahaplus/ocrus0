/*
 * fixshadow.h
 *
 *  Created on: Apr 29, 2015
 *      Author: litton
 */

#ifndef FIXSHADOW_H_
#define FIXSHADOW_H_

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <set>
#include "opencv2/text.hpp"
#include "opencv2/core/utility.hpp"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../cca/CCA.h"

using namespace std;
using namespace cv;

enum FixMode {
	NOT_FIX_HOLE,
	ZERO_FIX,
	AVER_FIX,
	ALL_AVER
};

void removeShadow(Mat& src, Mat& background, Mat& result){
	for(int i=0;i<src.cols;i++)
		for(int j=0;j<src.rows;j++){
			if(src.at<uchar>(j,i)>=background.at<uchar>(j,i)){
				result.at<uchar>(j,i) = src.at<uchar>(j,i);
			}
			else
				result.at<uchar>(j,i) = 0;
		}
}

void refineBackgroud(Mat& background, Mat& result, FixMode fmode){

	int fixScale = 5;

	Mat bbg;
	threshold(background, bbg, 0,255,THRESH_BINARY);

	Mat labelImg;
	CCA::labelByTwoPass(bbg, labelImg);

	vector<Blob> blobs;
	CCA::findBlobs(labelImg, blobs);

	map<int,int> blobAver;
	map<int,Mat> blobMask;
	set<int> garbageBlob;

	Mat finalMask = Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);

	for (int i = 0; i < blobs.size(); i++) {

		if(CCA::isGarbageBlob(blobs[i], finalMask.cols, finalMask.rows, blobs.size()))
		{
			garbageBlob.insert(i);
			continue;
		}

		vector<Point>& points = blobs[i].points;
		Mat filteredMat = Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);

		int sum = 0;
		int aver = 0;
		for (int j = 0; j < points.size(); j++) {
			if(fmode==AVER_FIX)
				sum += background.at<uchar>(points[j].y,points[j].x);
			filteredMat.ptr<uchar>(points[j].y)[points[j].x] = 255;
		}
		if(fmode==AVER_FIX){
			aver = sum/points.size();
			blobAver[i] = aver;
			blobMask[i] = filteredMat;
		}
		if(fmode==ZERO_FIX){
			blobMask[i] = filteredMat;
		}
		bitwise_or(finalMask,filteredMat,finalMask);
	}


	Mat middle;
	bitwise_and(background,finalMask,middle);
	result = middle.clone();
//	imshow("middle",middle);
//	waitKey();
	if(fmode==NOT_FIX_HOLE){
		return;
	}

	Mat contImg = Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);
	for (int i = 0; i < blobs.size(); i++) {

		if(garbageBlob.find(i)!=garbageBlob.end())
			continue;

		vector<vector<Point> > contours;
//		imshow("filter mat",blobMask[i]);
//		waitKey();
		findContours(blobMask[i], contours,CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		int aver = blobAver[i];
//		cout<<contours.size()<<" "<<aver<<endl;
		for(int j=0;j<contours.size();j++){

			Mat contour = Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);
			drawContours(contour, contours, j, Scalar(255), CV_FILLED);
//			contour.convertTo(contour,CV_8UC1);
			int nonzero = countNonZero(contour);

			if(nonzero<0.005*contour.cols*contour.rows)
			for(int y=0;y<result.rows;y++){
				for(int x=0;x<result.cols;x++){
					if(contour.at<uchar>(y,x)!=0)
						contImg.at<uchar>(y,x)=255;
					if(contour.at<uchar>(y,x)!=0&&middle.at<uchar>(y,x)==0){
						for(int curx=x-fixScale>0?x-fixScale:0;curx<=x+fixScale&&curx<result.cols;curx++){
							for(int cury=y-fixScale>0?y-fixScale:0;cury<=y+fixScale&&cury<result.rows;cury++)
								if(result.at<uchar>(cury,curx)!=aver){
									result.ptr<uchar>(cury)[curx]=aver;
								}
						}
					}
				}
			}
//			imshow("contour",contour);
//			waitKey();
		}
//		imshow("contour image",contImg);
//		waitKey();
	}
}

Mat mat2gray(const Mat& src)
{
    Mat dst;
    normalize(src, dst, 0.0, 1.0, NORM_MINMAX);
    return dst;
}

void getBackground(Mat& src, Mat& background){
	int w = 11;

	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);


	Mat grey;
	gray.convertTo(grey, CV_32F);

	Mat mu0;
	blur(grey,mu0, Size(w,w));

	Mat mu1;
	blur(grey,mu1, Size(w,w));

	Mat mu2;
	blur(mu0,mu2, Size(2*w+1,2*w+1));

	Mat mu3;
	blur(mu0.mul(mu0),mu3, Size(2*w+1,2*w+1));

	Mat sigma;
	cv::sqrt(mu3-mu2.mul(mu2),sigma);

//	imshow("mean",mat2gray(mu1));
//	imshow("sigma",mat2gray(sigma));
//	waitKey();

	Mat tv1 = 0.3*sigma+16;
	//imshow("tv1",mat2gray(tv1));
	Mat mask;
	threshold(sigma - tv1, mask,0,255,THRESH_BINARY_INV);
	//imshow("mask1",mat2gray(mask));

	Mat rem,remb;
	bitwise_and(sigma,mask,rem);
	threshold(rem, remb,0,255,THRESH_BINARY);

	double fz = 0.0, fm = 0.0;
	double avg1=cv::mean(rem).val[0],avg2=cv::mean(remb).val[0];

	double noise=avg1*255/avg2;
	cout<<avg1<<" "<<avg2<<" "<<noise<<endl;

	Mat tv2 = 0.3*sigma+noise;
	Mat mask2;

	threshold(sigma - tv2, mask2,0,255,THRESH_BINARY);

//	imshow("tv2",mat2gray(tv2));
//	imshow("mask2",mat2gray(mask2));
//	waitKey();

//	Mat rmv;
//	bitwise_and(mu1,mask2,background/*rmv*/);
	Mat mask3;
	bitwise_not(mask2,mask3);
	bitwise_and(grey,mask3,background);
/*
	imshow("rmv",mat2gray(rmv));

	Mat rem2 = mu1 - rmv;
	imshow("rem",mat2gray(rem2));

	Mat proc0;
	threshold(rem2, proc0,240,255,THRESH_BINARY);
	imshow("proc0",mat2gray(proc0));

	Mat proc1;
	threshold(rem2, proc1,10,255,THRESH_BINARY_INV);
	imshow("proc1",mat2gray(proc1));

	Mat proc2;
	bitwise_or(proc0,proc1,proc2);
	imshow("proc2",mat2gray(proc2));
	waitKey();
*/
	background.convertTo(background,CV_8U);
//	imshow("background",background);
//	waitKey();
}

#endif /* FIXSHADOW_H_ */
