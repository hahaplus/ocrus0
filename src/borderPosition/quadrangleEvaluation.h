#ifndef BORDER_QUADRA_EVAL_H
#define BORDER_QUADRA_EVAL_H

struct quadrNode {
	int k;
	int l;
	int score;
	friend bool operator<(quadrNode n1, quadrNode n2) {
		return n1.score < n2.score;
	}
};

//TODO detect the qudrangle a real one or fake one, with the continuing points
bool isRealQuadr(cv::Mat pic, cv::Vec4i xylines[], Vec4i lineSeg[], int thresh,
		int size, double& score, bool debug, int k, int l,
		priority_queue<quadrNode>& qn) {

	cv::Point2f pt[4];
	thresh = 7;
	size = 2;
	if (debug)
		cout << "[DEBUG] " << thresh << " " << size << endl;

	for (int n = 0; n < 4; n++) {
		pt[n] = computeLineIntersect(xylines[n / 2], xylines[2 + n % 2]);
		if (debug) {
			cout << "inter point " << n << ": " << pt[n].x << " " << pt[n].y
					<< endl;
		}
	}
	int THRESHOLD = thresh;
	int SIZE = size;
	double dd1, dd2, dd3, dd4;
	double dx1, dx2, dx3, dx4;
	if (debug)
		for (int i = 0; i < 4; i++) {
			cout << "xianduan " << i << ": " << lineSeg[i][0] << " "
					<< lineSeg[i][1] << " " << lineSeg[i][2] << " "
					<< lineSeg[i][3] << endl;
		}

	if (!isLine(pic, dd1, dx1, pt[0], pt[1], THRESHOLD, SIZE, 2, debug))
		return false;
	if (!isLine(pic, dd2, dx2, pt[2], pt[3], THRESHOLD, SIZE, 2, debug))
		return false;
	if (!isLine(pic, dd3, dx3, pt[0], pt[2], THRESHOLD, SIZE, 2, debug))
		return false;
	if (!isLine(pic, dd4, dx4, pt[1], pt[3], THRESHOLD, SIZE, 2, debug))
		return false;

	score = (dd1 + dd2 + dd3 + dd4);	///(dx1+dx2+dx3+dx4);
	//score = dd1/dx1+dd2/dx2+dd3/dx3+dd4/dx4;
	if (debug)
		cout << (dd1 + dd2 + dd3 + dd4) << " " << (dx1 + dx2 + dx3 + dx4)
				<< endl;

	//std::cout<<"true quadr"<<std::endl;
	quadrNode n;
	n.k = k;
	n.l = l;
	n.score = score;
	qn.push(n);
	return true;
}

bool toushi(CvLinePolar2* line1, CvLinePolar2* line2, bool debug) {

	CvLinePolar2* line_r;
	CvLinePolar2* line_l;
	bool find = false;

	if (debug)
		cout << "[debug toushi] " << line1->angle << " " << line2->angle
				<< endl;

	if (line1->angle
			>= 0&&line1->angle<=CV_PI/2+0.0001&&line2->angle>=CV_PI/2-0.0001&&line2->angle<=CV_PI) {

		if (debug)
			cout << "[debug toushi] TRUE" << endl;
		find = true;
		line_r = line1;
		line_l = line2;
	}

	if (line2->angle
			>= 0&&line2->angle<=CV_PI/2+0.0001&&line1->angle>=CV_PI/2-0.0001&&line1->angle<=CV_PI) {
		if (debug)
			cout << "[debug toushi] TRUE" << endl;
		find = true;
		line_r = line2;
		line_l = line1;
	}

	if (find) {
		double ang1 = CV_PI / 2 - line_r->angle;
		double ang2 = line_l->angle - CV_PI / 2;
//		if(debug)
//			cout<<"[debug toushi] ANG1 "<<ang1<<" ANG2 "<<ang2<<endl;
		if (fabs(ang1 - ang2) > CV_PI / 10)
			return false;
		else
			return true;
	}
	if (fabs(line1->angle - line2->angle) < 0.05)
		return true;
	return false;
}

bool shuzhi(CvLinePolar2* line1, CvLinePolar2* line2, bool debug) {

	CvLinePolar2* line_r;
	CvLinePolar2* line_l;
	bool find = false;
	bool find2 = false;
	bool find3 = false;

	if (debug)
		cout << "[debug shuzhi] " << line1->angle << " " << line2->angle
				<< endl;

	if (line1->angle
			>= 3 * CV_PI
					/ 2&&line1->angle<=2*CV_PI&&line2->angle>=3*CV_PI/2&&line2->angle<=2*CV_PI) {
		if ((2 * CV_PI - line1->angle) < CV_PI / 15
				&& (2 * CV_PI - line2->angle) < CV_PI / 15) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}
	if (line1->angle >= 0 && line1->angle <= CV_PI / 2 && line2->angle >= 0
			&& line2->angle <= CV_PI / 2) {
		if ((line1->angle) < CV_PI / 15 && (line2->angle) < CV_PI / 15) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}
	if (line1->angle
			>= CV_PI
					/ 2&&line1->angle<=CV_PI&&line2->angle>=3*CV_PI/2&&line2->angle<=2*CV_PI) {
		if ((CV_PI - line1->angle) < CV_PI / 15
				&& (2 * CV_PI - line2->angle) < CV_PI / 15) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}
	if (line2->angle
			>= CV_PI
					/ 2&&line2->angle<=CV_PI&&line1->angle>=3*CV_PI/2&&line1->angle<=2*CV_PI) {
		if ((CV_PI - line2->angle) < CV_PI / 15
				&& (2 * CV_PI - line1->angle) < CV_PI / 15) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}
	if (line1->angle >= 0 && line1->angle <= CV_PI / 2
			&& line2->angle >= 3 * CV_PI / 2 && line2->angle <= 2 * CV_PI
			&& line1->rho < line2->rho) {
		find = true;
		line_l = line1;
		line_r = line2;
	}

	if (line2->angle >= 0 && line2->angle <= CV_PI / 2
			&& line1->angle >= 3 * CV_PI / 2 && line1->angle <= 2 * CV_PI
			&& line2->rho < line1->rho) {
		find = true;
		line_l = line2;
		line_r = line1;
	}
	if (find) {
		double ang1 = line_l->angle;
		double ang2 = 2 * CV_PI - line_r->angle;

		if (fabs(ang1 - ang2) < CV_PI / 25) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}

	if (line1->angle >= 0 && line1->angle <= CV_PI / 2
			&& line2->angle >= 3 * CV_PI / 2 && line2->angle <= 2 * CV_PI
			&& line1->rho > line2->rho) {
		find2 = true;
		line_l = line2;
		line_r = line1;
	}

	if (line2->angle >= 0 && line2->angle <= CV_PI / 2
			&& line1->angle >= 3 * CV_PI / 2 && line1->angle <= 2 * CV_PI
			&& line2->rho > line1->rho) {
		find2 = true;
		line_l = line1;
		line_r = line2;
	}
	if (find2) {
		double ang1 = line_r->angle;
		double ang2 = 2 * CV_PI - line_l->angle;

		if (fabs(ang1 - ang2) < CV_PI / 25) {
			if (debug)
				cout << "[debug shuzhi] true" << endl;
			return true;
		}
	}
	//TODO some rare case exists

	if (debug)
		cout << "[debug shuzhi] false" << endl;
	return false;
}

bool isLikeRect(CvLinePolar2 ** clines, bool debug) {
	CvLinePolar2* line1 = clines[0];
	CvLinePolar2* line2 = clines[1];
	CvLinePolar2* line3 = clines[2];
	CvLinePolar2* line4 = clines[3];

	if (debug) {
		cout << "[debug like rect] " << line1->angle << " " << line2->angle
				<< " " << line3->angle << " " << line4->angle << endl;
	}

//	if(debug) cout<<"[debug like rect] TEST1"<<endl;
	if (verti(line1, debug) && verti(line2, debug)
			&& !toushi(line3, line4, debug)) {
		if (debug)
			cout << "[debug like rect] FALSE1" << endl;
		return false;
	}
//	if(debug) cout<<"[debug like rect] TEST2"<<endl;
	if (verti(line1, debug) && verti(line2, debug)
			&& toushi(line3, line4, debug)) {
		if (debug)
			cout << "[debug like rect] TRUE1" << endl;
		return true;
	}
//	if(debug) cout<<"[debug like rect] TEST3"<<endl;
	if (verti(line3, debug) && verti(line4, debug)
			&& !toushi(line1, line2, debug)) {
		if (debug)
			cout << "[debug like rect] FALSE2" << endl;
		return false;
	}
//	if(debug) cout<<"[debug like rect] TEST4"<<endl;
	if (verti(line3, debug) && verti(line4, debug)
			&& toushi(line1, line2, debug)) {
		if (debug)
			cout << "[debug like rect] TRUE2" << endl;
		return true;
	}
///**/if(debug) cout<<"[debug like rect] TEST5"<<endl;
	if (horiz(line1, debug) && horiz(line2, debug)
			&& !shuzhi(line3, line4, debug)) {
		if (debug)
			cout << "[debug like rect] FALSE3" << endl;
		return false;
	}
//	if(debug) cout<<"[debug like rect] TEST6"<<endl;
	if (horiz(line1, debug) && horiz(line2, debug)
			&& shuzhi(line3, line4, debug)) {
		if (debug)
			cout << "[debug like rect] TRUE3" << endl;
		return true;
	}
//	if(debug) cout<<"[debug like rect] TEST7"<<endl;
	if (horiz(line3, debug) && horiz(line4, debug)
			&& !shuzhi(line1, line2, debug)) {
		if (debug)
			cout << "[debug like rect] FALSE4" << endl;
		return false;
	}
//	if(debug) cout<<"[debug like rect] TEST8"<<endl;
	if (horiz(line3, debug) && horiz(line4, debug)
			&& shuzhi(line1, line2, debug)) {
		if (debug)
			cout << "[debug like rect] TRUE4" << endl;
		return true;
	}
	//if(sameDir(line1,line2)&&sameDir(line3,line4)&&(fabs(line1->angle-line2->angle)+fabs(line3->angle-line4->angle)>CV_PI/10))
	//	return false;

	if (debug)
		cout << "[debug like rect] TRUE" << endl;
	return true;
}

bool doubt = true;

//though it is a quadrangle, the shape should be good
bool doubtShape(vector<double> lineAngles, vector<cv::Point2f> corners, Mat slt, bool binary) {

	double d01;
	distance(corners[0], corners[1], d01);
	double d12;
	distance(corners[1], corners[2], d12);
	double d23;
	distance(corners[2], corners[3], d23);
	double d30;
	distance(corners[3], corners[0], d30);
	double d02;
	distance(corners[0], corners[2], d02);
	double d13;
	distance(corners[1], corners[3], d13);

	double ang0;
	getAng(d01, d30, d13, ang0);
	double ang1;
	getAng(d01, d12, d02, ang1);
	double ang2;
	getAng(d12, d23, d13, ang2);
	double ang3;
	getAng(d23, d30, d02, ang3);

//	std::ostringstream strs;
//	strs << ang0 <<"_"<<ang1<<"_"<<ang2<<"_"<<ang3;
//	std::string str = strs.str();
//
//	string angs = "_angs_"+str;
	if (min(d01, d23) < 50) {
//		reason = "R1"+angs;
		return true;
	}

	if (max(d30, d12) / min(d01, d23) > 2) {
//		reason = "R2"+angs;
		return true;
	}

	if (ang0 < 8 * CV_PI / 18 && ang3 < 8 * CV_PI / 18 && 2 * d12 < d30) {
//		reason ="R3"+angs;
		return true;
	}

	if (ang1 < CV_PI / 2 && ang2 < CV_PI / 2 && 2 * d30 < d12) {
//		reason ="R4"+angs;
		return true;
	}

	if (ang0 < CV_PI / 2 && ang1 < CV_PI / 2 && 2 * d23 < d01) {
//		reason = "R5"+angs;
		return true;
	}

	if (ang2 < CV_PI / 2 && ang3 < CV_PI / 2 && 2 * d01 < d23) {
//		reason = "R6"+angs;
		return true;
	}

	int acuteCount = countAcuteAngle(ang0, ang1, ang2, ang3);
	if (acuteCount == 1) {
		//reason = "R7"+angs;
		return true;
	}

	double HORITHRESH = CV_PI/18;
	if(fabs(angleToHori(lineAngles[0])-angleToHori(lineAngles[1]))>HORITHRESH)
		return true;

//	HORITHRESH = CV_PI/36;
//	double ACUTETHRESH = CV_PI/2;
//
//	if(fabs(angleToHori(lineAngles[0])-angleToHori(lineAngles[1]))<HORITHRESH&&(ang0<ACUTETHRESH&&ang2<ACUTETHRESH||ang1<ACUTETHRESH&&ang3<ACUTETHRESH))
//		return true;

	pair<float, float> pr = binary?coverage(corners, slt):pair<float,float>(1,1);
//	cout<<"PR "<<pr.first<<" "<<pr.second<<endl;
	//
	//	if(pr.first>0&&pr.first<0.9) return true;
	double areascore = calcAreaScore(pr.first,pr.second);
//	if(binary) cout<<"areascore: "<<areascore<<endl;
	if(binary&&(areascore<0.7||pr.first<=0.85||pr.second<0.85))
		return true;

	if (binary&&pr.first > 0.9 && pr.second > 0 && pr.second < 0.7) {
		return true;
	}
	if (pr.first > 0.85 && pr.second > 0.85)
		doubt = true;//TODO it is nonsense to force to run different light modes! it should be false logically
	//reason = "";
	anglScore[curphase][scoreCur[curphase]] = myAngleScore(lineAngles, ang0, ang1, ang2,
			ang3);

	areaScore[curphase][scoreCur[curphase]] = areascore;
	spaceScore[curphase][scoreCur[curphase]] = (d01 + d12 + d23 + d30);
	return false;
}

#endif
