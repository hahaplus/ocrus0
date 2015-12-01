/*
 * ShadowRemove.cpp
 *
 *  Created on: Nov 26, 2015
 *      Author: michael
 */

#include "ShadowRemove.h"

ShadowRemove::ShadowRemove() {
	// TODO Auto-generated constructor stub

}

ShadowRemove::~ShadowRemove() {
	// TODO Auto-generated destructor stub
}
void ShadowRemove::removeShadow(Mat& mat)
{
	Mat rgbImg = mat.clone();
	rgb2Ycrcb(mat);
	//Mat ymat = getYFromYcrcb(mat);
	//namedWindow("xx",CV_WINDOW_NORMAL);
	//imshow("xx", ymat);
	//waitKey();
	// get average intensity
	int cnt_iter = 3;
	while (cnt_iter--){
	double sum = 0;
	for (int i = 0; i < mat.rows; i++)
		for (int j = 0; j < mat.cols; j++)
		{
			Vec3b val = mat.at<Vec3b>(Point(i, j));
            sum += val[0];
		}
    double avg = sum/(mat.rows * mat.cols);
    double nonshadow = 0;
    int nonshadowCnt = 0;

    for (int i = 0; i < mat.rows; i++)
    		for (int j = 0; j < mat.cols; j++)
    		{
    			Vec3b val = mat.at<Vec3b>(Point(j, i));
    			if (!(val[0] < 0.9*avg && val[0] > 0.4 * avg))
    			{
    				nonshadow += val[ 0 ];
    				nonshadowCnt++;
    			}
    		}
    for (int i = 0; i < mat.rows; i++)
        		for (int j = 0; j < mat.cols; j++)
        		{
        			Vec3b &val = mat.at<Vec3b>(Point(j, i));
        			if ((val[0] < 0.9*avg && val[0] > 0.4 * avg))
        			{
        				double k = nonshadow / nonshadowCnt / val[ 0 ];
        				val[0] *= k;
        				Vec3b &rgbPixel = rgbImg.at<Vec3b>(Point(j,i));
        				rgbPixel *= (k + 1)/(0.3*k+1);
        			}
        		}

	}
    //mat = rgbImg;
	ycrcb2Rgb(mat);
    //namedWindow("xx",CV_WINDOW_NORMAL);

    //imshow("xx", mat);
    //waitKey();

}
Mat ShadowRemove::getYFromYcrcb(Mat& mat)
{
	IplImage ycb = IplImage(mat);
	IplImage* y=cvCreateImage(cvGetSize(&ycb),8,1);
    IplImage* cr=cvCreateImage(cvGetSize(&ycb),8,1);
    IplImage* cb=cvCreateImage(cvGetSize(&ycb),8,1);
	cvSplit(&ycb,y,cr,cb,0);
	return cvarrToMat(y);
}
void ShadowRemove::rgb2Ycrcb(Mat& mat)
{
	 IplImage src = IplImage(mat);
	 IplImage* ycb=cvCreateImage(cvGetSize(&src),8,3);
	 cvCvtColor(&src,ycb,CV_BGR2YCrCb);
     Mat ymat = cvarrToMat(ycb);
	 //ymat.data = y->imageData;
	 //namedWindow("xx",CV_WINDOW_NORMAL);
	 //imshow("xx", ymat);
	 //waitKey();
	 //mat.release();
	 mat = ymat;
}
void ShadowRemove::ycrcb2Rgb(Mat& mat)
{
	 IplImage src = IplImage(mat);
	 IplImage* rgb=cvCreateImage(cvGetSize(&src),8,3);
	 cvCvtColor(&src,rgb,CV_YCrCb2BGR);
	 Mat ymat = cvarrToMat(rgb);
		 //ymat.data = y->imageData;
		 //namedWindow("xx",CV_WINDOW_NORMAL);
		 //imshow("xx", ymat);
		 //waitKey();
		 //mat.release();
	 mat = ymat;
}
