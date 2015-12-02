/*
 * ShadowRemove.h
 *
 *  Created on: Nov 26, 2015
 *      Author: michael
 */

#ifndef SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_
#define SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <set>
#include "opencv2/core/utility.hpp"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../cca/CCA.h"

class ShadowRemove {
public:
	// Mat is a rgb image
	static void removeShadow(Mat&);

	virtual ~ShadowRemove();
private:
	static void rgb2Ycrcb(Mat&);
	static void ycrcb2Rgb(Mat&);
	static Mat getYFromYcrcb(Mat& mat);
    ShadowRemove();
};

#endif /* SRC_PREPROCESSING_SHADOW_SHADOW_REMOVE_H_ */
