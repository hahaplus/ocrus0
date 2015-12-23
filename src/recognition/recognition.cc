/*
 * Recognition functionalities
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include "recognition.h"

#include <stdio.h>
#include <stdlib.h>

#include <algorithm>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <tesseract/ltrresultiterator.h>

#include "dto/ocr_result_dto.h"

namespace ocrus {

void ocrPrintBoundingBox(const cv::Mat& src,
                         tesseract::PageSegMode page_seg_mode,
                         tesseract::PageIteratorLevel level,
                         const std::string& lang) {
  tesseract::TessBaseAPI api;
  api.Init(NULL, lang.c_str());

  //api.SetVariable("tessedit_dump_choices", "true");
  //api.SetVariable("classify_debug_level", "2");

  // Fraction of neighbourhood, default: 0.4
  api.SetVariable("textord_occupancy_threshold", "0.6");

  // Find vertical and horizontal line objects and removes them, default: true
  api.SetVariable("find_remove_lines", "false");

  api.SetPageSegMode(page_seg_mode);
  api.SetImage(reinterpret_cast<uchar*>(src.data), src.cols, src.rows, 1,
               src.cols);
  api.Recognize(NULL);

  tesseract::ResultIterator* ri = api.GetIterator();
  if (ri != 0) {
    do {
      const char* word = ri->GetUTF8Text(level);
      float conf = ri->Confidence(level);
      int x1, y1, x2, y2;
      ri->BoundingBox(level, &x1, &y1, &x2, &y2);       // here is the postion
      printf("word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n", word,
             conf, x1, y1, x2, y2);
      delete[] word;
    } while (ri->Next(level));
  }
  api.End();
}

void ocrPrintCandidates(const cv::Mat& gray_img, const std::string& lang) {
  tesseract::PageSegMode page_seg_mode = tesseract::PSM_SINGLE_COLUMN;
  tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;

  tesseract::TessBaseAPI api;
  api.Init(NULL, lang.c_str());

  api.SetVariable("tessedit_dump_choices", "true");
  api.SetVariable("classify_debug_level", "2");

  // Fraction of neighbourhood, default: 0.4
  api.SetVariable("textord_occupancy_threshold", "0.6");

  // Find vertical and horizontal line objects and removes them, default: true
  api.SetVariable("find_remove_lines", "false");

  api.SetPageSegMode(page_seg_mode);
  api.SetImage(reinterpret_cast<uchar*>(gray_img.data), gray_img.cols, gray_img.rows, 1,
               gray_img.cols);
  api.Recognize(NULL);

  tesseract::ResultIterator* ri = api.GetIterator();
  if (ri != 0) {
    do {
      const char* word = ri->GetUTF8Text(level);
      float conf = ri->Confidence(level);
      int x1, y1, x2, y2;
      ri->BoundingBox(level, &x1, &y1, &x2, &y2);
      printf("word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n", word,
             conf, x1, y1, x2, y2);
      delete[] word;

      printf("  ");
      tesseract::ChoiceIterator choiceIterator(*ri);
      do {
        const char* text = choiceIterator.GetUTF8Text();
        float confidence  = choiceIterator.Confidence();
        printf("(%s, %.2f); ", text, confidence);
      } while (choiceIterator.Next());
      printf("\n");

    } while (ri->Next(level));
  }
  api.End();
}

void drawOcrResult(const cv::Mat &in_img, const OcrDetailResult &result,
                   cv::Mat *out_img) {
  const char *path_text = "/tmp/temp.txt";
  const char *path_temp_img = "/tmp/temp_img.png";
  const char *path_out_img = "/tmp/out_img.png";

  FILE *f_text = fopen(path_text, "w");
  auto res = result.getResult();
  for (auto r : res) {
    int x1 = r.bounding_box[0].x, y1 = r.bounding_box[0].y;
    int x2 = r.bounding_box[1].x, y2 = r.bounding_box[1].y;
    std::string content = r.content;
    std::replace(content.begin(), content.end(), '\n', ' ');
    std::replace(content.begin(), content.end(), '\r', ' ');
    const char *word = content.c_str();
    float conf = r.confidence;

    fprintf(f_text, "word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n",
            word, conf, x1, y1, x2, y2);
  }
  fclose(f_text);

  cv::Mat temp_img = in_img.clone();
  if (temp_img.channels() == 1) {
    cv::cvtColor(temp_img, temp_img, cv::COLOR_GRAY2BGR);
  }
  cv::imwrite(path_temp_img, temp_img);

  char cmd[512];
  sprintf(cmd, "ocrus_draw_bbox.py %s %s %s", path_temp_img, path_text,
          path_out_img);
  system(cmd);

  *out_img = cv::imread(path_out_img);
}

}  // namespace ocrus
