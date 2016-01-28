/*
 * Segmentator.cpp
 *
 *  Created on: Jan 20, 2016
 *      Author: michael
 */

#include <segmentation/segmentator.h>
#include "util/algorithm_util.h"
#include <set>
#include "util/general.h"
#include "recognition/recognition_by_CNN.h"
#include "image_similarity/ImageSimilarity.h"
#include <fstream>
vector<ResultUnit> Segmentator::result;
vector<vector<vector<int> > > Segmentator::result_map;   //
vector<vector<vector<float> > > Segmentator::confidence;
Segmentator::Segmentator() {
  // TODO Auto-generated constructor stub

}
Segmentator::~Segmentator() {
  // TODO Auto-generated destructor stub
}
void Segmentator::filterBigBox(cv::Mat &img, OcrDetailResult* seg_result) {
  vector<vector<vector<int> > > unit_map(img.rows,
                                         vector<vector<int> >(img.cols));
  for (int i = 0; i < seg_result->getResultSize(); i++) {
    ResultUnit ru = seg_result->getResultAt(i);
    for (int j = 0; j < ru.getHeight(); j++)
      for (int k = 0; k < ru.getWidth(); k++) {
        unit_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x].push_back(
            i);
      }
  }
  vector<ResultUnit> new_result;
  for (int i = 0; i < seg_result->getResultSize(); i++) {
    ResultUnit ru = seg_result->getResultAt(i);
    set<int> unit_set;  // the character unit in the box
    for (int j = 0; j < ru.getHeight(); j++)
      for (int k = 0; k < ru.getWidth(); k++) {
        if (unit_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x].size()
            > 1) {
          for (auto a : unit_map[j + ru.bounding_box[0].y][k
              + ru.bounding_box[0].x])
            unit_set.insert(a);
        }
      }
    if (unit_set.size() < 10) {
      new_result.push_back(ru);
    }
  }
  seg_result->setResult(new_result);
}
void Segmentator::mergeSmallBox(cv::Mat &img, OcrDetailResult* seg_result) {
  // TODO
  int iterate = 2;
  while (iterate--){
  vector<bool> is_delete(seg_result->getResultSize(), false);
  for (int i = 0; i < seg_result->getResultSize(); i++)
    for (int j = 0; j < seg_result->getResultSize(); j++) {
      if (i == j)
        continue;

      ResultUnit unit_i = seg_result->getResultAt(i);
      ResultUnit unit_j = seg_result->getResultAt(j);
      if (unit_i.getHeight() * unit_i.getWidth()
          < unit_j.getHeight() * unit_j.getWidth())
        continue;
      double overlap_area = ResultUnit::getOverlapArea(unit_i, unit_j);
      if (overlap_area / (unit_j.getWidth() * unit_j.getHeight()) >= 0.4) {
        is_delete[j] = true;
        ResultUnit merge_one = ResultUnit::getMergeUnit(unit_i, unit_j);
        merge_one.confidence = unit_i.confidence;
        merge_one.content = unit_i.content;
        seg_result->setResultAt(i, merge_one);
      }
    }
  vector<ResultUnit> new_result;
  for (int i = 0; i < seg_result->getResultSize(); i++) {
    if (!is_delete[i]) {
      new_result.push_back(seg_result->getResultAt(i));
    }
  }

  seg_result->setResult(new_result);
  }
}
void Segmentator::segmentImg(cv::Mat &image, OcrDetailResult* seg_result) {
  vector<vector<pair<int, int> > > block_list = AlgorithmUtil::floodFillInMat<
      Vec<uchar, 1> >(image, 0, 0);

  for (auto &block : block_list) {
    Rect2i rect = AlgorithmUtil::getBoundingBox(block);
    ResultUnit ru;
    ru.bounding_box[0] = cv::Point2i(rect.x, rect.y);
    ru.bounding_box[1] = cv::Point2i(rect.x + rect.width - 1,
                                     rect.y + rect.height - 1);
    ru.property = block.size();
    seg_result->push_back_symbol(ru);
  }
  filterBigBox(image, seg_result);  // remove the big box which contain many characters
  mergeSmallBox(image, seg_result);
  getRow(image, seg_result);
  // draw the row
  /*int start_index = 0;
   for (int i = 1; i < seg_result->getResultSize(); i++) {
   if (seg_result->getResultAt(i).line_index
   == seg_result->getResultAt(start_index).line_index) {
   ResultUnit last_unit = seg_result->getResultAt(i-1);
   ResultUnit cur_unit = seg_result->getResultAt(i);
   line(image, (last_unit.bounding_box[0]+last_unit.bounding_box[1])/2,
   (cur_unit.bounding_box[0]+cur_unit.bounding_box[1])/2, 0);
   }
   else
   start_index = i;
   }*/

  map<int, vector<ResultUnit>> row_map;
  for (auto &unit : seg_result->getResult()) {
    row_map[unit.line_index].push_back(unit);
  }
  handleEachRowInit(image, row_map);
  //seg_result->setResult(result);
  //return;
  for (auto &p : row_map) {
    handleEachRow(image, p.first, p.second, confidence[p.first]);  // handle each row
  }
  vector<ResultUnit> new_result;
  for (auto &p : row_map) {
    for (auto &ru : p.second) {
      new_result.push_back(ru);
    }
  }

  // cout << "result.size() "<<new_result.size() << endl;
  // seg_result->clear();
  seg_result->setResult(new_result);
  mergeSmallBox(image, seg_result);
}
void Segmentator::getRow(cv::Mat &image, OcrDetailResult* seg_result) {
  //vector<int> y_hist(image.rows, 0);
  vector<double> y_hist(image.rows, 0);
  vector<vector<int> > row_members(image.rows);  // each resultUnit index in one row
  for (int i = 0; i < seg_result->getResultSize(); i++) {
    ResultUnit ru = seg_result->getResultAt(i);

    for (int x = 0; x < ru.getHeight(); x++) {
      double a = (ru.getHeight() - 1) / 2.0;
      double w = -(1 / (a * a)) * (x - a) * (x - a) + 1;  // a bell line function in range (0, 1)
      y_hist[ru.bounding_box[0].y + x] += w * ru.property;
      row_members[ru.bounding_box[0].y + x].push_back(i);
    }
  }
  bool need_draw = false;
  // draw the result for the test
  if (need_draw) {
    double maximum = 0;
    for (int i = 0; i < y_hist.size(); i++) {
      maximum = max(y_hist[i], maximum);
    }
    for (int i = 0; i < y_hist.size(); i++) {
      int end = y_hist[i] / maximum * image.cols / 2;
      for (int j = 0; j < end; j++) {
        image.at<uchar>(i, j) = 0;
      }
    }
  }
  //return;
  // get the row's middle axis
  int window_size = 5;  // calculate the mean value in the window to get the gradient
  int max_id = 0;      // y_hist[max_id] has the maximum during the scan process
  vector<pair<double, int> > axises;  // the axises of each row (confidence, row_index)
  for (int i = 0; i < y_hist.size(); i++) {
    if (y_hist[i] > y_hist[max_id])
      max_id = i;
    int pre = max(i - window_size, 0), next = min(i + window_size,
                                                  image.rows - 1);
    double pre_mean = 0, next_mean = 0;
    for (int j = pre; j < i; j++) {
      pre_mean += y_hist[j];
    }
    for (int j = i + 1; j < next; j++) {
      next_mean += y_hist[j];
    }
    pre_mean /= (i - pre);
    next_mean /= (next - i - 1);
    if (pre_mean > y_hist[i] && next_mean >= y_hist[i]
        && y_hist[max_id] > y_hist[i]) {
      axises.push_back(make_pair(y_hist[max_id], max_id));
      y_hist[max_id] = 0;
    }
  }
  // draw the axis
  if (need_draw) {
    for (int i = 0; i < axises.size(); i++) {
      line(image, Point2i(0, axises[i].second),
           Point2i(image.cols, axises[i].second), 0);
    }
  }

  // use the axises to get the row
  sort(axises.begin(), axises.end(), std::greater<pair<double, int> >());  // as the confidence to sort descend
  map<int, vector<int> > row_map;  // key is row_index, value is the id list in this row
  vector<int> unit_line_num(seg_result->getResultSize(), -1);
  for (auto ax : axises) {
    int row_index = ax.second;
    // check if the row can be merged with other row
    int cnt = 0;
    set<int> other_row;
    for (auto unit_index : row_members[row_index]) {
      if (unit_line_num[unit_index] != -1) {
        other_row.insert(unit_line_num[unit_index]);
        cnt++;
      }
    }
    if (other_row.size() > 1) {
      continue;             // drop this axis
    }

    for (auto unit_index : row_members[row_index]) {
      if (unit_line_num[unit_index] == -1) {
        unit_line_num[unit_index] = cnt > 3 ? *other_row.begin()  // merge row
            : row_index;
      }
    }
  }
  for (int i = 0; i < seg_result->getResultSize(); i++) {
    row_map[unit_line_num[i]].push_back(i);
  }
  // handle the unit which does not allocate to any row
  vector<int> homelesser = row_map[-1];
  for (auto unit_index : homelesser) {
    ResultUnit ru = seg_result->getResultAt(unit_index);
    int y_center = (ru.bounding_box[0].y + ru.bounding_box[1].y) / 2;
    int nearest_row = -1;
    for (auto p : row_map) {
      if (p.first == -1)
        continue;
      if (abs(p.first - y_center) < (abs(nearest_row - y_center))) {
        nearest_row = p.first;
      }
    }
    row_map[nearest_row].push_back(unit_index);
  }

  // sort each row as their x coordinate
  vector<ResultUnit> new_result;
  int line_cnt = 0;
  for (auto row : row_map) {
    if (row.first == -1)
      continue;
    int start_index = new_result.size();
    for (auto index : row.second) {
      ResultUnit r = seg_result->getResultAt(index);
      r.line_index = line_cnt;
      new_result.push_back(r);
    }
    line_cnt++;
    sort(new_result.begin() + start_index, new_result.end(), CompX());
  }
  seg_result->setResult(new_result);
}

void Segmentator::handleEachRowInit(Mat &image,
                                    map<int, vector<ResultUnit> > &row_map) {
  confidence.clear();
  result_map.clear();
  confidence.resize(row_map.size());
  result_map.resize(row_map.size());
  set<string> do_not_handle;
  set<string> do_not_merge;
  do_not_merge.insert("1");do_not_merge.insert("2");
  do_not_merge.insert("3");do_not_merge.insert("4");
  do_not_merge.insert("5");do_not_merge.insert("6");
  do_not_merge.insert("7");do_not_merge.insert("8");
  do_not_merge.insert("9");do_not_merge.insert("0");
  do_not_merge.insert("年");do_not_merge.insert("月");
  do_not_merge.insert("日");do_not_merge.insert("￥");
  do_not_merge.insert("円");do_not_merge.insert("o");
  do_not_merge.insert("。");
  do_not_handle.insert("-");
  do_not_handle.insert("一");
  do_not_handle.insert(".");
  vector<Mat> imagelist;
  for (auto row : row_map) {
    vector<ResultUnit> &rs = row.second;
    confidence[row.first].resize(rs.size(), vector<float>(rs.size(), 0));
    result_map[row.first].resize(rs.size(), vector<int>(rs.size(), 0));
    for (int size = 1; size <= 4; size++) {
      for (int i = 0; i < (int) rs.size() - size + 1; i++) {
        vector<ResultUnit> segment;
        for (int k = i; k < i + size; k++) {
          segment.push_back(rs[k]);
        }
        ResultUnit merge_ru = ResultUnit::getMergeUnit(segment);
        Mat unit_img;
        General::cutImage(image, unit_img, merge_ru.bounding_box[0].x,
                          merge_ru.bounding_box[0].y, merge_ru.getWidth(),
                          merge_ru.getHeight());
        imagelist.push_back(unit_img);
      }
    }
  }
  result.clear();

  RecognitionByCNN::recognition(imagelist, result);
  cout << "rec " << result.size() << endl;
  int req_cnt = 0;
  for (auto &row : row_map) {
    vector<ResultUnit> &rs = row.second;
    // get histogram
    vector<int> x_histogram(image.cols, 0);
    double average_width = 0;
    for (ResultUnit &ru : rs) {
      for (int i = ru.bounding_box[0].y; i <= ru.bounding_box[1].y; i++)
        for (int j = ru.bounding_box[0].x; j <= ru.bounding_box[1].x; j++) {
          if (image.at<uchar>(i, j) == 0) {
            x_histogram[j]++;
          }
        }
      average_width += ru.getWidth();
    }
    average_width /= rs.size();
    vector<bool> is_num(rs.size(), false);
    vector<bool> cant_merge(rs.size(), false);
    for (int size = 1; size <= 4; size++) {
      if (size == 1) {
        for (int i = 0; i < (int) rs.size(); i++) {
          if (result[req_cnt + i].content.find_first_of("0123456789", 0)
              != string::npos) {
            is_num[i] = true;
          }
          if (do_not_merge.count(result[req_cnt + i].content))
          {
            cant_merge[i] = true;
            result[req_cnt + i].confidence = 100;
          }
        }
      }

      for (int i = 0; i < (int) rs.size() - size + 1; i++) {
        // get the largest width of the whitespace area
        int white_cnt = 0, largest_cnt = 0;
        if (size > 1)
        for (int j = rs[i].bounding_box[0].x;
            j <= rs[i + size - 1].bounding_box[1].x; j++) {
          if (x_histogram[j] == 0) {
            white_cnt++;
          } else {
            largest_cnt = max(white_cnt, largest_cnt);
            white_cnt = 0;
          }
        }
        /*
         vector<ResultUnit> segment;
         for ( int k = i; k < i + size; k++)
         {
         segment.push_back(rs[k]);
         }
         ResultUnit merge_ru = ResultUnit::getMergeUnit(segment);
         */
        // do not merge two numbers
        /*int num_cnt = 0;
        bool merge_ok = true;
        for (int k = i; k < i + size; k++) {
           if (is_num[k])
             num_cnt++;
           if (cant_merge[k])
             merge_ok = false;
        }
        if (num_cnt >= 2 || !merge_ok)
        {
           result[req_cnt].confidence = 0;
        }
        else if (result[req_cnt].confidence > 50
            && !do_not_handle.count(result[req_cnt].content))
          result[req_cnt].confidence = getSimilarity(imagelist[req_cnt],
                                                     result[req_cnt].content);

        double geometry_cost = largest_cnt / average_width * 50;
        if (geometry_cost > 0)
          result[req_cnt].confidence -= geometry_cost;
        else
          result[req_cnt].confidence += 10;*/
        if (size > 1)
        {
          if (do_not_merge.count(result[req_cnt].content))
          {
           result[req_cnt].confidence = 100;
          }
          else
          {
            result[req_cnt].confidence = 0;
          }
        }
        //result[req_cnt].bounding_box = merge_ru.bounding_box;
        confidence[row.first][i][i + size - 1] = result[req_cnt].confidence;
        ///cout << result[req_cnt].confidence << " " << result[req_cnt].content << endl;
        result_map[row.first][i][i + size - 1] = req_cnt++;
      }
    }
  }
}
void Segmentator::handleEachRow(cv::Mat &img, int row_index,
                                vector<ResultUnit> &row_units,
                                vector<vector<float> > &confi) {

  vector<vector<int>> choice(confi.size(), vector<int>(confi.size(), -1));  // memory the best choice

  for (int size = 1; size <= row_units.size(); size++) {
    for (int i = 0; i < (int) row_units.size() - size + 1; i++) {
      for (int k = i; k < i + size - 1; k++) {
        if (confi[i][i + size - 1]
            <= (confi[i][k] * (k - i + 1)
                + confi[k + 1][i + size - 1] * (i + size - 1 - k)) / size) {
          confi[i][i + size - 1] = (confi[i][k] * (k - i + 1)
              + confi[k + 1][i + size - 1] * (i + size - 1 - k)) / size;
          choice[i][i + size - 1] = k;
        }
      }
    }
  }
  // get all the segment
  set<int> split_index_set;
  getAllTheSplitIndex(split_index_set, 0, row_units.size() - 1, choice);
  vector<ResultUnit> new_result;
  vector<ResultUnit> temp_segment;
  int start_index = 0;
  for (int i = 0; i < row_units.size(); i++) {
    temp_segment.push_back(row_units[i]);
    if (split_index_set.count(i) || i == row_units.size() - 1) {
      ResultUnit ru = ResultUnit::getMergeUnit(temp_segment);
      ResultUnit &reco_ru = result[result_map[row_index][start_index][i]];
      ru.confidence = reco_ru.confidence;
      ru.content = reco_ru.content;
      //cout << ru.content << " " << ru.confidence << endl;
      start_index = i + 1;
      new_result.push_back(ru);
      temp_segment.clear();
    }
  }
  row_units = new_result;
}
void Segmentator::getAllTheSplitIndex(set<int>& split_index_set, int start,
                                      int end, vector<vector<int> > &choice) {
  if (choice[start][end] != -1) {
    split_index_set.insert(choice[start][end]);
    getAllTheSplitIndex(split_index_set, start, choice[start][end], choice);
    getAllTheSplitIndex(split_index_set, choice[start][end] + 1, end, choice);
  }
}

double Segmentator::getSimilarity(Mat& img, string character) {
  //cout << "handle: " << character << endl;
  if (character == "\\" || character == "/") {
    character = "slash";
  }

  string prototype_path = "/home/michael/workspace_python/characterData/"
      + character + "/" + character + "_fonts-japanese-gothic.ttf_0.png";
  ifstream prototype_file(prototype_path);
  if (prototype_file.fail()) {
    cout << "fail to open prototype file: " << character << endl;
    return 0;
  } else {
    prototype_file.close();
  }
  Mat prototype_img = imread(prototype_path, CV_LOAD_IMAGE_GRAYSCALE);
  return ImageSimilarity::getSimilarity(img, prototype_img);
}
