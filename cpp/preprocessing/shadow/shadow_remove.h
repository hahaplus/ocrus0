/*
 * ShadowRemove.h
 *
 *  Created on: Nov 26, 2015
 *      Author: michael
 */

#ifndef SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_
#define SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_

#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <set>
#include <string.h>
#include <stdlib.h>

class ShadowRemove {
public:
	// Mat is a rgb image
	static void removeShadow(cv::Mat&);

	virtual ~ShadowRemove();
private:
	static void rgb2Ycrcb(cv::Mat&);
	static void ycrcb2Rgb(cv::Mat&);
	static cv::Mat getYFromYcrcb(cv::Mat& mat);
    ShadowRemove();
};

#endif /* SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_ */
