#ifndef BORDER_PICK_CROSSCANDS_H
#define BORDER_PICK_CROSSCANDS_H

void makeFinalLine(int index, CvLinePolar2* line, vector<double>& lineAngles){

	float rho = line->rho, theta = line->angle;
	cv::Point pt1, pt2;
	double a = cos(theta), b = sin(theta);
	double x0 = a * rho, y0 = b * rho;
	pt1.x = cvRound(x0 + 1000 * (-b));
	pt1.y = cvRound(y0 + 1000 * (a));
	pt2.x = cvRound(x0 - 1000 * (-b));
	pt2.y = cvRound(y0 - 1000 * (a));
	finalines[index][0] = pt1.x;
	finalines[index][1] = pt1.y;
	finalines[index][2] = pt2.x;
	finalines[index][3] = pt2.y;

	lineAngles.push_back(line->angle);
}

void makeFinalLines(vector<quadrNode>& top10, int i0,
		vector<OppositeLines>& horiPairs, vector<OppositeLines>& vertPairs, vector<double>& lineAngles){

	int finalK = top10[i0].k;
	int finalL = top10[i0].l;

//  Mat dist = src.clone();

	makeFinalLine(0,(CvLinePolar2*) cvGetSeqElem(lines, horiPairs[finalK].one), lineAngles);
	//cv::line( dist, pt1, pt2, CV_RGB(0,255,0),4);double areaScore[3][30];
	//std::cout<<"line "<<line->angle<<" "<<line->rho<<std::endl;

	makeFinalLine(1,(CvLinePolar2*) cvGetSeqElem(lines, horiPairs[finalK].two), lineAngles);
	//cv::line( dist, pt1, pt2, CV_RGB(0,255,0),4);
	//std::cout<<"line "<<line->angle<<" "<<line->rho<<std::endl;

	makeFinalLine(2,(CvLinePolar2*) cvGetSeqElem(lines, vertPairs[finalL].one), lineAngles);
	//cv::line( dist, pt1, pt2, CV_RGB(0,255,0),4);
	//std::cout<<"line "<<line->angle<<" "<<line->rho<<std::endl;

	makeFinalLine(3,(CvLinePolar2*) cvGetSeqElem(lines, vertPairs[finalL].two), lineAngles);
	//cv::line( dist, pt1, pt2, CV_RGB(0,255,0),4);
}

void makeCorners(vector<cv::Point2f>& corners, Mat& src){
	corners.clear();
	for (int i = 0; i < finalines.size(); i++) {
		for (int j = i + 1; j < finalines.size(); j++) {
			cv::Point2f pt = computeLineIntersect(finalines[i], finalines[j]);

			if (pt.x < 0 && pt.x > -10)
				pt.x = 0;
			if (pt.y < 0 && pt.y > -10)
				pt.y = 0;
			if (pt.x > src.cols && pt.x < src.cols + 10)
				pt.x = src.cols;
			if (pt.y > src.rows && pt.y < src.rows + 10)
				pt.y = src.rows;
			if (pt.x >= 0 && pt.y >= 0 && pt.x <= src.cols
					&& pt.y <= src.rows) {

				corners.push_back(pt);
			}

		}
	}
}

bool validateCorners(vector<cv::Point2f>& corners, Mat& slt, bool binary, vector<double> lineAngles){
	std::vector<cv::Point2f> approx;
	cv::approxPolyDP(cv::Mat(corners), approx,
			cv::arcLength(cv::Mat(corners), true) * 0.02, true);

	if (approx.size() != 4) {
		std::cout << "The object is not quadrilateral!" << std::endl;
		return false;	//return;
	}

	center.x = 0.0;
	center.y = 0.0;
	// Get mass center
	for (int i = 0; i < corners.size(); i++)
		center += corners[i];
	center *= (1. / corners.size());

	sortCorners(corners, center);
//	cout<<"center "<<center.x<<" "<<center.y<<endl;
//	for(int i=0;i<4;i++){
//		cout<<"corner "<<corners[i].x<<" "<<corners[i].y<<endl;
//	}
	if (corners.size() == 0) {
		std::cout << "The corners were not sorted correctly!" << std::endl;
		return false;	//return;
	}

	bool doubtThis = doubtShape(lineAngles,corners, slt, binary);//binary ? false : doubtShape(corners, slt);
	//		if (doubtThis)
	//		{	doubtCount++;//doubts.push_back(corners);//(dist);
	//          reason.push_back(myreason);}
	return !doubtThis;
}

//from the queue, to get candidates to result crosses
void collectCrossCands(Mat& src, Mat& slt, vector<vector<cv::Point2f> >& crosses,
		priority_queue<quadrNode>& qn, vector<OppositeLines> horiPairs, vector<OppositeLines> vertPairs,
		bool binary) {

	std::vector<cv::Point2f> corners;
	vector<double> lineAngles;
//	cout<<"size "<<src.cols<<" "<<src.rows<<endl;
//	cout<<"qn size "<<qn.size()<<endl;

	int mscore = qn.top().score;

	vector<quadrNode> top10;

	for (int i = 0; qn.size() > 0 && (binary? i<3:(i < 20 || qn.top().score > mscore / 3));
			i++) {
		top10.push_back(qn.top());
		qn.pop();
	}

	while (!qn.empty())
		qn.pop();

//	int doubtCount = 0;
	int i0 = 0;
	for (; i0 < top10.size() && i0 < 30; i0++) {
		lineAngles.clear();
		makeFinalLines(top10, i0,horiPairs,vertPairs, lineAngles);
		makeCorners(corners, src);
		bool valid = validateCorners(corners, slt, binary, lineAngles);

		//cout<<"score "<<top10[i0].score<<endl;
		if (valid){
			crosses.push_back(corners);		//(dist);
			//spaceScore[curphase][scoreCur[curphase]]=((int)top10[i0].score);
			lineScore[curphase][scoreCur[curphase]++] = ((int) top10[i0].score);
			//cout<<"phase-cur: "<<scoreCur[curphase]<<endl;
			if (binary)
				return;
		}
	}
}

#endif
