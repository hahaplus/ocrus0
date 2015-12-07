/*
 * WapOcrApi.cpp
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#include "wap_ocr_api.h"
#include "util/string_util.h"
#include "util/algorithm_util.h"
using namespace std;
using namespace tesseract;
using namespace cv;

WapOcrApi::WapOcrApi() {
  // TODO Auto-generated constructor stub
}
void WapOcrApi::release() {
  if (api != NULL) {
    api->End();
    delete api;
    api = NULL;
  }
}
int AlgorithmUtil::connect_threshold = 0;
Mat WapOcrApi::img;
tesseract::TessBaseAPI *WapOcrApi::api;
/*
 * overlap ratio (0.0 ~ 1.0)to judge as overlap
 * */
double WapOcrApi::epsY = 0.5;
double WapOcrApi::epsX = -0.08;
string WapOcrApi::recognitionToText(cv::Mat src, const string lang,
                                    int cut_level, OcrDetailResult* result) {
  if (result == NULL) {
    result = new OcrDetailResult;
  }
  if (api == NULL) {
    api = new tesseract::TessBaseAPI();
  }
  if (api->Init(NULL, lang.c_str())) {
    printf("Could not initialize tesseract.\n");
    exit(-1);
  }
  WapOcrApi::img = src.clone();
  Mat tmpImg = src.clone();
  api->SetVariable("save_blob_choices", "T");
  api->SetImage((uchar*) src.data, src.cols, src.rows, src.channels(),
                src.cols);
  api->Recognize(0);
  tesseract::ResultIterator* ri = api->GetIterator();
  tesseract::PageIteratorLevel level =
      (cut_level == 0) ? tesseract::RIL_SYMBOL : tesseract::RIL_TEXTLINE;
  string res;
  int line_cnt = 0;
  if (ri != 0) {
    do {
      const char* word = ri->GetUTF8Text(level);
      float conf = ri->Confidence(level);
      int x1, y1, x2, y2;
      ri->BoundingBox(level, &x1, &y1, &x2, &y2);
      if (ri->IsAtBeginningOf(tesseract::RIL_TEXTLINE)) {
        line_cnt++;
      }
      vector<cv::Point2i> corners;
      corners.push_back(cv::Point2i(x1, y1));
      corners.push_back(cv::Point2i(x2, y2));
      // the other choices of the character
      tesseract::ChoiceIterator choiceIterator(*ri);
      // save the result
      ResultUnit rt(corners, string(word));
      rt.confidence = conf;
      rt.line_index = line_cnt;
      if (cut_level == 0)
        do {
          const char* txt = choiceIterator.GetUTF8Text();
          if (txt != NULL && strlen(txt) != 0) {
            rt.candidates.push_back(
                make_pair(txt, choiceIterator.Confidence()));
          }
          //delete[] txt;
        } while (choiceIterator.Next());
      delete[] word;
      result->push_back_symbol(rt);
    } while (ri->Next(level));

    //api->End();
    //delete api;
    //api = new tesseract::TessBaseAPI();
    // init again
    /*if (api->Init(NULL, lang.c_str())) {
     printf("optimize process.Could not initialize tesseract.\n");
     exit(-1);
     }

     //api->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
     api->SetImage((uchar*) tmpImg.data, tmpImg.cols, tmpImg.rows, tmpImg.channels(), tmpImg.cols);
*/
    getBBox(src, result);
    optimize(result);
    res = result->toString(cut_level);
  }
  return res;
}
void WapOcrApi::getBBox(const cv::Mat &img, OcrDetailResult* odr) {
  odr->clear();
  vector<vector<pair<int, int> > > block_list = AlgorithmUtil::floodFillInMat<
      Vec<uchar, 1> >(img, 0, 0);
  vector<double> avg_height, avg_width;
  for (int i = 0; i < block_list.size(); i++) {
    vector<pair<int, int> > block = block_list[i];
    ResultUnit ru;
    for (auto b : block) {
      ru.bounding_box[0].x = min(ru.bounding_box[0].x, b.second);
      ru.bounding_box[1].x = max(ru.bounding_box[1].x, b.second);
      ru.bounding_box[0].y = min(ru.bounding_box[0].y, b.first);
      ru.bounding_box[1].y = max(ru.bounding_box[1].y, b.first);
    }
    avg_height.push_back(ru.bounding_box[1].y - ru.bounding_box[0].y);
    avg_width.push_back(ru.bounding_box[1].x - ru.bounding_box[0].x);
    odr->push_back_symbol(ru);
  }
  double avg_h = AlgorithmUtil::getAverageValue<double>(avg_height);
  double avg_w = AlgorithmUtil::getAverageValue<double>(avg_width);
  // filt too small box, they are noise
  vector<ResultUnit> filted_result;
  for (int i = 0; i < odr->getResultSize(); i++) {
      ResultUnit ru = odr->getResultAt(i);
      pair<double, double> box_size(ru.bounding_box[1].x - ru.bounding_box[0].x, ru.bounding_box[1].y - ru.bounding_box[0].y);
      if (box_size.first < 0.8*avg_w && box_size.second < 0.8*avg_h)
      {
          continue;
      }
      filted_result.push_back(ru);
  }
  odr->setResult(filted_result);
}
bool WapOcrApi::overlap(pair<int, int> a, pair<int, int> b, int eps) {
  if (a.first > b.first) {
    swap(a, b);
  }
  return b.first + eps <= a.second;
}
void WapOcrApi::mapToLine(vector<ResultUnit> &symbols) {
  sort(symbols.begin(), symbols.end(), CompY());
  vector<pair<int, int> > intervals;
  for (int i = 0; i < symbols.size(); i++) {
    ResultUnit &rt = symbols[i];
    pair<int, int> interval = make_pair(rt.bounding_box[0].y,
                                        rt.bounding_box[1].y);
    if (!intervals.empty()
        && overlap(interval, intervals.back(),
                   epsY * (interval.second - interval.first))) {
      // update
      intervals.back().second = max(intervals.back().second, interval.second);
    } else {
      intervals.push_back(interval);
    }
    rt.line_index = intervals.size() - 1;
  }
}
void WapOcrApi::optimize(OcrDetailResult* result) {
  if (result == NULL)
    return;
  vector<ResultUnit> symbols = result->getResult();
  mapToLine(symbols);
  vector<ResultUnit> line;
  result->clear();
  // degbug
  int iter_cnt = 3, tmp_cnt = 0;
  for (int i = 0; i < symbols.size(); i++) {
    line.push_back(symbols[i]);
    if (i == symbols.size() - 1
        || symbols[i].line_index != symbols[i + 1].line_index) {
      mergeAndSplit(line);
      result->push_back_symbol(line);
      line.clear();
      //if (++tmp_cnt == iter_cnt)
      //break;
    }
  }
}
void WapOcrApi::mergeAndSplit(vector<ResultUnit> &line) {
  // try to merge
  sort(line.begin(), line.end(), CompX());
  pair<int, int> interval_back(-1, -1);
  vector<ResultUnit> segment;
  vector<ResultUnit> new_line;
  for (int i = 0; i < line.size(); i++) {
    ResultUnit &rt = line[i];
    pair<int, int> interval = make_pair(rt.bounding_box[0].x,
                                        rt.bounding_box[1].x);
    if (interval_back.first == -1)
      interval_back = interval;
    else
      interval_back.second = max(interval_back.second, interval.second);
    pair<int, int> next_interval(1e6, 1e6);
    segment.push_back(line[i]);
    if (i != line.size() - 1) {
      next_interval = make_pair(line[i + 1].bounding_box[0].x,
                                line[i + 1].bounding_box[1].x);
    }
    if (!overlap(next_interval, interval_back,
                 epsX * (next_interval.second - next_interval.first))) {
      interval_back = make_pair(-1, -1);
      handle(segment);
      for (int j = 0; j < segment.size(); j++) {
        new_line.push_back(segment[j]);
      }
      segment.clear();
    }
  }
  line = new_line;
}
void WapOcrApi::handle(vector<ResultUnit> &segment) {
  if (segment.size() < 2)
    return;
  // try to merge
  vector<cv::Point2i> merge_bounding_box(2);
  merge_bounding_box[0].x = merge_bounding_box[0].y = 1e6;
  merge_bounding_box[1].x = merge_bounding_box[1].y = -1e6;
  double best_confi = 0;
  double mean_confi = 0;
  ResultUnit merge_unit;
  vector<ResultUnit> split_units;
  for (int i = 0; i < segment.size(); i++) {
    ResultUnit ru = segment[i];
    merge_bounding_box[0].x = min(ru.bounding_box[0].x,
                                  merge_bounding_box[0].x);
    merge_bounding_box[0].y = min(ru.bounding_box[0].y,
                                  merge_bounding_box[0].y);
    merge_bounding_box[1].x = max(ru.bounding_box[1].x,
                                  merge_bounding_box[1].x);
    merge_bounding_box[1].y = max(ru.bounding_box[1].y,
                                  merge_bounding_box[1].y);
    mean_confi += ru.confidence;
  }
  merge_unit.bounding_box = merge_bounding_box;
  // try to split
  // width / height
  double width = merge_bounding_box[1].x - merge_bounding_box[0].x;
  double height = merge_bounding_box[1].y - merge_bounding_box[0].y;
  double wh_ratio = width / (height * 0.8);
  if (wh_ratio > 1.5) {
    int part = (int) (wh_ratio + 0.5);  // round
    split_units.clear();
    for (int i = 0; i < part; i++) {
      ResultUnit ru;
      ru.bounding_box[0].x = i * 1.0 / part * width + merge_bounding_box[0].x;
      ru.bounding_box[0].y = merge_bounding_box[0].y;
      ru.bounding_box[1].x = (i + 1) * 1.0 / part * width
          + merge_bounding_box[0].x;
      ru.bounding_box[1].y = merge_bounding_box[1].y;
      recognizeUnit(ru);
      ru.line_index = segment[0].line_index;
      split_units.push_back(ru);
    }
    segment.clear();
    for (int i = 0; i < split_units.size(); i++) {
      segment.push_back(split_units[i]);
    }
  } else if (false && 1.0 / wh_ratio > 1.5) {
    int part = (int) (wh_ratio + 0.5);  // round
    split_units.clear();
    for (int i = 0; i < part; i++) {
      ResultUnit ru;
      ru.bounding_box[0].y = i * 1.0 / part * height + merge_bounding_box[0].y;
      ru.bounding_box[0].x = merge_bounding_box[0].x;
      ru.bounding_box[1].y = (i + 1) * 1.0 / part * height
          + merge_bounding_box[0].y;
      ru.bounding_box[1].x = merge_bounding_box[1].x;
      recognizeUnit(ru);
      ru.line_index = segment[0].line_index;
      split_units.push_back(ru);
    }
    segment.clear();
    for (int i = 0; i < split_units.size(); i++) {
      segment.push_back(split_units[i]);
    }
  } else {
    recognizeUnit(merge_unit);
    segment.clear();
    segment.push_back(merge_unit);
  }
}
void WapOcrApi::recognizeUnit(ResultUnit &ru) {



  api->TesseractRect(img.data,1,img.step1(),ru.bounding_box[0].x, ru.bounding_box[0].y,
                     ru.bounding_box[1].x - ru.bounding_box[0].x,
                     ru.bounding_box[1].y - ru.bounding_box[0].y);
  /*api->SetImage((uchar*) img.clone().data, img.cols, img.rows, img.channels(),
                 img.cols);
  api->SetRectangle(ru.bounding_box[0].x, ru.bounding_box[0].y,
                        ru.bounding_box[1].x - ru.bounding_box[0].x,
                        ru.bounding_box[1].y - ru.bounding_box[0].y);
  api->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
  */
  ru.confidence = api->MeanTextConf();
  ru.content = api->GetUTF8Text()/*"a"*/;
}
WapOcrApi::~WapOcrApi() {
// TODO Auto-generated destructor stub

}

