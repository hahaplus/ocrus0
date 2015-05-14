#ifndef BORDER_LINE_EVAL_H
#define BORDER_LINE_EVAL_H

bool hasPixel(cv::Mat& mat, int thresh) {
	int channels = mat.channels();
	int nRows = mat.rows * channels;
	int nCols = mat.cols;
	//cout<<"size: " << nRows * nCols<<endl;
	if (mat.isContinuous()) {
		nCols *= nRows;
		nRows = 1;
	}
	for (int i = 0; i < nRows; ++i) {
		uchar* p = mat.ptr<uchar>(i);
		for (int j = 0; j < nCols; ++j) {
			//cout<<"value: "<<p[j]<<endl;
			if (p[j] > thresh) {
				return true;
			}
		}
	}
	return false;
}

bool nosimilar(CvLinePolar2 line, CvSeq* seq) {
	for (int i = 0; i < seq->total; i++) {
		CvLinePolar2* line2 = (CvLinePolar2*) cvGetSeqElem(seq, i);
		if (fabs(line2->angle - line.angle) < CV_PI / 36
				&& fabs(line2->rho - line.rho) < 5) {
			//cout<<"found similar "<<line.score<<" "<<line2->score<<endl;
			if (line.score > line2->score) {
				line2->score = line.score;
				line2->x1 = line.x1;
				line2->y1 = line.y1;
				line2->x2 = line.x2;
				line2->y2 = line.y2;
				line2->angle = line.angle;
				line2->rho = line.rho;
				line2->votes = line.votes;
				CvLinePolar2* line3 = (CvLinePolar2*) cvGetSeqElem(lines, i);

				//cout<<"score change "<<line3->score<<endl;
			}
			return false;
		}
	}
	//cout<<"no similar "<<line.score<<endl;
	return true;
}

bool isLine(cv::Mat& mat, double& linkScore, double& linkSpace, cv::Point pt1,
		cv::Point pt2, int threshhold, int size, int mode, bool debug) {
	if (mat.channels() != 1) {
		CV_Error(CV_StsBadArg, "Mat should be gray image.");
	}
	if (size < 1) {
		CV_Error(CV_StsBadArg, "Size should be positive");
	}
	//std::cout<<"points "<<pt1.x<<"-"<<pt1.y<<","<<pt2.x<<"-"<<pt2.y<<std::endl;

	CvMat src = mat;
	cv::Mat dstMat = cv::Mat::zeros(size, size, CV_8UC1);
	CvMat dst = dstMat;
	CvLineIterator iterator;
	int count = cvInitLineIterator(&src, pt1, pt2, &iterator, 8);

	int space = 0;
	int maxSpace = 0;
	int link = 0;
	int maxLink = 0;

	int ag_x = -9999;
	int ag_y = -9999;
	int lr = 0;
	int tb = 0;
	int maxLr = 0;
	int maxTb = 0;
	int lastLv = -1;
	//cout<<"test grad "<<(int)grad_y.at<char>(843,0)<<" "<<(int)grad_y.at<char>(843,1)<<" "<<(int)grad_y.at<char>(843,2)<<" "<<(int)grad_y.at<char>(843,3)<<endl;
	//cout<<count<<endl;
	int lastx = -1;
	int lasty = -1;
	int cutcount = 0;
	while (--count) {
		CV_NEXT_LINE_POINT(iterator);

		/* print the pixel coordinates: demonstrates how to calculate the coordinates */
		int offset, x, y;
		/* assume that ROI is not set, otherwise need to take it into account. */
		offset = iterator.ptr - src.data.ptr;
		y = offset / src.step;
		x = (offset - y * src.step) / sizeof(uchar);

		int g_x = 0;
		int g_y = 0;

//		if (debug)
//			cout<<"x: " << x << ", y:" <<y<<endl;

		if (x == lastx && y == lasty)
			return false;

		lastx = x;
		lasty = y;

		//cout<<"gx: " << g_x << ", gy:" <<g_y<<endl;
		CvPoint2D32f ptr = cvPoint2D32f(x, y);
		cvGetRectSubPix(&src, &dst, ptr);

		cv::Mat I = cv::cvarrToMat(&dst);
		//cout<<"Mat: "<<I<<endl;
		int greyLv = mat.at<uchar>(ptr);
		if (!hasPixel(I, THRESHSCALE) || lastLv == -1
				|| abs(lastLv - greyLv) > 50) {
			space++;

			maxLink = std::max(maxLink, link);
			link = 0;
		} else {
			maxSpace = std::max(maxSpace, space);
			space = 0;
			if (link == 0)
				cutcount++;
			link++;
		}

		lastLv = greyLv;
	}
	//cout<<"xxxxx"<<endl;
	maxSpace = std::max(maxSpace, space);
	maxLink = max(maxLink, link);
	maxLr = max(maxLr, lr);
	maxTb = max(maxTb, tb);
	//cout<<"grad "<<maxLr<<" "<<maxTb<<endl;

	MAXLINK = 10;
	if (mode <= 2 && maxLink >= MAXLINK) {
		//if (debug) std::cout<<"true1 "<<maxLink<<std::endl;
		linkScore = maxLink;
		linkSpace = cutcount;		//maxSpace;
		if (linkSpace == 0)
			linkSpace = 1;
		return true;
	}
	if (maxSpace >= threshhold) {
		//if (debug) std::cout<<"false1 "<<maxSpace<<" "<<maxLink<<" "<<threshhold<<std::endl;
		return false;
	}
	//if (debug) std::cout<<"true2 "<<maxSpace<<" "<<maxLink<<std::endl;
	linkScore = maxLink;
	return true;
}

bool verti(CvLinePolar2* line, bool debug) {
	if (fabs(line->angle) < CV_PI / 24.0)
		return true;
	if (fabs(line->angle - CV_PI) < CV_PI / 24.0)
		return true;
	return false;
}

bool horiz(CvLinePolar2* line, bool debug) {
	if (fabs(line->angle - CV_PI / 2) < CV_PI / 36.0)
		return true;

	return false;
}

bool isVertLine(double ang){
	if(ang>=0&&ang<=CV_PI/4) return true;
	if(ang>=CV_PI*3/4&&ang<=CV_PI*5/4) return true;
	if(ang>=CV_PI*7/4&&ang<=CV_PI*2) return true;
	return false;
}

bool isHoriLine(double ang){
	if(ang>=CV_PI/4&&ang<=CV_PI*3/4) return true;
	if(ang>=CV_PI*5/4&&ang<=CV_PI*7/4) return true;
	return false;
}

int compareLineScore(const void * a, const void * b) {
	int ai = *(int*) a;
	int bi = *(int*) b;
	CvLinePolar2* l1 = (CvLinePolar2*) cvGetSeqElem(lines, ai);
	CvLinePolar2* l2 = (CvLinePolar2*) cvGetSeqElem(lines, bi);
	return -(l1->score - l2->score);
}

void sortLines(CvSeq* lines) {

	for (int i = 0; i < min(5000, lines->total); i++) {
		lineSorted[i] = i;
	}

	qsort(lineSorted, min(5000, lines->total), sizeof(int), compareLineScore);
}


CvSeq*
convertXYLineToPolar(std::vector<cv::Vec4i> lines0, CvMemStorage* storage,
		cv::Mat pic1, map<int, set<int> >& lineMap) {
	int count = 0;
	int lineType = CV_32FC(8);
	int elemSize = sizeof(float) * 8;

	lines1.clear();
	lines = cvCreateSeq(lineType, sizeof(CvSeq), elemSize, storage);
	int recMaxLink = MAXLINK;
	for (int i = 0; i < lines0.size(); i++) {
		CvLinePolar2 line;
		line.x1 = lines0[i][0];
		line.y1 = lines0[i][1];
		line.x2 = lines0[i][2];
		line.y2 = lines0[i][3];
		line.votes = 1;

		//cout<<"x-y: "<<line.x1<<","<<line.y1<<","<<line.x2<<","<<line.y2<<endl;
		if (lines0[i][1] == lines0[i][3]) {
			line.angle = lines0[i][1] >= 0 ? CV_PI / 2 : 3 * CV_PI / 2;
			line.rho = fabs(line.y1);
		} else {
			double k = -(line.x2 - line.x1) / (line.y2 - line.y1);
			double x3 = (line.x1 + k * line.y1) / (1 + k * k);
			double y3 = k * x3;
			line.rho = sqrt(x3 * x3 + y3 * y3);
			line.angle = atan(k);

			if (x3 > 0 && y3 > 0)
				;
			if (x3 < 0 && y3 < 0)
				line.angle = CV_PI + line.angle;
			if (x3 < 0 && y3 > 0)
				line.angle = CV_PI + line.angle;
			if (x3 > 0 && y3 < 0)
				line.angle = 2 * CV_PI + line.angle;

		}

		MAXLINK = 10;	//20;
		double linkScore = 0.0;
		double linkSpace = 0.0;
		if ((isHoriLine(line.angle)||isVertLine(line.angle))&&isLine(pic1, linkScore, linkSpace,
				cv::Point(lines0[i][0], lines0[i][1]),
				cv::Point(lines0[i][2], lines0[i][3]), 2, 1, 1, true)) {

			//cout<<"scores "<<linkScore<<" "<<sqrt((line.x1-line.x2)*(line.x1-line.x2)+(line.y1-line.y2)*(line.y1-line.y2))<<endl;
			line.score = linkScore
					+ sqrt(
							(line.x1 - line.x2) * (line.x1 - line.x2)
									+ (line.y1 - line.y2) * (line.y1 - line.y2))
							/ 5;
			if (nosimilar(line, lines)) {
				lines1.push_back(lines0[i]);
				cvSeqPush(lines, &line);
				//cout<<"lines increase "<<lines->total<<endl;
			}
		}
	}
	MAXLINK = recMaxLink;
	sortLines(lines);

	for(int i=0;i<lines->total;i++){
		CvLinePolar2* line1 = (CvLinePolar2*) cvGetSeqElem(lines, i);
		if(lineMap.find(i)==lineMap.end())
			lineMap[i] = set<int>();

		for(int j=i+1;j<lines->total;j++){
			CvLinePolar2* line2 = (CvLinePolar2*) cvGetSeqElem(lines, j);
			if(lineMap.find(j)==lineMap.end())
				lineMap[j] = set<int>();

			if (fabs(line2->angle - line1->angle) < CV_PI / 36){
				lineMap[i].insert(j);
				lineMap[j].insert(i);
			}
		}
	}

	return lines;
}

Vec4i polarToPoints(CvLinePolar2* polar){
	Vec4i res;
	float rho = polar->rho, theta = polar->angle;

	double a = cos(theta), b = sin(theta);
	double x0 = a * rho, y0 = b * rho;
	res[0] = cvRound(x0 + 1000 * (-b));
	res[1] = cvRound(y0 + 1000 * (a));
	res[2] = cvRound(x0 - 1000 * (-b));
	res[3] = cvRound(y0 - 1000 * (a));

	return res;
}

#endif
