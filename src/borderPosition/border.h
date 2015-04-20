/*
 * DisplayImage.cpp
 *
 *  Created on: Jan 7, 2015
 *      Author: litton
 */

/**
 * Automatic perspective correction for quadrilateral objects. See the tutorial at
 * http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */

#ifndef BORDER_H
#define BORDER_H

#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <strstream>
#include <fstream>
#include "integration.h"
#include "dataStructures.h"
#include "basicOperations.h"
#include "lineEvaluation.h"
#include "outputGenerate.h"
#include "quadrangleCorrection.h"
#include "quadrangleScore.h"
#include "quadrangleEvaluation.h"
#include "pickCrossCands.h"
#include "../salientRecognition/rc/main.h"

using namespace cv;
using namespace std;

#define hough_cmp_gt(l1,l2) (aux[l1] > aux[l2])

int process(cv::Mat tsrc, Mat tslt,
		vector<vector<cv::Point2f> >& cross, bool binary) {

	scoreCur[0] = 0;
	scoreCur[1] = 0;
	scoreCur[2] = 0;
	//step0: to gray picture

	cv::Mat bw;
	if (!binary)
		cv::cvtColor(tsrc, bw, CV_BGR2GRAY);
	else {
		Mat myRGB = convertToVisibleMat<float>(tsrc);
		cvtColor(myRGB, bw, CV_BGR2GRAY);
	}
	//step1: edge detection
	cv::Mat pic1;
	int ddepth = 3;

	cv::Sobel(bw, grad_x, ddepth, 1, 0);
	cv::convertScaleAbs(grad_x, abs_grad_x);

	cv::Sobel(bw, grad_y, ddepth, 0, 1);
	cv::convertScaleAbs(grad_y, abs_grad_y);

	cv::addWeighted(abs_grad_x, 1, abs_grad_y, 1, 0, grad);

	cv::threshold(grad, pic1, lighting, 255, CV_THRESH_TOZERO);

	std::cout << "img size: " << pic1.cols << " " << pic1.rows << std::endl;

	//step2: Hough transform
	IplImage iplimg = pic1;
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	//lines = cvHoughLines3( &iplimg, storage, CV_HOUGH_STANDARD, 5, CV_PI/90, 70, 30, 10 );

	std::vector<cv::Vec4i> lines0;
	cv::HoughLinesP(pic1, lines0, 5, CV_PI / 90, 100, 70, 20);

	lines = convertXYLineToPolar(lines0, storage, pic1);
	//step3: quadrangle formation
	//s3.1: filter candidate lines
	int i = 0;
	std::vector<int> fakeLines(lines->total);
	int fakes = 0;
	for (; i < lines->total; i++) {
		//std::cout<<i<<" of "<<lines->total<<std::endl;
		CvLinePolar2* line = (CvLinePolar2*) cvGetSeqElem(lines, lineSorted[i]);

		if (line->votes * VOTERATE
						> ((CvLinePolar2*) cvGetSeqElem(lines, 0))->votes) {

			cv::Point pt1, pt2;
			float rho = line->rho, theta = line->angle;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = line->x1;	//cvRound(x0 + 1000*(-b));
			pt1.y = line->y1;	//cvRound(y0 + 1000*(a));
			pt2.x = line->x2;	//cvRound(x0 - 1000*(-b));
			pt2.y = line->y2;	//cvRound(y0 - 1000*(a));
			cv::Mat pic11 = pic1.clone();
			fakeLines[i] = 1;

		} else {
			break;
		}
	}
	int cut = i;
	std::cout << "lines number " << cut << std::endl;
	std::cout << "fake lines " << fakes << std::endl;
	if (cut > 1000)
		cut = 1000;

	//s3.2 filter pairs of lines with angle and distance between them
	vector<OppositeLines> horiPairs;
	vector<OppositeLines> vertPairs;

	int width = grad.cols;
	int height = grad.rows;

	//iteration can be merged with the previous one
	for (i = 0; i < cut; i++) {
		if (fakeLines[i] == 0)
			continue;
		CvLinePolar2* l1 = (CvLinePolar2*) cvGetSeqElem(lines, lineSorted[i]);
		//if(fabs(fabs(l1->angle)-CV_PI/2)<0.00001) continue;

		for (int j = i + 1; j < cut; j++) {
			if (fakeLines[j] == 0)
				continue;
			CvLinePolar2* l2 = (CvLinePolar2*) cvGetSeqElem(lines,
					lineSorted[j]);
			//if(fabs(fabs(l2->angle)-CV_PI/2)<0.00001) continue;

			double dangle = fabs(l1->angle - l2->angle);//TODO when rho is minus...
			double drho = fabs(fabs(l1->rho) - fabs(l2->rho));
			if (l1->rho * l2->rho < 0 && dangle < CV_PI)
				drho = fabs(l1->rho - l2->rho);
			double rho1 = l1->rho, rho2 = l2->rho;
			double theta1 = l1->angle, theta2 = l2->angle;

			OPPOANG = 0.25;
			if ((dangle >= CV_PI * (1.0 - OPPOANG)
					&& dangle <= CV_PI * (OPPOANG + 1.0)
					&& (drho = l1->rho + l2->rho)
					|| (dangle <= CV_PI * OPPOANG
							|| (dangle >= (2.0 - OPPOANG) * CV_PI
									&& dangle <= 2.0 * CV_PI))
							&& (rho1
									/ (cos(theta1)
											+ height * sin(theta1) / width)
									- 0.5 * width)
									* (rho2
											/ (cos(theta2)
													+ height * sin(theta2)
															/ width)
											- 0.5 * width) < 0)
					&& (drho > 0.02 * width && drho > 0.02 * height)) {

				OppositeLines oppLines;
				oppLines.one = lineSorted[i];
				oppLines.two = lineSorted[j];

				if(isHoriLine(l1->angle)&&isHoriLine(l2->angle))
					horiPairs.push_back(oppLines);
				if(isVertLine(l1->angle)&&isVertLine(l2->angle))
					vertPairs.push_back(oppLines);
			}
		}
	}

	//s3.3 try to compose line pairs into quadrangle
	std::vector<Quadrangle> quadVector;
	int finalK = -1;
	int finalL = -1;
	double mcirc = -1;
	double minDistToM = 9999;
	double minAngleSum = 9999;
	double maxScore = -1;
	int lStart = 0;

	priority_queue<quadrNode> qn;

	for (int k = 0; k<100&&k<horiPairs.size(); k++) {
		OppositeLines pair1 = horiPairs.at(k);
		CvLinePolar2 *clines[4];
		clines[0] = (CvLinePolar2*) cvGetSeqElem(lines, pair1.one);
		clines[1] = (CvLinePolar2*) cvGetSeqElem(lines, pair1.two);
		cv::Vec4i xylines[4];
		for (int m = 0; m < 2; m++) {
			float rho = clines[m]->rho, theta = clines[m]->angle;
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			xylines[m][0] = cvRound(x0 + 1000 * (-b));
			xylines[m][1] = cvRound(y0 + 1000 * (a));
			xylines[m][2] = cvRound(x0 - 1000 * (-b));
			xylines[m][3] = cvRound(y0 - 1000 * (a));
		}

		for (int l=0;l<100&&l<vertPairs.size();l++) {

			OppositeLines pair2 = vertPairs.at(l);
			if (pair1.one == pair2.one || pair1.one == pair2.two
					|| pair1.two == pair2.one || pair1.two == pair2.two)
				continue;

			//std::cout<<"here"<<std::endl;
			clines[2] = (CvLinePolar2*) cvGetSeqElem(lines, pair2.one);
			clines[3] = (CvLinePolar2*) cvGetSeqElem(lines, pair2.two);

			for (int m = 2; m < 4; m++) {
				float rho = clines[m]->rho, theta = clines[m]->angle;
				cv::Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a * rho, y0 = b * rho;
				xylines[m][0] = cvRound(x0 + 1000 * (-b));
				xylines[m][1] = cvRound(y0 + 1000 * (a));
				xylines[m][2] = cvRound(x0 - 1000 * (-b));
				xylines[m][3] = cvRound(y0 - 1000 * (a));
			}

//			if(k==0&&l==12)
//				cout<<"here1"<<endl;

			int padding = 10.0;
			cv::Point2f pt[4];
			bool pass1 = true;
			for (int n = 0; n < 4; n++) {

				pt[n] = computeLineIntersect(xylines[n / 2], xylines[2 + n % 2]);

				double dnangle = fabs(
						clines[n / 2]->angle - clines[2 + n % 2]->angle);
//				if(k==0&&l==12)
//					std::cout<<"DANAGEL "<<n<<" "<<dnangle<<std::endl;
				/*
				 if(n>0)
				 std::cout<<n<<" "<<dnangle<<std::endl;
				 */

                //s3.3.1 if the intersect point of two lines is outside the range of picture, fail to compose
				if (pt[n].x < -padding || pt[n].y < -padding
						|| pt[n].x > width + padding
						|| pt[n].y > height + padding) {
					pass1 = false;
//					if(k==0&&l==12)
//						std::cout<<"dead1"<<std::endl;
					break;
				}

				//s3.3.2 if the angle between the two lines is too far from Right Angle, fail to compose
				if (!((dnangle >= CV_PI / 3.0 && dnangle <= CV_PI * 2.0 / 3.0)
						|| (dnangle >= CV_PI * 4.0 / 3.0 - 0.04
								&& dnangle <= CV_PI * 5.0 / 3.0))) {
					pass1 = false;
//					if(k==0&&l==12)
//						std::cout<<"dead2"<<std::endl;
					break;
				}
			}

			if (!pass1)
				continue;

//			if(k==0&&l==12)
//				cout<<"here2"<<endl;
			//s3.3.3 the found quadrangle must be large enough
			double lnab = dist(pt[0], pt[1]);
			double lnbd = dist(pt[1], pt[3]);
			double lncd = dist(pt[2], pt[3]);
			double lnac = dist(pt[0], pt[2]);
			double circ = lnab + lnbd + lncd + lnac;
			if (circ <= (width + height) * 0.5)
				continue;
			//s3.3.4* there must not be too strange positions of different angles
			int sort_arr[4] = { 0, 1, 2, 3 };
			for (int sort_cur1 = 0; sort_cur1 < 4; sort_cur1++) {
				for (int sort_cur2 = sort_cur1 + 1; sort_cur2 < 4;
						sort_cur2++) {
					int ix1 = sort_arr[sort_cur1];
					int ix2 = sort_arr[sort_cur2];
					//if(k==0&&l==13)
					//	std::cout<<"angle: "<<ix1<<std::endl;
					double angv1 = normalizeAngle(clines[ix1], width, height);
					//if(k==0&&l==13)
					//	std::cout<<"angle: "<<ix2<<std::endl;
					double angv2 = normalizeAngle(clines[ix2], width, height);

					if (angv1 > angv2) {
						sort_arr[sort_cur1] = ix2;
						sort_arr[sort_cur2] = ix1;
					}
				}
			}
//			if(k==48&&l==63)
//				std::cout<<"herek"<<std::endl;
			if (sort_arr[0] / 2 != sort_arr[1] / 2
					&& sort_arr[1] / 2 != sort_arr[2] / 2) {//s3.3.4 filter angle positions
				//success to get a quadrangle
				//if(k==270&&l==380)
				//	std::cout<<"here2"<<std::endl;
				double rho0 = clines[0]->rho;
				double rho1 = clines[1]->rho;
				double rho2 = clines[2]->rho;
				double rho3 = clines[3]->rho;

				double theta0 =
						rho0 >= 0 ?
								clines[0]->angle : -CV_PI + clines[0]->angle;
				double theta1 =
						rho1 >= 0 ?
								clines[1]->angle : -CV_PI + clines[1]->angle;
				double theta2 =
						rho2 >= 0 ?
								clines[2]->angle : -CV_PI + clines[2]->angle;
				double theta3 =
						rho3 >= 0 ?
								clines[3]->angle : -CV_PI + clines[3]->angle;

				cv::Vec4i chaline1, chaline2;
				chaline1[0] = pt[0].x;
				chaline1[1] = pt[0].y;
				chaline1[2] = pt[3].x;
				chaline1[3] = pt[3].y;

				chaline2[0] = pt[1].x;
				chaline2[1] = pt[1].y;
				chaline2[2] = pt[2].x;
				chaline2[3] = pt[2].y;

//				cv::Point2d jiao = computeLineIntersect(chaline1, chaline2);
//				cv::Point2d zhon;
//				zhon.x = width / 2;
//				zhon.y = height / 2;
//
//				double distToM = dist(jiao, zhon); currently we don't consider the position of the quadrangle, so it is commented out
				double angleSum = fabs(theta0 - theta1) + fabs(theta2 - theta3);//TODO this is a problem!That when angleSum is nearly PI...like the cat book!
				//int score = clines[0]->score+clines[1]->score+clines[2]->score+clines[3]->score;//blackScore[opplineVector.at(l).one]+blackScore[opplineVector.at(l).two]+blackScore[opplineVector.at(k).one]+blackScore[opplineVector.at(k).two];
				double score = 0;

				if (angleSum > CV_PI && !(2 * CV_PI - angleSum < CV_PI / 6))
					angleSum -= CV_PI;

				bool debug = false;
				if (k == 0 && l == 2)
					debug = true;

				Vec4i segs[4];
				segs[0] = lines1[pair1.one];
				segs[1] = lines1[pair1.two];
				segs[2] = lines1[pair2.one];
				segs[3] = lines1[pair2.two];
				//s3.3.5 filter and order the quadrangles with their likeliness to be real rectangles
				if (isLikeRect(clines, debug)
						&& isRealQuadr(pic1, xylines, segs, THRESHOLD[1],
								SIZE[1], score, debug, k, l, qn)
						&& (((angleSum < CV_PI / 6
										|| CV_PI - angleSum < CV_PI / 6
										|| 2 * CV_PI - angleSum < CV_PI / 6)
										&& maxScore <= score/*(distToM<minDistToM||distToM<20)&&(circ>1.0*mcirc||(circ>0.95*mcirc&&circ<mcirc))*/))) {
					finalK = k;
					finalL = l;
					mcirc = circ;
					minAngleSum = angleSum;
//					minDistToM = distToM;
					maxScore = score;
				}
			}
		}
	}

	if (finalK >= 0 && finalL >= 0) {

		collectCrossCands(tsrc, tslt, cross, qn, horiPairs, vertPairs, binary);
		//release the memory
		cvReleaseMemStorage(  & lines -> storage );
		lines = 0;
//		grad_x.release();
//		grad_y.release();
//		abs_grad_x.release();
//		abs_grad_y.release();
//		grad.release();
		return 0;
	}
//	grad_x.release();
//	grad_y.release();
//	abs_grad_x.release();
//	abs_grad_y.release();
//	grad.release();
	return -1;
}

int getBorderPtOnSalient(Mat src, vector<Point2f>& result){

	if(10*countNonZero(src)<src.cols*src.rows)
			return -1;
	vector<vector<Point2f> > crosses;
	Mat tsrc;
	myNormalSize(src, tsrc, CV_32FC3);

	process(tsrc, tsrc, crosses, true);
	if (crosses.size() > 0) {
		vector<Point2f> corners = crosses[0];
		for(int i=0;i<corners.size();i++){
			Point2f pNew;
			pNew.x = corners[i].x/scale;
			pNew.y = corners[i].y/scale;
			result.push_back(pNew);
		}
		return 0;
	}
}

int getBorderImgOnSalient(Mat orig, Mat src, Mat& cross, Mat& turned) {

	//too small salient, bad!
	if(10*countNonZero(src)<src.cols*src.rows)
		return -1;

	vector<vector<Point2f> > crosses;
	Mat tsrc, torig;
	myNormalSize(orig, torig, CV_32S);
	myNormalSize(src, tsrc, CV_32FC3);

	lighting = 180.0;
	curphase = 0;

	process(tsrc, tsrc, crosses, true);
	if (crosses.size() > 0) {
		vector<Point2f> corners = crosses[0];

		drawResult(torig, cross, corners);
		turnImage(orig, turned, corners, scale);
		return 0;
	} else {
		cross = orig;                //Mat::zeros(src.rows,src.cols,CV_32SC3);
		turned = orig;
//		Mat::zeros(src.rows, src.cols, CV_32SC3);
		return -1;
	}
}

int getBorderPtOnRaw(Mat src, Mat slt, vector<Point2f>& finalCorners){
	vector<vector<cv::Point2f> > cross_l;
	vector<vector<cv::Point2f> > cross_m;
	vector<vector<cv::Point2f> > cross_s;

	vector<vector<cv::Point2f> > crosses;
	Mat tsrc, tslt;
	myNormalSize(src, tsrc, CV_32S);
	myNormalSize(slt, tslt, CV_32F);
	crosses.clear();
	cross_l.clear();
	cross_m.clear();
	cross_s.clear();
	tLineScore.clear();
	tAreaScore.clear();
	tAnglScore.clear();
	tSpaceScore.clear();
	doubt = true;

	lighting = 180.0;
	curphase = 0;

	int result = process(tsrc, tslt, cross_l, false);

	if (doubt) {
		lighting = 110.0;
		curphase = 1;
		result = process(tsrc, tslt, cross_m, false);
	}
	if (doubt) {
		lighting = 40.0;
		curphase = 2;
		result = process(tsrc, tslt, cross_s, false);
	}
	for (int j = 0; j < 30 && j < cross_l.size(); j++) {
		crosses.push_back(cross_l[j]);

		tLineScore.push_back(lineScore[0][j]);
		tAreaScore.push_back(areaScore[0][j]);
		tAnglScore.push_back(anglScore[0][j]);
		tSpaceScore.push_back(spaceScore[0][j]);
	}

	for (int j = 0; j < 30 && j < cross_m.size(); j++) {
		crosses.push_back(cross_m[j]);

		tLineScore.push_back(lineScore[1][j]);
		tAreaScore.push_back(areaScore[1][j]);
		tAnglScore.push_back(anglScore[1][j]);
		tSpaceScore.push_back(spaceScore[1][j]);
	}

	for (int j = 0; j < 30 && j < cross_s.size(); j++) {
		crosses.push_back(cross_s[j]);

		tLineScore.push_back(lineScore[2][j]);
		tAreaScore.push_back(areaScore[2][j]);
		tAnglScore.push_back(anglScore[2][j]);
		tSpaceScore.push_back(spaceScore[2][j]);
	}

	if (crosses.size() == 0) {
		return -1;
	}

	for (int i = 0; i < 90; i++) {
		topRank[i] = i;
	}

	qsort(topRank, min(90, (int) crosses.size()), sizeof(int),
			compareTopScore);

	vector<cv::Point2f> corners;

	if (tAreaScore[topRank[0]] > 2) {
		corners = crosses[topRank[0]];
	} else {
		for (int i = 0; i < 30; i++) {
			finalRank[i] = i;
			spaceRank[i] = i;
			angleRank[i] = i;
		}
		//logic is finalscore = spacerank + anglerank!
		qsort(spaceRank, min(30, (int) crosses.size()), sizeof(int),
				compareSpaceScore);
		qsort(angleRank, min(30, (int) crosses.size()), sizeof(int),
				compareAngleScore);

		for (int i = 0; i < 30 && i < crosses.size(); i++) {
			//INDEX To RANK
			spaceRankDic[spaceRank[i]] = i;
			angleRankDic[angleRank[i]] = i;
		}

		qsort(finalRank, min(30, (int) crosses.size()), sizeof(int),
				compareFinalScore);
		corners = crosses[topRank[finalRank[0]]];
		for(int i=0;i<corners.size();i++){
			Point2f pNew;
			pNew.x = corners[i].x/scale;
			pNew.y = corners[i].y/scale;
			finalCorners.push_back(pNew);
		}
	}

	return 0;
}

int getBorderImgOnRaw(cv::Mat src, Mat slt, Mat& cross, Mat& turned) {
	vector<vector<cv::Point2f> > cross_l;
	vector<vector<cv::Point2f> > cross_m;
	vector<vector<cv::Point2f> > cross_s;

	vector<vector<cv::Point2f> > crosses;
	Mat tsrc, tslt;
	myNormalSize(src, tsrc, CV_32S);
	myNormalSize(slt, tslt, CV_32F);                //really?
	crosses.clear();
	cross_l.clear();
	cross_m.clear();
	cross_s.clear();
	tLineScore.clear();
	tAreaScore.clear();
	tAnglScore.clear();
	tSpaceScore.clear();
	doubt = true;

	lighting = 180.0;
	curphase = 0;

	int result = process(tsrc, tslt, cross_l, false);

	if (doubt) {
		lighting = 110.0;
		curphase = 1;
		result = process(tsrc, tslt, cross_m, false);
	}
	if (doubt) {
		lighting = 40.0;
		curphase = 2;
		result = process(tsrc, tslt, cross_s, false);
	}
	for (int j = 0; j < 30 && j < cross_l.size(); j++) {
		crosses.push_back(cross_l[j]);

		tLineScore.push_back(lineScore[0][j]);
		tAreaScore.push_back(areaScore[0][j]);
		tAnglScore.push_back(anglScore[0][j]);
		tSpaceScore.push_back(spaceScore[0][j]);
	}

	for (int j = 0; j < 30 && j < cross_m.size(); j++) {
		crosses.push_back(cross_m[j]);

		tLineScore.push_back(lineScore[1][j]);
		tAreaScore.push_back(areaScore[1][j]);
		tAnglScore.push_back(anglScore[1][j]);
		tSpaceScore.push_back(spaceScore[1][j]);
	}

	for (int j = 0; j < 30 && j < cross_s.size(); j++) {
		crosses.push_back(cross_s[j]);

		tLineScore.push_back(lineScore[2][j]);
		tAreaScore.push_back(areaScore[2][j]);
		tAnglScore.push_back(anglScore[2][j]);
		tSpaceScore.push_back(spaceScore[2][j]);
	}

	if (crosses.size() == 0) {
		cross = Mat::zeros(src.rows, src.cols, CV_32SC3);
		turned = Mat::zeros(src.rows, src.cols, CV_32SC3);
		return -1;
	}
	for (int i = 0; i < 90; i++) {
		topRank[i] = i;
	}

	qsort(topRank, min(90, (int) crosses.size()), sizeof(int),
			compareTopScore);
//		for(int i=0;i<90&&i<crosses.size();i++){
//			string js;
//			strstream ss2;
//			ss2<<i<<"_"<<tAreaScore[topRank[i]];
//			ss2>>js;
//			Mat dist;
//			//dumpShape(crosses[topRank[i2]]);
//			drawResult(tsrc, dist, crosses[topRank[i]]);
//			imwrite("/home/litton/test_result_score4/dump_"+js+".jpg",dist);
//		}

	vector<cv::Point2f> corners;

	if (tAreaScore[topRank[0]] > 2) {
		corners = crosses[topRank[0]];
	} else {
		for (int i = 0; i < 30; i++) {
			finalRank[i] = i;
			spaceRank[i] = i;
			angleRank[i] = i;
		}
		//logic is finalscore = spacerank + anglerank!
		qsort(spaceRank, min(30, (int) crosses.size()), sizeof(int),
				compareSpaceScore);
		qsort(angleRank, min(30, (int) crosses.size()), sizeof(int),
				compareAngleScore);

		for (int i = 0; i < 30 && i < crosses.size(); i++) {
			//INDEX To RANK
			spaceRankDic[spaceRank[i]] = i;
			angleRankDic[angleRank[i]] = i;
		}

		qsort(finalRank, min(30, (int) crosses.size()), sizeof(int),
				compareFinalScore);
		corners = crosses[topRank[finalRank[0]]];
	}

	drawResult(tsrc, cross, corners);
	turnImage(src, turned, corners, scale);

	return 0;
}

#endif
