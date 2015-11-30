////#include "workflow/processor.h"
////int Processor::time01 = 0;
////int Processor::time02 = 0;
////int Processor::time1 = 0;
////int Processor::time2 = 0;
////int Processor::time3 = 0;
////int Processor::time4 = 0;
////int Processor::nopic = 0;
////#include "textExtraction/textExtraction.h"
//#include "preprocessing/utils/OCRUtil.h"
//#include <leptonica/allheaders.h>
////#include "preprocessing/shadow/fixshadow.h"
////#include "textDetect/textorient.h"
////#include "simpleNLP/contact.h"
////#include "api/borderAPI.h"
////#include "simpleNLP/namecardPost.h"
//#include "simpleNLP/textClassifier.h"
//
////using namespace std;
////using namespace cv;
//
//int main(int argc, char** argv){
//
////Name card test
////string input = ", , , , ,iﬁ—ﬁ\nReggie Chen\nAccount Manager\nM: +61410134 388\nT; +61 2 9929 3837\nF: +61 2 9929 3490\nE: regglechenéhltuomau\nW: www.hllv.com.au\nA: Suite 502. 275 Amed Street.\nNorth Macy, NSWZOGO Austnlia\nR0. Bax 1391mm 8160:1208)";
////correctNameCard(input);
//
////Regex test
//
////	if(isPhoneNumber("+86-152-6856-7302"))
////		cout<<"is phone number"<<endl;
////	else
////		cout<<"not phone number"<<endl;
////
////	if(isPhoneNumber("(0571)87951647"))
////		cout<<"is phone number"<<endl;
////	else
////		cout<<"not phone number"<<endl;
////	//+86-152-6856-7302
////	if(isEmail("zhangli85@zju.edu.cn"))
////		cout<<"is email"<<endl;
////	else
////		cout<<"not email"<<endl;
//
////process test
////	Processor::process_main(argc,argv);
//
///* Below is some research on resolution impact on the OCR result*/
///* 1. use opencv imread, the DPI information is lost!*/
//
////	Mat src = imread("/home/litton/notebook_demo.jpg");
////	Mat src = imread("/home/litton/table1.png");
////	cout<<OCRUtil::ocrFile(src,"eng")<<endl;
//
///* 2. Use leptonica image reading, it can rescale the DPI!*/
//
////	Pix *image1 = pixRead("/home/litton/cpp/snapnote/images/turn/NameCard2.jpg");
////	cout<<OCRUtil::ocrFile(image1,"eng")<<endl;
//
////	Pix *image2 = pixRead("/home/litton/cpp/snapnote/images/turn/NameCard1.jpg");
////	cout<<OCRUtil::ocrFile(image2,"chi_sim")<<endl;
//
///* We have three choices to solve the DPI's impact on OCR
// * Choice 1: record the DPI when taking photo, which can be calculated with phone screen size and photosize
// * Choice 2: after preprocessing, save the result at first, then use leptonica to read it again
// * Choice 3: use 72 as default resolution
// */
//
///* Below is testing code for shadow fix*/
////	Mat src = imread("/home/litton/upload/shadow.jpg");
////	Mat bg;
////	getBackground(src,bg);
////
////	Mat res;
////
//////	refineBackgroud(bg, res1, NOT_FIX_HOLE);
////	refineBackgroud(bg, res, AVER_FIX);
//////	res1.convertTo(res1,CV_8U);
////	res.convertTo(res,CV_8U);
//////	imshow("res1",res1);
////	imshow("res",res);
////	waitKey();
//
////Make the training and testing data of text classifier
//	DIR *dir;
//	struct dirent *ent;
//	dataCount = 0;
//
//	if ((dir = opendir ("/home/litton/upload/namecard/train")) != NULL) {
//
//	  /* print all the files and directories within directory */
//	  while ((ent = readdir (dir)) != NULL) {
//		  string name(ent->d_name);
//		  if(name!="."&&name!=".."){
//			  string fullname = "/home/litton/upload/namecard/train/"+name;
//			  Pix *image1 = pixRead(fullname.c_str());
//		  	  string s = OCRUtil::ocrFile(image1,"eng");
//		  	  prepareVector(s, 1);
//		  }
//	  }
//	}
//	if ((dir = opendir ("/home/litton/upload/papers/train")) != NULL) {
//
//	  /* print all the files and directories within directory */
//	  while ((ent = readdir (dir)) != NULL) {
//		  string name(ent->d_name);
//		  if(name!="."&&name!=".."){
//			  string fullname = "/home/litton/upload/papers/train/"+name;
//			  Pix *image1 = pixRead(fullname.c_str());
//			  string s = OCRUtil::ocrFile(image1,"eng");
//			  prepareVector(s, 0);
//		  }
//	  }
//	}
//
//
//	LogisticRegression<> classifier = trainClassifier();
//	cout<<classifier.Parameters()<<endl;
//
//	if ((dir = opendir ("/home/litton/upload/namecard/test")) != NULL) {
//
//	  /* print all the files and directories within directory */
//	  while ((ent = readdir (dir)) != NULL) {
//		  string name(ent->d_name);
//		  if(name!="."&&name!=".."){
//			  string fullname = "/home/litton/upload/namecard/test/"+name;
//			  Pix *image1 = pixRead(fullname.c_str());
//			  string s = OCRUtil::ocrFile(image1,"eng");
//			  prepareTest(s, 1);
//		  }
//	  }
//	}
//	if ((dir = opendir ("/home/litton/upload/papers/test")) != NULL) {
//
//	  /* print all the files and directories within directory */
//	  while ((ent = readdir (dir)) != NULL) {
//		  string name(ent->d_name);
//		  if(name!="."&&name!=".."){
//			  string fullname = "/home/litton/upload/papers/test/"+name;
//			  Pix *image1 = pixRead(fullname.c_str());
//			  string s = OCRUtil::ocrFile(image1,"eng");
//			  prepareTest(s, 0);
//		  }
//	  }
//	}
//
//
//
//	cout<<classifier.ComputeAccuracy(testMat,testRes)<<endl;
//}
