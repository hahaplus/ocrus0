/*
 * processor.h
 *
 *  Created on: Feb 3, 2015
 *      Author: xxy
 */

#ifndef IMAGE_PROCESS_SRC_WORKFLOW_PROCESSOR_H_
#define IMAGE_PROCESS_SRC_WORKFLOW_PROCESSOR_H_

#include "../api/wap_ocr_api.h"
#include "../preprocessing/utils/OCRUtil.h"
#include "../util/configUtil.h"
#include "../salientRecognition/execute.h"
#include "../preprocessing/utils/FileUtil.h"
#include "../borderPosition/border.h"
#include "../textDetect/textarea.h"
#include "../preprocessing/binarize/binarize.h"
#include "../preprocessing/deskew/deskew.h"
#include "../preprocessing/GaussianSPDenoise/denoise.h"
#include "../preprocessing/utils/TimeUtil.h"
#include "../preprocessing/cca/CCA.h"
#include "../preprocessing/shadow/shadow_remove.h"
#include "../dto/ocr_result_dto.h"
#include "recognition/recognition.h"
#include "preprocessing/denoise/denoise_line_point.h"
using namespace std;
using namespace cv;

/*
 * -s Single image mode.
 * -d Directory images mode.
 * -i Input file or input directory (depends on mode).
 * -o OCR output directory.
 * -c Configuration file path, (method = directory). see sn.conf as an example.
 * ex. ./image_process -s -i "test/workflow/input/ad1.jpg" -o "test/workflow/ocr" -c sn.conf
 */

class Processor {
public:
	const static string SALIENT;
	const static string BORDER;
	const static string TURN;
	const static string TEXT;
	const static string BINARIZE;
	const static string DENOISE;
	const static string DESKEW;
	const static string CCA;

	static string lang;
//	static int time01;
//	static int time02;
//	static int time1;
//	static int time2;
//	static int time3;
//	static int time4;
//	static int nopic;

	static void usage() {
		cout << "Please add parameters:" << endl;
		cout << " -s Single image mode." << endl;
		cout << " -d Directory images mode." << endl;
		cout << " -i Input file or input directory (depends on mode)." << endl;
		cout << " -o OCR output directory." << endl;
		cout
				<< " -c Configuration file path, (method = directory). see sn.conf as an example."
				<< endl;

	}

	static void process_main(int argc, char** argv) {
		int oc; /*选项字符 */
		char ec; /*无效的选项字符*/

		bool singleMode = true;
		string input;
		string ocrOutput;
		string configPath;
		string lang = "eng";

		cout << "Read parameters..." << endl;

		while ((oc = getopt(argc, argv, "sdi:o:c:l:")) != -1) {
			switch (oc) {
			case 's':
				printf("Single mode.\n");
				singleMode = true;
				break;
			case 'd':
				printf("Directory mode.\n");
				singleMode = false;
				break;
			case 'i':
				printf("Input path is %s\n", optarg);
				input = optarg;
				break;
			case 'o':
				printf("OCR output path is %s\n", optarg);
				ocrOutput = optarg;
				break;
			case 'l':
				printf("OCR Language is %s\n", optarg);
				lang = optarg;
				break;
			case 'c':
				printf("Config file path is %s\n", optarg);
				configPath = optarg;
				break;
			case '?':
				ec = (char) optopt;
				printf("Invalid option \' %c \'!\n", ec);
				break;
			case ':':
				printf("Lack option！\n");
				break;
			}
		}
		if (input.empty() && configPath.empty()) {
			usage();
			return;
		}

		Config config(configPath);

		if (singleMode) {
			vector<Mat> dsts;
			if (!input.empty())
				dsts = Processor::processFile(input, config);

			if (!ocrOutput.empty()) {
				string textPath = ocrOutput + "/"
						+ FileUtil::getFileNameNoSuffix(input) + ".txt";
//				time_t t1 = time(NULL);
				cout << "OCR to: " << textPath << endl;


        DenoiseLinePoint::removeNoise(dsts[0]);
        Mat tmp_img = dsts[0].clone();
        OcrDetailResult ocrResult;
				string text = WapOcrApi::recognitionToText(dsts[0], lang, 0, &ocrResult);
//				time_t t2 = time(NULL);
//				time4+=(t2-t1);

				/*string wholeText = WapOcrApi::recognitionToText(tmpImg, lang, 1);
				string wholeTextPath = ocrOutput + "/"
						+ FileUtil::getFileNameNoSuffix(input) + "_whole.txt";*/
				FileUtil::writeToFile(text, textPath);
				//FileUtil::writeToFile(wholeText, wholeTextPath);
				// output the image
				string imgPath = ocrOutput + "/"
										+ FileUtil::getFileNameNoSuffix(input) + ".jpg";
				cout << "OCR IMG to: " << imgPath;
				Mat out_img;

				ocrus::drawOcrResult(dsts[0], ocrResult, &out_img);
				imwrite( imgPath, out_img );
			}
		} else {
			if (!input.empty()) {

				vector<string> files = FileUtil::getAllFiles(input);
				for (int i = 0; i < files.size(); i++) {
					vector<Mat> mats = processFile(input + "/" + files[i],
							config);
					if (!ocrOutput.empty()) {

//						time_t t1 = time(NULL);
						string text = ocrMats(mats, lang);
//						time_t t2 = time(NULL);
//						time4+=(t2-t1);
						string textPath = ocrOutput + "/"
								+ FileUtil::getFileNameNoSuffix(files[i])
								+ ".txt";

						FileUtil::writeToFile(text, textPath);
					}
				}
			}
		}
		/*
		 cout<<"Aver. Salient&Border: "<<(0.0+time1)/nopic<<endl;
		 cout<<" -Aver. Salient: "<<(0.0+time01)/nopic<<endl;
		 cout<<" -Aver. Border: "<<(0.0+time02)/nopic<<endl;
		 cout<<"Aver. Text Detection: "<<(0.0+time2)/nopic<<endl;
		 cout<<"Aver. Pre-processing: "<<(0.0+time3)/nopic<<endl;

		 cout<<"Aver. Text OCR Procs: "<<(0.0+time4)/nopic<<endl;
		 */
	}

	//used in JNI
	static vector<Mat> process_image_main(Mat& img) {

		SalientRec src;
		Mat outputSRC, seg, crossBD, outputBD;

		cout << "salient and border..." << endl;
		long long int start = getSystemTime();
		src.salient(img, outputSRC, seg);
		Mat outputFileSRC = convertToVisibleMat<float>(outputSRC);

		int res = getBorderImgOnSalient(img, outputSRC, crossBD, outputBD);
		if (res == -1) {
			res = getBorderImgOnRaw(img, outputSRC, crossBD, outputBD);
		}

		normalize(outputBD, outputBD, 0, 255, NORM_MINMAX);
		outputBD.convertTo(outputBD, CV_8UC1);

		long long int end = getSystemTime();
		printf("salient and border time: %lld ms\n", end - start);

		cout << "text detection..." << endl;
		start = getSystemTime();
		vector<Mat> textPieces;
		textDetect(outputBD, textPieces, res == -1 ? false : true);
		end = getSystemTime();
		printf("text detection time: %lld ms\n", end - start);

		cout << "Preprocessing..." << endl;
		start = getSystemTime();
		for (unsigned int i = 0; i < textPieces.size(); i++) {
			cvtColor(textPieces[i], textPieces[i], COLOR_BGR2GRAY);
		}
		//vector<Mat> bins, denoises, deskews;
		Binarize::binarizeSet(textPieces, textPieces);
		Denoise::denoiseSet(textPieces, textPieces);
		Deskew::deskewSet(textPieces, textPieces);
		end = getSystemTime();
		printf("Preprocessing time: %lld ms\n", end - start);

		return textPieces;

	}

	//used in JNI
	static void process_image_main(Mat& img, Mat& dst) {

		vector<Mat> mats = process_image_main(img);
		dst = merge(mats);
	}

	static string ocrMat(Mat& mat) {
		ostringstream os;
		os << OCRUtil::ocrFile(mat, lang) << endl;
		return os.str();
	}

	static string ocrMats(vector<Mat>& mats, string lang) {
		ostringstream os;
		for (unsigned i = 0; i < mats.size(); i++) {
			os << OCRUtil::ocrFile(mats[i], lang) << endl;
		}
		return os.str();
	}

	static vector<Mat> processFile(string input, const Config conf) {
		Config config = conf;
		Mat img = imread(input);
		ShadowRemove::removeShadow(img);
		imwrite(string("tmp/shadow/")+FileUtil::getFileName(input)+".jpg", img);
		cout << "Process " << input << endl;
//

		vector<Mat> textPieces;
		textPieces.push_back(img);
//		textDetect(outputBD, textPieces, res == -1 ? false : true);
//
//		string textPath = textOut + "/" + FileUtil::getFileName(input);
//
//		imwrite(textPath, merge(textPieces));

		cout << "Preprocessing..." << endl;

		for (unsigned int i = 0; i < textPieces.size(); i++) {
			cvtColor(textPieces[i], textPieces[i], COLOR_BGR2GRAY);
		}

		for (int i = 0; i < config.size(); i++) {
			vector<Mat> cur(textPieces.size());
			pair<string, string> step = config.get(i);
			void (*process)(vector<Mat>&, vector<Mat>&) = getMethod(step.first);

			string outputPath = step.second + "/"
					+ FileUtil::getFileName(input);
			cout<<"outputpath:" + outputPath<<endl;
			process(textPieces, cur);
			imwrite(outputPath, merge(cur));
			textPieces = cur;
		}

		return textPieces;
	}
	static Mat merge(vector<Mat>& mats) {
		int width = maxWidth(mats);
		int height = totalHeight(mats);
		int index = 0;
		Mat dst(height, width, CV_8UC1);
		for (unsigned int i = 0; i < mats.size(); i++) {
			Mat roi = dst(Rect(0, index, mats[i].cols, mats[i].rows));
			mats[i].copyTo(roi);
			index += mats[i].rows;
		}
		return dst;
	}
	static int maxWidth(vector<Mat>& mats) {
		int width = 0;
		for (unsigned int i = 0; i < mats.size(); i++) {
			if (width < mats[i].cols)
				width = mats[i].cols;
		}
		return width;
	}
	static int totalHeight(vector<Mat>& mats) {
		int height = 0;
		for (unsigned int i = 0; i < mats.size(); i++) {
			height += mats[i].rows;
		}
		return height;
	}
	static void processDir(string input, const Config conf) {
		vector<string> files = FileUtil::getAllFiles(input);
		for (int i = 0; i < files.size(); i++) {
			vector<Mat> mats = processFile(input + "/" + files[i], conf);
		}
	}
	static void (*getMethod(string methodName))(vector<Mat>&, vector<Mat>&)
			{
				if (methodName == BINARIZE) {
					return Binarize::binarizeSet;
				} else if (methodName == DENOISE) {
					return Denoise::denoiseSet;
				} else if (methodName == DESKEW) {
					return Deskew::deskewSet;
				} else if (methodName == CCA) {
					return CCA::removeGarbageSet;
				} else
					return NULL;
			}

		};

		const string Processor::SALIENT = "salient";
		const string Processor::BORDER = "border";
		const string Processor::TURN = "turn";
		const string Processor::TEXT = "text";
		const string Processor::BINARIZE = "binarize";
		const string Processor::DENOISE = "denoise";
		const string Processor::DESKEW = "deskew";
		const string Processor::CCA = "cca";

		string Processor::lang = "eng";

#endif /* IMAGE_PROCESS_SRC_WORKFLOW_PROCESSOR_H_ */
