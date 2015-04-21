/*
 * textExtraction.h
 *
 *  Created on: Feb 10, 2015
 *      Author: fc
 */

#ifndef TEXTEXTRACTION_H_
#define TEXTEXTRACTION_H_

#include <iostream>
#include <iterator>
#include <opencv2/opencv.hpp>

#include "RobustTextDetection.h"
#include "ConnectedComponent.h"
#include "lineFormation.h"
#include "../salientRecognition/pyramid/pyramid.h"

using namespace cv;
using namespace std;

class TextExtraction{
public:
	vector<Rect> textExtract(Mat &mat);
	void debug(Mat &originalImg, vector<Rect> regions, char* title);
	vector<Mat> findRegions(Mat &originalImg, vector<Rect> regions);
	vector<Mat> findMergedRegions(Mat &originalImg, vector<Rect> regions);
private:
	bool _debug;
};

vector<Rect> TextExtraction::textExtract(Mat &mat){

	Mat image1 = mat.clone();
	Pyramid pyramid(image1);
	Mat image = pyramid.scale(true);

	/* Quite a handful or params */
	RobustTextParam param;
	param.minMSERArea        = 10;
	param.maxMSERArea        = 2000;
	param.cannyThresh1       = 20;
	param.cannyThresh2       = 100;

	param.maxConnCompCount   = 10000;
	param.minConnCompArea    = 15;// origin 75
	param.maxConnCompArea    = 800;

	param.minEccentricity    = 0.1;
	param.maxEccentricity    = 0.995;
	param.minSolidity        = 0.4;
	param.maxStdDevMeanRatio = 0.5;

	/* Apply Robust Text Detection */
	/* ... remove this temp output path if you don't want it to write temp image files */
	string temp_output_path = ".";
	RobustTextDetection detector(param );
	pair<Mat, Rect> result = detector.apply( image );

	LineFormation lf;
	vector<Rect> rects = lf.findLines(result.first);
//	debug(image, rects, "scaledResult");
	for(unsigned int i = 0, len = rects.size(); i < len; ++i){
		Rect r = rects[i];
		rects[i] = pyramid.reScale(r);
	}
	return rects;
}

void TextExtraction::debug(Mat &originalImg, vector<Rect> regions, char * title){
	for(unsigned int i = 0, len = regions.size(); i < len; ++i){
		Rect r = regions[i];
		rectangle( originalImg, r, Scalar(0, 0, 255), 2);
	}
//	namedWindow(title);
//	imshow(title, originalImg);
}

bool inRect(int y, int x, int top, int left, int bottom, int right){

	if(y>=top&&y<=bottom&&x>=left&&x<=right)
		return true;
	return false;
}

bool hasCommonPart(Rect r1, Rect r2){

	int top1 = r1.y;
	int left1 = r1.x;
	int bottom1 = r1.y+r1.height;
	int right1 = r1.x+r1.width;

	int top2 = r2.y;
	int left2 = r2.x;
	int bottom2 = r2.y+r2.height;
	int right2 = r2.x+r2.width;

//	if(top1<=bottom2&&left1<=right2)
//		return true;
//
//	if(top2<=bottom1&&left2<=right1)
//		return true;
//
//	if(bottom1>=top2&&left1<=right2)
//		return true;
//
//	if(bottom2>=top1&&left2<=right1)
//		return true;

	if(inRect(top1, left1, top2, left2, bottom2, right2))
		return true;
	if(inRect(top1, right1, top2, left2, bottom2, right2))
		return true;
	if(inRect(bottom1, left1, top2, left2, bottom2, right2))
		return true;
	if(inRect(bottom1, right1, top2, left2, bottom2, right2))
		return true;

	return false;
}

int cluster[200];
vector<int> graph[200];

void dfs(int i, const int& size){
	cluster[i]=size+1;
	vector<int> neibors = graph[i];
//	cout<<"neibors: "<<neibors.size()<<endl;
	vector<int>::const_iterator itor ;
	itor = neibors.begin();
	while(itor!=neibors.end()){
		int neibor = *itor;
		if(cluster[neibor]==0)
			dfs(neibor,size);
		itor++;
	}
}

vector<Mat> TextExtraction::findMergedRegions(Mat &originalImg, vector<Rect> regions){

	int len = regions.size();
	len = len<200?len:200;

	for(unsigned int i = 0; i < len; ++i){
		graph[i].clear();
	}
	//merge rectangles that have common part
	//1. make graph
	for(unsigned int i = 0; i < len; ++i){

		for(unsigned int j = i+1; j < len; ++j){
			//if have common part, add an edge between the two
			Rect rect1 = regions[i];
			Rect rect2 = regions[j];

			if(hasCommonPart(rect1,rect2)){
				graph[i].push_back(j);
				graph[j].push_back(i);
			}
		}

	}

	//2. find connected sub-graphs
	for(int i=0;i<len;i++){
		cluster[i]=0;
	}

	int clusterSize = 0;
	for(int i=0;i<len;i++){
		if(cluster[i]==0){
			dfs(i,clusterSize);
			clusterSize ++;
		}
	}

//	for(int i=0;i<len;i++)
//		cout<<cluster[i]<<endl;
//	cout<<"phase2"<<endl;
	//3. merge connected rectangles
	vector<Rect> mRects;
	int mSize = 0;
	for(int i=0;i<len;i++){
		//have not stored this cluster in mRects
		int clusterId = cluster[i];
		Rect newRect = regions[i];
		if(mSize<clusterId){
			mRects.push_back(newRect);
			mSize++;
		}
		else{
			Rect curRect = mRects[clusterId-1];
			int newX, newY, newW, newH;

			newX = newRect.x<curRect.x?newRect.x:curRect.x;
			newY = newRect.y<curRect.y?newRect.y:curRect.y;

			newW = newRect.x+newRect.width>curRect.x+curRect.width?newRect.x+newRect.width-newX:curRect.x+curRect.width-newX;
			newH = newRect.y+newRect.height>curRect.y+curRect.height?newRect.y+newRect.height-newY:curRect.y+curRect.height-newY;

			curRect.x = newX;
			curRect.y = newY;
			curRect.width = newW;
			curRect.height = newH;
			mRects[clusterId-1] = curRect;
		}
	}
//	cout<<"phase3"<<endl;
	//4. finally generate target sub-images
	vector<Mat> mergedRegions(mSize);
	for(unsigned int i = 0; i < mSize; ++i){
//		cout<<"MREGION "<<i<<": "<<mRects[i].x<<" "<<mRects[i].y<<" "<<mRects[i].x+mRects[i].width<<" "<<mRects[i].y+mRects[i].height<<endl;
		mergedRegions[i] = Mat(originalImg, mRects[i]);
	}
//	cout<<"merged "<<len<<" "<<mSize<<endl;
	return mergedRegions;
}

vector<Mat> TextExtraction::findRegions(Mat &originalImg, vector<Rect> regions){
	vector<Mat> textRegions(regions.size());
	for(unsigned int i = 0, len = regions.size(); i < len; ++i){
		textRegions[i] = Mat(originalImg, regions[i]);
	}
	return textRegions;
}

#endif /* TEXTEXTRACTION_H_ */
