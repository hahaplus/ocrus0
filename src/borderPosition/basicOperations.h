#ifndef BORDER_BASIC_OPRATIONS_H
#define BORDER_BASIC_OPRATIONS_H

double dotProduct(double* v1, double* v2, int c) {
	double ret = 0.0;
	for (int i = 0; i < c; i++)
		ret += v1[i] * v2[i];
	//std::cout<<"diancheng "<<ret<<std::endl;
	return ret;
}

void crossProduct(double* v1, double* v2, double* dist, int c) {
	dist[0] = (v1[1] * v2[2] - v1[2] * v2[1]);
	dist[1] = (v1[2] * v2[0] - v1[0] * v2[2]);
	dist[2] = (v1[0] * v2[1] - v1[1] * v2[0]);
	//std::cout<<"chacheng: "<<dist[0]<<", "<<dist[1]<<", "<<dist[2]<<std::endl;
}

void distance(Point2f p1, Point2f p2, double& db) {
	db = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

cv::Point2d computeLineIntersect(cv::Vec4i a, cv::Vec4i b) {
	double x1 = a[0] + 0.0, y1 = a[1] + 0.0, x2 = a[2] + 0.0, y2 = a[3] + 0.0,
			x3 = b[0] + 0.0, y3 = b[1] + 0.0, x4 = b[2] + 0.0, y4 = b[3] + 0.0;
	//std::cout<<"x1: "<<x1<<",y1: "<<y1<<",x2: "<<x2<<",y2: "<<y2<<",x3: "<<x3<<",y3: "<<y3<<",x4: "<<x4<<",y4: "<<y4<<std::endl;
	float denom;

	if (double d = ((double) (x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
		cv::Point2d pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4)
				- (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4)
				- (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		//std::cout<<"see: "<<(x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)<<std::endl;
		//std::cout<<"intersect: ("<<pt.x<<","<<pt.y<<")"<<std::endl;
		return pt;
	} else
		return cv::Point2f(-12345, -12345);
}

//triangle abc to calculate angle c
void getAng(double a, double b, double c, double& db) {
	db = acos((a * a + b * b - c * c) / (2 * a * b));
}

int countAcuteAngle(double ang0, double ang1, double ang2, double ang3, double thresh= 6 * CV_PI / 18) {

	//60 is too small...70 is ok!
	int ret = 0;
	if (ang0 < thresh)
		ret++;
	if (ang1 < thresh)
		ret++;
	if (ang2 < thresh)
		ret++;
	if (ang3 < thresh)
		ret++;
	return ret;
}

void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center) {
	std::vector<cv::Point2f> top, bot;

	for (int i = 0; i < corners.size(); i++) {
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}

	if (top.size() == 2 && bot.size() == 2) {
		corners.clear();
		cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
		cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
		cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
		cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

		corners.push_back(tl);
		corners.push_back(tr);
		corners.push_back(br);
		corners.push_back(bl);
	} else {
		std::vector<cv::Point2f> left, right;

		for (int i = 0; i < corners.size(); i++) {
			if (corners[i].x < center.x)
				left.push_back(corners[i]);
			else
				right.push_back(corners[i]);
		}
		corners.clear();
		if (left.size() == 2 && right.size() == 2) {
			cv::Point2f tl = left[0].y > left[1].y ? left[1] : left[0];
			cv::Point2f tr = right[0].y > right[1].y ? right[1] : right[0];
			cv::Point2f bl = left[0].y > left[1].y ? left[0] : left[1];
			cv::Point2f br = right[0].y > right[1].y ? right[0] : right[1];

			corners.push_back(tl);
			corners.push_back(tr);
			corners.push_back(br);
			corners.push_back(bl);
		}
	}
}

void pointToVecP(cv::Point2f pt, double* v3) {
	//double v3[3];
	v3[0] = pt.x;
	v3[1] = pt.y;
	v3[2] = 1.0;
	//return v3;
}

double dist(cv::Point2d p1, cv::Point2d p2) {
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

double scale = 1.0;

void myNormalSize(Mat& src, Mat& tsrc, int type) {

	double bili =
			src.cols > src.rows ?
					(src.cols > 500 ? 500.0 / src.cols : 1) :
					(src.rows > 500 ? 500.0 / src.rows : 1);
	Size sz = Size(src.cols * bili, src.rows * bili);
	tsrc = Mat(sz, type);
	cv::resize(src, tsrc, sz);
	scale = bili;
}

//whether two lines have same direction
bool sameDir(CvLinePolar2* line1, CvLinePolar2* line2) {

	if (line1->angle >= 0 && line1->angle <= CV_PI / 2 + 0.00001
			&& line2->angle >= 0 && line2->angle <= CV_PI / 2 + 0.00001)
		return true;

	if (line1->angle
			>= CV_PI / 2
					+ 0.00001&&line1->angle<=CV_PI&&line2->angle>=CV_PI/2+0.00001&&line2->angle<=CV_PI)
		return true;

	if (line1->angle >= CV_PI && line1->angle <= 3 * CV_PI / 2
			&& line2->angle >= CV_PI && line2->angle <= 3 * CV_PI / 2)
		return true;

	if (line1->angle
			>= CV_PI * 3
					/ 2&&line1->angle<=2*CV_PI&&line2->angle>=3*CV_PI/2&&line2->angle<=2*CV_PI)
		return true;

	return false;
}

double angleToVert(double angle){
	if(angle>=0&&angle<=CV_PI/2+0.0001)
		return angle;
	if(angle>CV_PI/2&&angle<=CV_PI*3/2+0.0001)
		return fabs(CV_PI-angle);
	return 2*CV_PI - angle;
}

double angleToHori(double angle){
	return fabs(CV_PI/2 -angleToVert(angle));
}

//calculate the angle of a line, when set the center of photo as (0,0) point
double normalizeAngle(CvLinePolar2* line, int w, int h) {
	double rho = line->rho;
	double theta0 = line->angle;
	if (rho < 0) {
		rho = -rho;
		theta0 += CV_PI;
	}
	double dx = w / 2.0, dy = h / 2.0;
	double xjd = rho / (cos(theta0) + dy * sin(theta0) / dx);
	double res = theta0;
	//if(k==0&&l==13) std::cout<<"xjd! "<<xjd<<std::endl;

	if (xjd >= 0 && xjd < dx) {

		//if(k==0&&l==13)
		//	std::cout<<"intersect! "<<std::endl;

		if (fabs(CV_PI - theta0) <= 0.0000001)
			res = CV_PI;
		if (theta0 < CV_PI)
			res = theta0 + CV_PI;
		else
			res = theta0 - CV_PI;
	}

	//if(k==0&&l==13)
	//std::cout<<"normailize "<<res<<std::endl;

	return res;
}

#endif
