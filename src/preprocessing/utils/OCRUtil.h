/*
 * OCRUtil.h
 *
 *  Created on: Jan 26, 2015
 *      Author: xxy
 */

#ifndef GAUSSIAN_SP_DENOISE_SRC_OCRUTIL_H_
#define GAUSSIAN_SP_DENOISE_SRC_OCRUTIL_H_

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <iostream>
#include "FileUtil.h"

using namespace cv;
using namespace std;
using namespace tesseract;

class OCRUtil
{
public:
	static string ocrFile(Mat& src, const string lang = "eng+jpn+chi_sim") {

		TessBaseAPI tess;

//		cout<<src.cols<<" "<<src.rows<<endl;
//		imshow("xxx",src);
//		waitKey();

		/*init*/
		tess.Init(NULL, lang.c_str(), OEM_TESSERACT_ONLY);
		/*
		 * we can also use configuration files for the tesseract, such as whether to use a dictionary
		 */
//		char* configs[2];
//		configs[0]="/home/litton/tessdata/configs/api_config";
//		configs[1]="/home/litton/tessdata/configs/logfile";
//		tess.Init("/home/litton/tessdata", lang.c_str(), OEM_DEFAULT, configs, 2, NULL, NULL, false);

		/*default page segmentation mode*/
		tess.SetPageSegMode(PSM_AUTO_ONLY);
		//tess.SetPageSegMode(PSM_SINGLE_BLOCK);// Currently, for English name card, we use this way!
		/* if we first use the binarization algorithm for ourself, we can change it as a bit-wise
		 * graph for more fast processing
		 */
//		int lenBits = src.cols%8==0?src.cols/8:src.cols/8+1;
//		int rem = src.cols%8;
//
//		uchar myInput[src.rows*lenBits];
////		time_t t1 = time(NULL);
//		for(int j=0;j<src.rows;j++){
//
//			for(int i=0;i<lenBits-1;i++){
//				myInput[j*lenBits+i]=0;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+1)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+2)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+3)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+4)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+5)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+6)!=0?1:0;
//				myInput[j*lenBits+i]*=2;
//				myInput[j*lenBits+i]|=src.at<uchar>(j,8*i+7)!=0?1:0;
////				if(j==0)
////				cout<<"BIT: "<<(int)myInput[j+8*i]<<endl;
//			}
//			myInput[j*lenBits+(lenBits-1)]=0;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=1?src.at<uchar>(j,8*(lenBits-1))!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=2?src.at<uchar>(j,8*(lenBits-1)+1)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=3?src.at<uchar>(j,8*(lenBits-1)+2)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=4?src.at<uchar>(j,8*(lenBits-1)+3)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=5?src.at<uchar>(j,8*(lenBits-1)+4)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=6?src.at<uchar>(j,8*(lenBits-1)+5)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0||rem>=7?src.at<uchar>(j,8*(lenBits-1)+6)!=0?1:0:0;
//			myInput[j*lenBits+(lenBits-1)]*=2;
//			myInput[j*lenBits+(lenBits-1)]|=rem==0?src.at<uchar>(j,8*(lenBits-1)+7)!=0?1:0:0;
////			if(j==0)
////			cout<<"BIT: "<<(int)myInput[j+8*(lenBits-1)]<<endl;
//		}

		/*for greyscale image*/
		tess.SetImage((uchar*) src.data, src.cols, src.rows, 1, src.cols);

		/*for color image*/
//		tess.SetImage((uchar*) src.data, src.cols, src.rows, 3, 3*src.cols);

		/*for leptopia pix image*/
//		tess.SetImage(src);

		/*for bit-wise binary image*/
//		tess.SetImage((uchar*)&myInput, src.cols, src.rows, 0, lenBits);

		/*set resolution, if we don't use pix image*/
//		tess.SetSourceResolution(100);

		/*get result*/
		char* out = tess.GetUTF8Text();

		/*get different segmentation of the photo*/
//		ResultIterator* itor = tess.GetIterator();
//		cout<<"iterations"<<endl;
//		while(itor->Next(RIL_BLOCK)){
//			cout<<"TYPE: "<<itor->BlockType()<<endl<<"CONTENT: "<<endl<<itor->GetUTF8Text(RIL_BLOCK)<<endl;
//		}

		/*clear up*/
		string s = string(out);
//		delete itor;
		tess.End();

		return s;
	}
	static void ocrDir(string srcDir, string dstDir, const string lang = "eng+jpn+chi_sim")
	{
		vector<string> files = FileUtil::getAllFiles(srcDir);
		for(unsigned int i = 0 ; i < files.size(); i++)
		{
			string inputPath = srcDir + "/" + files[i];
			cout<<inputPath<<endl;
			Mat src = imread(inputPath, IMREAD_GRAYSCALE);
			string text;// = ocrFile(src, lang);
			string outputPath = dstDir + "/" + FileUtil::getFileNameNoSuffix(files[i]) + ".txt";
			cout<<outputPath<<endl;
			FileUtil::writeToFile(text, outputPath);
		}
	}
};


#endif /* GAUSSIAN_SP_DENOISE_SRC_OCRUTIL_H_ */
