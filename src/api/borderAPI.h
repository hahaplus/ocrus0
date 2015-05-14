#ifndef IMAGE_BORDER_API_H_
#define IMAGE_BORDER_API_H_

#include "../salientRecognition/execute.h"
#include "../borderPosition/border.h"

using namespace std;
using namespace cv;

static vector<Point2f> getBorder(Mat img, map<int, vector<Vec4i> >& lines){

	SalientRec src;
	Mat salientImg, seg;

	src.salient(img, salientImg, seg);
	vector<Point2f> result;

	int res = getBorderPtOnSalient(salientImg, result, lines);
	if (res == -1) {
		res = getBorderPtOnRaw(img, salientImg, result, lines);
	}
	if(res!=-1)
		return result;
	result.clear();
	return result;
}

static void transform(Mat& src, Mat& dst, vector<Point2f> corners){
	turnImage(src, dst, corners, 1);
}

#endif
