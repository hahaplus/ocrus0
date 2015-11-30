/*
 * This
 * ocr_file.cpp
 *
 *  Created on: Nov 30, 2015
 *      Author: Chang Sun
 */

#include <opencv2/opencv.hpp>
#include "workflow/processor.h"

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cout << "OCR image file by language setting: eng+jpn+chi_sim"
				<< std::endl << "Usage: ocr_file fname_image" << std::endl;
		return 0;
	}

	cv::Mat mat = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

	std::cout << OCRUtil::ocrFile(mat) << std::endl;

	return 0;
}
