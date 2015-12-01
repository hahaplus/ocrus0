/*
 * Recognition functionalities
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#include "ocrus/recognition.h"

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <tesseract/ltrresultiterator.h>

namespace ocrus {

void ocrPrintBoundingBox(cv::Mat& src, tesseract::PageIteratorLevel level,
                         const std::string& lang) {

  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  api->Init(NULL, lang.c_str());
  api->SetImage((uchar*) src.data, src.cols, src.rows, 1, src.cols);
  api->Recognize(NULL);

  tesseract::ResultIterator* ri = api->GetIterator();
  if (ri != 0) {
    do {
      const char* word = ri->GetUTF8Text(level);
      float conf = ri->Confidence(level);
      int x1, y1, x2, y2;
      ri->BoundingBox(level, &x1, &y1, &x2, &y2);       // here is the postion
      printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n", word,
             conf, x1, y1, x2, y2);
      delete[] word;
    } while (ri->Next(level));
  }
  api->End();
  delete api;
}

}  // namespace ocrus
