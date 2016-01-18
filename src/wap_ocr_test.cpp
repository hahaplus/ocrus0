/*
 * wap_ocr_test.cpp
 *
 *  Created on: Dec 11, 2015
 *      Author: michael
 */
#include "api/wap_ocr_api.h"
#include "preprocessing/shadow/shadow_remove.h"
#include "preprocessing/denoise/denoise_line_point.h"
#include "dto/ocr_result_dto.h"
#include "preprocessing/binarize/binarize.h"
#include "recognition/recognition.h"
#include "binarization/wap_binarize.h"
#include "util/general.h"
#include "textDetect/simple_text_detect.h"
#include "opencv2/highgui/highgui.hpp"
#include "preprocessing/enhancement/enhancement.h"
#include "preprocessing/denoise/remove_line.h"
int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Print the bounding boxes of an image\n"
           "Gray and binarized images are saved where the image is located\n"
           "Usage: ocrus_bounding_box page_seg_mode word|symbol path_img\n"
           "page_seg_mode: An enum integer from Tesseract");
    return 0;
  }

  tesseract::PageSegMode page_seg_mode =
      static_cast<tesseract::PageSegMode>(atoi(argv[1]));
  std::string level(argv[2]);
  std::string path_img(argv[3]);

  tesseract::PageIteratorLevel level_ = tesseract::RIL_SYMBOL;
  if (level == "word") {
    level_ = tesseract::RIL_WORD;
  } else if (level == "symbol") {
    level_ = tesseract::RIL_SYMBOL;
  } else {
    std::cout << "level can only be word or symbol" << std::endl;
    return -1;
  }

  cv::Mat img, gray_img, binarize_img, box_img, enhance_img;

  img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);
  //cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
  //ShadowRemove::removeShadow(img);
  ocrus::removeRedLineFor406(img);
  cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
  //IplImage gray_input(gray_img);
  //IplImage *gray_output = cvCreateImage(cvGetSize(&gray_input), 8, 1);
  //ImageStretchByHistogram(&gray_input, gray_output);
  //gray_img = cvarrToMat(gray_output);

  //Rect text_area = SimpleTextDetect::simpleDetect(gray_img);
  //Binarize::binarize(gray_img, binarize_img);
  //ocrus::binarize(gray_img, binarize_img);

 // IplImage input_image(binarize_img);
  //IplImage *img_dilate = cvCreateImage(cvGetSize(&input_image), 8, 1);
  //cvDilate(&input_image, img_dilate, NULL,1); //膨胀
  //cvErode( img_dilate, &input_image, NULL,1);

  //binarize_img
  Enhancement::enhancementAndBinarize(gray_img, binarize_img, 0);
 // Enhancement::enhancementAndBinarize(gray_img, enhance_img, 0.6);
  //DenoiseLinePoint::removeNoise(binarize_img);

  //General::showImage(enhance_img);
  OcrDetailResult ocr_detail_result, enhance_result;
  WapOcrApi::recognitionToTextByCNN(binarize_img, "jpn+jpnRSN", 0, &ocr_detail_result);
  //WapOcrApi::recognitionToTextByCNN(enhance_img, "jpn+jpnRSN", 0, &enhance_result);
  //WapOcrApi::mergeOcrResult(binarize_img, enhance_img, &ocr_detail_result, &enhance_result );
  for (auto ru : ocr_detail_result.getResult())
  {
    printf("word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n", ru.content == "" ? "?" : ru.content.c_str(),
                 ru.confidence, ru.bounding_box[0].x, ru.bounding_box[0].y, ru.bounding_box[1].x, ru.bounding_box[1].y);
  }

  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(path_img + "_binarize.png", binarize_img);
  Mat out_img;
  ocrus::drawOcrResult(binarize_img, ocr_detail_result, &out_img);
  imwrite(path_img + "_symbol.png", out_img);
  // ocrus::ocrPrintBoundingBox(binarize_img, page_seg_mode, level_, "jpn+jpnRSN");

  // cv::imwrite(path_img + "_gray.png", gray_img);

  //cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  //cv::imwrite(path_img + "_binarize.png", binarize_img);

  return 0;
}


