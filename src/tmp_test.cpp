#include "segmentation/segmentator.h"
#include "dto/ocr_result_dto.h"
#include "recognition/recognition.h"
#include "binarization/wap_binarize.h"
#include "util/general.h"
#include "textDetect/simple_text_detect.h"
#include "preprocessing/enhancement/enhancement.h"
#include "preprocessing/denoise/remove_line.h"
#include <fstream>

bool only_one_case = false;
void handleOneCase(string file_path) {
  std::string path_img(file_path);
  cv::Mat img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);
  cv::Mat gray_img, binarize_img, enhance_img;
  ocrus::removeRedLineFor406(img);
  cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

  Enhancement::enhancementAndBinarize(gray_img, binarize_img, 0);
  Enhancement::enhancementAndBinarize(gray_img, enhance_img, 0.5);
  OcrDetailResult ocr_detail_result, enhance_result;
  // if (cnt==2)
  Segmentator::segmentImg(binarize_img, &ocr_detail_result);
  Segmentator::segmentImg(enhance_img, &enhance_result);

  OcrDetailResult::mergeOcrResult(binarize_img, enhance_img, &ocr_detail_result, &enhance_result );
  //General::showImage(binarize_img);
  //cv::imwrite(file_path + "row.png", binarize_img);
  //cv::imwrite(file_path + "hist_pix_bell.png", binarize_img);
  //odr.clear();
  cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
  cv::imwrite(file_path + "_binarize.png", binarize_img);
  Mat out_img;
  if (only_one_case)
  {
      ocrus::drawOcrResult(binarize_img, ocr_detail_result, &out_img);
      imwrite(file_path + "_symbol.png", out_img);
  }
  //imwrite(file_path + "_0.6_symbol.png", out_img);
  ofstream output_file(file_path + "_symbol.txt");  // write the file
  for (auto ru : ocr_detail_result.getResult()) {
    output_file << "word: '" << ru.content << "';  \tconf: " << ru.confidence
                << "; bounding_box: " << ru.bounding_box[0].x << ","
                << ru.bounding_box[0].y << "," << ru.bounding_box[1].x << ","
                << ru.bounding_box[1].y << ";" << endl;
  }

  cout << "complete " << file_path << endl;
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("Print the bounding boxes of an image\n"
           "Gray and binarized images are saved where the image is located\n"
           "Usage: ocrus_bounding_box page_seg_mode word|symbol path_img\n"
           "page_seg_mode: An enum integer from Tesseract");
    return 0;
  }

  string file_path;
  string choice = argv[1];
  int cnt = 0;
  if (choice == "s") {
     file_path = argv[2];
     only_one_case = true;
     handleOneCase(file_path);
  } else if (choice == "m") {
    ifstream file_list(argv[2]);
    while (!file_list.eof()) {
      file_list >> file_path;
      handleOneCase(file_path);
    }
  }
  return 0;
}
