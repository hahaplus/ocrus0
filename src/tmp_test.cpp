
#include "segmentation/segmentator.h"
#include "dto/ocr_result_dto.h"
#include "recognition/recognition.h"
#include "binarization/wap_binarize.h"
#include "util/general.h"
#include "textDetect/simple_text_detect.h"
#include "preprocessing/enhancement/enhancement.h"
#include "preprocessing/denoise/remove_line.h"
#include <fstream>

int main(int argc, char *argv[]) {

  ifstream file_list(argv[3]);
  string file_path;
  int cnt = 0;
  while (!file_list.eof()) {
    file_list >> file_path;
    std::string path_img(file_path);
    cv::Mat img = cv::imread(path_img, CV_LOAD_IMAGE_COLOR);
    cv::Mat gray_img, binarize_img;
    ocrus::removeRedLineFor406(img);
    cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

    Enhancement::enhancementAndBinarize(gray_img, binarize_img, 0);
    OcrDetailResult odr;
   // if (cnt==2)
    Segmentator::segmentImg(binarize_img, &odr);
    //General::showImage(binarize_img);
    //cv::imwrite(file_path + "row.png", binarize_img);
    //cv::imwrite(file_path + "hist_pix_bell.png", binarize_img);
    //odr.clear();
    cv::cvtColor(binarize_img, binarize_img, COLOR_GRAY2BGR);
    cv::imwrite(file_path + "_binarize.png", binarize_img);
    Mat out_img;
    ocrus::drawOcrResult(binarize_img, odr, &out_img);
    imwrite(file_path + "_symbol.png", out_img);

    ofstream output_file(file_path+"_symbol.txt"); // write the file
    for (auto ru : odr.getResult())
    {
       output_file << "word: '"<<ru.content<<"';  \tconf: "<<ru.confidence<<"; bounding_box: "<<
           ru.bounding_box[0].x<<","<<ru.bounding_box[0].y<<","<< ru.bounding_box[1].x << ","
           << ru.bounding_box[1].y<< ";"<<endl;
    }

    cout << "complete " << file_path << endl;
  }
  return 0;
}
