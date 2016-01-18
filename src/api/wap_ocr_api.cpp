/*
 * WapOcrApi.cpp
 *
 *  Created on: Nov 30, 2015
 *      Author: michael
 */

#include "wap_ocr_api.h"
#include "util/string_util.h"
#include "util/algorithm_util.h"
#include <set>
#include <fstream>
#include "recognition/recognition.h"
using namespace std;
using namespace tesseract;
using namespace cv;
int AlgorithmUtil::connect_threshold = 0;
//Mat WapOcrApi::img;
tesseract::TessBaseAPI *WapOcrApi::api;
//dict of all the character
vector<string> WapOcrApi::dict;
string WapOcrApi::dict_path = "/home/michael/workspace/ocrus0_build/networkModel/chars_id.txt";

//for the deeplearning
PyObject* WapOcrApi::pMod = NULL;
PyObject* WapOcrApi::pFunc = NULL;
PyObject* WapOcrApi::pDict = NULL;
/*
 * overlap ratio (0.0 ~ 1.0)to judge as overlap
 * */
double WapOcrApi::epsY = 0.5;
double WapOcrApi::epsX = -0.3;
WapOcrApi::WapOcrApi() {
  // TODO Auto-generated constructor stub
}
void WapOcrApi::init()
{
    loadDict();
    // Load the network module
    Py_Initialize();
    pMod = PyImport_ImportModule("ocrus.neural_network.network_api");
    pDict = PyModule_GetDict(pMod);
    pFunc = PyDict_GetItemString(pDict, "recognition_img");
}
void WapOcrApi::loadDict()
{
    ifstream dict_file(dict_path.c_str());
    while (!dict_file.eof())
    {
        int id;
        string val;
        dict_file >>id >> val;
        dict.push_back(val);
        //cout << val << endl;
    }
    dict_file.close();
}
void WapOcrApi::release() {
  if (api != NULL) {
    api->End();
    delete api;
    api = NULL;
  }
  Py_Finalize();
}

string WapOcrApi::recognitionToText(const cv::Mat &src, const string lang,
                                    int cut_level, OcrDetailResult* result) {
  if (result == NULL) {
    result = new OcrDetailResult;
  }
  recognitionWithTesseract(src, lang, cut_level, result);
  return result->toString(cut_level);
  // first pass
  OcrDetailResult &first_pass = *result;

  // format the Image and do the second pass recognition
  OcrDetailResult format_result;
  getBBox(src, &format_result);
  optimize(&format_result);
  Mat out_img;
  ocrus::drawOcrResult(src, format_result, &out_img);
  //imwrite("/home/michael/tmp.jpg", out_img);
  Mat formatImg;
  map<pair<int, int>, ResultUnit> pos_map;
  formatImage(src, formatImg, &format_result, pos_map);

  OcrDetailResult second_pass;
  recognitionWithTesseract(formatImg, lang, cut_level, &second_pass);

  // use second_pass result to fix incorrect result of first_pass
  // first build a origin map  (pixel coordinate ----> resultUnit index)
  vector<ResultUnit> result_list = first_pass.getResult();
  vector<vector<int>> origin_map(src.cols, vector<int>(src.rows));
  for (int i = 0; i < result_list.size(); i++) {
    ResultUnit &ru = result_list[i];
    for (int j = 0; j < ru.getWidth(); j++)
      for (int k = 0; k < ru.getHeight(); k++) {
        origin_map[j + ru.bounding_box[0].x][k + ru.bounding_box[0].y] = i;
      }
  }
  // using second pass to fix first pass
  vector<bool> is_del(result_list.size(), false);
  vector<ResultUnit> new_result_list;
  map<int, vector<ResultUnit>> replace_map;
  for (auto ru : second_pass.getResult()) {
    if (!pos_map.count(make_pair(ru.bounding_box[0].x, ru.bounding_box[0].y)))
      continue;
    ResultUnit first_ru = pos_map[make_pair(ru.bounding_box[0].x,
                                            ru.bounding_box[0].y)];

    // find the result unit index in first pass covered by first_ru
    set<int> index_set;
    for (int j = 0; j < first_ru.getWidth(); j++)
      for (int k = 0; k < first_ru.getHeight(); k++) {
        index_set.insert(
            origin_map[j + first_ru.bounding_box[0].x][k
                + first_ru.bounding_box[0].y]);
      }
    if (index_set.size() == 1) {
      ResultUnit ori_ru = result_list[*index_set.begin()];
      //if (ru.confidence > first_ru.confidence )
      if (ru.getHeight() * ru.getWidth()
          < ori_ru.getHeight() * ori_ru.getWidth()
          && ru.getHeight() * ru.getWidth()
              > 0.5 * ori_ru.getHeight() * ori_ru.getWidth()) {
        is_del[*index_set.begin()] = true;

        //new_result_list.push_back(ResultUnit(first_ru.bounding_box, ru.content, ru.candidates, ru.confidence, ru.line_index));
        replace_map[*index_set.begin()].push_back(
            ResultUnit(first_ru.bounding_box, ru.content, ru.candidates,
                       ru.confidence, ru.line_index));
      }
    }
  }
  for (int i = 0; i < result_list.size(); i++) {
    if (!is_del[i]) {
      new_result_list.push_back(result_list[i]);
    } else {
      for (auto ru : replace_map[i]) {
        new_result_list.push_back(ru);
      }
    }
  }

  result->setResult(new_result_list);
  //optimize(result);
  //namedWindow("xx",CV_WINDOW_NORMAL);
  //imshow("xx", src);
  //waitKey();

  return result->toString(cut_level);
}

void WapOcrApi::recognitionWithTesseract(const cv::Mat &src,
                                         const std::string lang,
                                         const int cut_level,
                                         OcrDetailResult* result) {
  if (result == NULL) {
    result = new OcrDetailResult;
  }
  if (api == NULL) {
    api = new tesseract::TessBaseAPI();
  }
  char *config[] = { (char*) ("wap") };

  if (api->Init(NULL, lang.c_str(), OEM_DEFAULT, config, 1, NULL, NULL,
                false)) {
    printf("Could not initialize tesseract.\n");
    exit(-1);
  }
  api->SetVariable("find_remove_lines", "false");
  api->SetVariable("textord_occupancy_threshold", "0.6");
  //WapOcrApi::img = src.clone();
  api->SetVariable("save_blob_choices", "T");
  api->SetPageSegMode(PSM_SINGLE_COLUMN);
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
  }
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
    avg_height.push_back(ru.bounding_box[1].y - ru.bounding_box[0].y + 1);
    avg_width.push_back(ru.bounding_box[1].x - ru.bounding_box[0].x + 1);
    odr->push_back_symbol(ru);
  }
  double avg_h = AlgorithmUtil::getAverageValue<double>(avg_height);
  double avg_w = AlgorithmUtil::getAverageValue<double>(avg_width);
  // filt too small box, they are noise
  vector<ResultUnit> filted_result;
  for (int i = 0; i < odr->getResultSize(); i++) {
    ResultUnit ru = odr->getResultAt(i);
    pair<double, double> box_size(
        ru.bounding_box[1].x - ru.bounding_box[0].x + 1,
        ru.bounding_box[1].y - ru.bounding_box[0].y + 1);
    if (box_size.first < avg_w || box_size.second < avg_h) {
      //continue;
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
                 epsX * ((next_interval.second - next_interval.first)))) {
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
  //return;
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
  merge_unit.line_index = segment[0].line_index;
  // try to split
  // width / height
  double width = merge_bounding_box[1].x - merge_bounding_box[0].x + 1;
  double height = merge_bounding_box[1].y - merge_bounding_box[0].y + 1;
  double wh_ratio = width / (height * 0.8);
  if (false && wh_ratio > 1.5 && segment.size() > 1) {
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
  } else if (false && 1.0 / wh_ratio > 1.5 && segment.size() > 1) {
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
int tmp_cnt = 0;
void WapOcrApi::recognizeUnit(ResultUnit &ru) {
  return;
  /*
   api->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);

   Mat character_unit;
   cutImage(img, character_unit, ru.bounding_box[0].x, ru.bounding_box[0].y,
   ru.bounding_box[1].x - ru.bounding_box[0].x,
   ru.bounding_box[1].y - ru.bounding_box[0].y);
   double scale_size = 20;
   scaleImage(character_unit, character_unit.cols * scale_size,
   character_unit.rows * scale_size);
   //Mat out;

   //character_unit = out;
   imwrite(
   "/home/michael/tmp_output/" + StringUtil::toString(tmp_cnt++) + ".jpg",
   character_unit);

   api->SetImage((uchar*) character_unit.data, character_unit.cols,
   character_unit.rows, img.channels(), character_unit.cols);
   ru.confidence = api->MeanTextConf();
   ru.content = api->GetUTF8Text()/*"a";
   character_unit.deallocate();
   */
}
void WapOcrApi::cutImage(const Mat &src, Mat &dst, int x, int y, int width,
                         int height) {
  dst = Mat(src, cv::Rect2i(x, y, width, height));

  //printf( "%d %d\n", width, height);
  //namedWindow("xx",CV_WINDOW_NORMAL);
  //imshow("xx", dst);
  //waitKey();
}
void WapOcrApi::scaleImage(Mat &src, double new_width, double new_height) {
  IplImage srcImg(src);
  IplImage* dstImg = cvCreateImage(CvSize(new_width, new_height), srcImg.depth,
                                   src.channels());
  cvResize(&srcImg, dstImg, CV_INTER_CUBIC);
  src = cvarrToMat(dstImg);
}
void WapOcrApi::writeCharacter(const Mat &src, Mat &dst, ResultUnit unit, int x,
                               int y) {
  for (int i = 0; i < unit.getHeight(); i++)
    for (int j = 0; j < unit.getWidth(); j++) {
      if (unit.bounding_box[0].y + i < 0
          || unit.bounding_box[0].y + i >= dst.rows
          || unit.bounding_box[0].x + j < 0
          || unit.bounding_box[0].x + j >= dst.cols)
        continue;
      if (i + y < 0 || i + y >= dst.rows || j + x < 0 || j + x >= dst.cols)
        continue;
      dst.at<uchar>(i + y, j + x) = src.at<uchar>(unit.bounding_box[0].y + i,
                                                  unit.bounding_box[0].x + j);
    }
}
// format the image to be more neat
void WapOcrApi::formatImage(const Mat &src, Mat &dst, OcrDetailResult *result,
                            map<pair<int, int>, ResultUnit> &pos_map) {
  dst = Mat(src.rows, src.cols, src.type());
  // make the white background
  memset(dst.data, 255, dst.rows * dst.cols * dst.channels());
  int space = 70;
  vector<ResultUnit> result_list = result->getResult();
  int last_index = 0;  // line index
  int y_pos = 0;     // each line 's y coordinate
  int x_pos = 0;     // each character's start at x coordinate
  int max_height = 0;
  for (auto ru : result_list) {
    if (ru.line_index != last_index) {
      y_pos += max_height + space;
      x_pos = 10;
      last_index = ru.line_index;
      max_height = 0;
    }
    pos_map[make_pair(x_pos, y_pos)] = ru;
    ResultUnit dbg = pos_map[make_pair(x_pos, y_pos)];
    writeCharacter(src, dst, ru, x_pos, y_pos);
    x_pos += space + ru.getWidth();
    max_height = max(ru.getHeight(), max_height);
  }
}

void WapOcrApi::mergeOcrResult(cv::Mat &main_img, cv::Mat assit_img,
                               OcrDetailResult* main_result,
                               OcrDetailResult* assit_result) {

  // write the main_result into man_map
  vector<vector<int> > main_map(main_img.rows, vector<int>(main_img.cols, -1));
  for (int i = 0; i < main_result->getResultSize(); i++) {
    ResultUnit ru = main_result->getResultAt(i);
    for (int j = 0; j < ru.getHeight(); j++)
      for (int k = 0; k < ru.getWidth(); k++) {
        main_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x] = i;
      }
  }

  // check the assist_result
  for (auto ru : assit_result->getResult()) {
    double cnt = 0;
    set<int> index_set;
    for (int j = 0; j < ru.getHeight(); j++)
      for (int k = 0; k < ru.getWidth(); k++) {
        // assert(j + ru.bounding_box[0].y < main_map.size() );
        if (j + ru.bounding_box[0].y >= main_img.rows
            || k + ru.bounding_box[0].x >= main_img.cols) {
          continue;
        }
        if (main_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x]
            != -1) {
          cnt++;
          index_set.insert(
              main_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x]);
        }
      }
    //assert((ru.getHeight() * ru.getWidth()) != 0);
    if (index_set.size() == 1 && cnt / (ru.getHeight() * ru.getWidth()) > 0.8) {
      //assert(*index_set.begin() >=0 && *index_set.begin() < main_result->getResultSize() );
      ResultUnit &ori_ru = main_result->getResultAt(*index_set.begin());

      if (cnt / (ori_ru.getHeight() * ori_ru.getWidth()) > 0.8
          && ori_ru.confidence <= ru.confidence) {
        //assert((ori_ru.getHeight() * ori_ru.getWidth()) != 0);
        // replace the result
        ori_ru = ru;
        for (int j = 0; j < ru.getHeight(); j++)
          for (int k = 0; k < ru.getWidth(); k++) {
            // assert(j + ru.bounding_box[0].y < main_img.rows && k + ru.bounding_box[0].x < main_img.cols);
            // assert(j + ru.bounding_box[0].y >= 0 && k + ru.bounding_box[0].x >= 0);
            // assert(j + ru.bounding_box[0].y < assit_img.rows && k + ru.bounding_box[0].x < assit_img.cols);
            if (j + ru.bounding_box[0].y >= main_img.rows
                || k + ru.bounding_box[0].x >= main_img.cols) {
              continue;
            }
            main_img.at<uchar>(j + ru.bounding_box[0].y,
                               k + ru.bounding_box[0].x) = assit_img.at<uchar>(
                j + ru.bounding_box[0].y, k + ru.bounding_box[0].x);
          }
      }
    }
  }
}

// recoginition the image with CNN

void WapOcrApi::recognitionWithCNN(const cv::Mat &img, ResultUnit &result) {
   // format the img
   // make the image to 28 * 28 without distortion
   int square_size = max(img.rows, img.cols);
   Mat square(square_size, square_size, CV_8UC1, 255);
   int offset_x = (square_size - img.cols) / 2;
   int offset_y = (square_size - img.rows) / 2;
   img.copyTo(square(cv::Rect(offset_x, offset_y, img.cols, img.rows)));
   cv::resize(square, square, Size(28, 28));

   // convert the image to  1d vector and rescale the value in range between (0, 1)
   // 0 is white, 1 is black
   vector<float> input_feature;
   for (int r = 0; r < square.rows; r++)
     for (int c = 0; c < square.cols; c++)
     {
         input_feature.push_back((255.0 - square.at<uchar>(r,c)) / 255.0);
     }
   // send the feature vector to the neutral network
    if (!pFunc)
      exit(-2);
    if (PyCallable_Check(pFunc)) {
      PyObject* pParm_tuple = PyTuple_New(input_feature.size());
      for (int i = 0; i < input_feature.size(); i++) {
        PyObject* pValue = Py_BuildValue("f", input_feature[i]);
        if (!pValue) {
          PyErr_Print();
          return;
        }
        PyTuple_SetItem(pParm_tuple, i, pValue);
      }
      PyObject*ret_list = PyObject_CallObject(pFunc, pParm_tuple);
      //printf("here3");
      int target_id = 0;
      for (int i = 0; i < PyTuple_GET_SIZE(ret_list); i++)
      {
          PyObject* value = PyTuple_GetItem(ret_list, i);
          float item_value = 0;
          if (i == 0)
          {
            float characer_id;
            PyArg_Parse(value, "f", &characer_id);
            result.content = dict[int(characer_id+1e-6)];
          }
          else if (i - 1 == target_id)
          {
            PyArg_Parse(value, "f", &item_value);
            result.confidence = item_value;
          }
          //result.candidates.push_back(pair(valueint))
      }
      printf("%s %f\n", result.content.c_str(), result.confidence);
    }

}
//
std::string WapOcrApi::recognitionToTextByCNN(const cv::Mat &img, const std::string lang,
                                   const int cutLevel,
                                   OcrDetailResult* bboxes_result)
{
    if (pMod == NULL)
    {
       init();
    }
    //getBBox(img, bboxes_result);
    //optimize(bboxes_result);
    recognitionWithTesseract(img, lang, cutLevel, bboxes_result);
    for (int i = 0; i < (*bboxes_result).getResultSize(); i++)
    {
        ResultUnit &ru = (*bboxes_result).getResultAt(i);
        // cut the single character from the img
        cv::Range row_range(ru.bounding_box[0].y, ru.bounding_box[1].y);
        cv::Range col_range(ru.bounding_box[0].x, ru.bounding_box[1].x);
        cv::Mat character_img = Mat(img, row_range, col_range);
        recognitionWithCNN(character_img, ru);
        //break;
    }
    return bboxes_result->toString();
}
