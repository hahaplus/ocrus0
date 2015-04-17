#ifndef BORDER_OUTPUT_GEN_H
#define BORDER_OUTPUT_GEN_H

void drawResult(Mat src, Mat& dist, vector<cv::Point2f> corners) {

	dist = src.clone();
	cv::circle(dist, corners[0], 3, CV_RGB(255, 0, 0), 6);
	cv::circle(dist, corners[1], 3, CV_RGB(0, 255, 0), 6);
	cv::circle(dist, corners[2], 3, CV_RGB(0, 0, 255), 6);
	cv::circle(dist, corners[3], 3, CV_RGB(255, 255, 255), 6);

	line(dist, corners[0], corners[1], CV_RGB(0, 255, 0), 4);
	line(dist, corners[1], corners[2], CV_RGB(0, 255, 0), 4);
	line(dist, corners[2], corners[3], CV_RGB(0, 255, 0), 4);
	line(dist, corners[3], corners[0], CV_RGB(0, 255, 0), 4);

}

#endif
