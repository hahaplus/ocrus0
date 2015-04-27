#ifndef BORDER_QUADRA_CORRECTION_H
#define BORDER_QUADRA_CORRECTION_H

void turnImage(Mat& src, Mat& turned, vector<Point2f> corners, double scale) {
	/**/
	for (int i = 0; i < 4; i++) {
		corners[i].x /= scale;
		corners[i].y /= scale;
	}

	//turn the angle
	double u0 = src.cols / 2.0;
	double v0 = src.rows / 2.0;

	double mp1[3];
	pointToVecP(corners[3], mp1);
	double mp2[3];
	pointToVecP(corners[2], mp2);
	double mp3[3];
	pointToVecP(corners[0], mp3);
	double mp4[3];
	pointToVecP(corners[1], mp4);

	double cha14[3], cha24[3], cha34[3];
//	cout<<"mp1: "<<mp1[0]<<", "<<mp1[1]<<", "<<mp1[2]<<endl;
//	cout<<"mp2: "<<mp2[0]<<", "<<mp2[1]<<", "<<mp2[2]<<endl;
//	cout<<"mp3: "<<mp3[0]<<", "<<mp3[1]<<", "<<mp3[2]<<endl;
//	cout<<"mp4: "<<mp4[0]<<", "<<mp4[1]<<", "<<mp4[2]<<endl;
	crossProduct(mp1, mp4, cha14, 3);
	crossProduct(mp2, mp4, cha24, 3);
	crossProduct(mp3, mp4, cha34, 3);
	double k2 = dotProduct(cha14, mp3, 3) / dotProduct(cha24, mp3, 3);
	double k3 = dotProduct(cha14, mp2, 3) / dotProduct(cha34, mp2, 3);

//	std::cout<<"k2k3 "<<k2<<" "<<k3<<std::endl;
	double n2[3], n3[3];
	for (int i = 0; i < 3; i++) {
		n2[i] = k2 * mp2[i] - mp1[i];
		n3[i] = k3 * mp3[i] - mp1[i];
	}
//	cout<<"n2: ";
//	for(int i=0;i<3;i++)
//		cout<<n2[i]<<",";
//	cout<<endl;
//	cout<<"n3: ";
	for (int i = 0; i < 3; i++)
		cout << n3[i] << ",";
	cout << endl;
	double fk1 = -(1.0 / (n2[2] * n3[2]));
	double fk2 = n2[0] * n3[0] - (n2[0] * n3[2] + n2[2] * n3[0]) * u0
			+ n2[2] * n3[2] * u0 * u0;
	double fk3 = n2[1] * n3[1] - (n2[1] * n3[2] + n2[2] * n3[1]) * v0
			+ n2[2] * n3[2] * v0 * v0;
	double f2 = fk1 * (fk2 + fk3);

//	std::cout<<"f2 "<<f2<<std::endl;

	double bl1 = (n2[0] - n2[2] * u0) * n2[0] + (n2[1] - n2[2] * v0) * n2[1]
			+ (u0 * u0 + v0 * v0 + f2) * n2[2] * n2[2]
			- (u0 * n2[0] + v0 * n2[1]) * n2[2];
	double bl2 = (n3[0] - n3[2] * u0) * n3[0] + (n3[1] - n3[2] * v0) * n3[1]
			+ (u0 * u0 + v0 * v0 + f2) * n3[2] * n3[2]
			- (u0 * n3[0] + v0 * n3[1]) * n3[2];

	double factor = 2;

	double bl0 = max(fabs(mp1[0] - mp2[0]), fabs(mp3[0] - mp4[0]))
			/ max(fabs(mp1[1] - mp3[1]), fabs(mp2[1] - mp4[1]));
	double bl = bl0;

	if (bl1 * bl2 > 0)
		bl = sqrt(bl1 / bl2);

	if (bl0 > 0 && bl < 0 || bl0 < 0 && bl > 0)
		bl = bl0;

	if (bl > 3 || bl < 0.3)
		bl = bl0;

	std::cout << "bl0 " << bl0 << ",bl1 " << bl1 << ",bl2 " << bl2 << ",bl "
			<< bl << std::endl;

	int width = src.cols > bl * src.rows ? src.cols : bl * src.rows;
	int height = (int) (width / bl);

	if(width>src.cols){
		float scale = (0.0+src.cols)/width;
		width *= scale;
		height *= scale;
	}

	if(height>src.rows){
		float scale = (0.0+src.rows)/height;
		width *= scale;
		height *= scale;
	}

	cv::Mat quad = cv::Mat::zeros(height, width, CV_8UC3);
	std::vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
	quad_pts.push_back(cv::Point2f(0, quad.rows));

	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
	cv::warpPerspective(src, quad, transmtx, quad.size());
	turned = quad.clone();
}

#endif
