#ifndef BORDER_DATA_STRUCTURES_H
#define BORDER_DATA_STRUCTURES_H

typedef struct CvLinePolar2 {
	float x1, y1, x2, y2;
	float rho;
	float angle;
	float votes;
	float score;
} CvLinePolar2;

typedef struct OppositeLines {
	int one;
	int two;
} OppositeLines;

typedef struct Quadrangle {
	cv::Point2f a, b, c, d;
} Quadrangle;

int THRESHOLD[2] = { 2, 7 };
int SIZE[2] = { 3, 7 };
int RUN[4] = { 1, 2, 2, 1 };
int VOTERATE = 2;
double OPPOANG = 1.0 / 4;//1.0/6
int THRESHSCALE = 40; //210;
int MAXLINK = 20;

CvSeq* lines = 0;
double lighting = 110.0;
cv::Point2f center(0, 0);

cv::Mat grad_x, grad_y, grad_x0, grad_y0;
cv::Mat abs_grad_x, abs_grad_y, abs_grad_x0, abs_grad_y0;
cv::Mat grad, grad0;
std::vector<cv::Vec4i> finalines(4);

int lineSorted[5000];

vector<Vec4i> lines1;

#endif
