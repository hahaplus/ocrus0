/*
 * denoise_line_point.cpp
 *
 *  Created on: Dec 9, 2015
 *      Author: michael
 */

#include <preprocessing/denoise/denoise_line_point.h>
#include <vector>
using namespace cv;
using namespace std;

DenoiseLinePoint::DenoiseLinePoint() {
  // TODO Auto-generated constructor stub

}

DenoiseLinePoint::~DenoiseLinePoint() {
  // TODO Auto-generated destructor stub
}
void DenoiseLinePoint::removeNoise(Mat &src) {
  //return;
  vector<vector<pair<int, int> > > blocks = AlgorithmUtil::floodFillInMat<
      Vec<uchar, 1> >(src, 0, 0);
  vector<int> width, height;
  vector<vector<pair<int, int> > > new_blocks;
  int iter_cnt = 3;
  while (iter_cnt--) {
    // reomve small noise points
    for (auto block : blocks) {
      pair<int, int> left_top(1e6, 1e6), right_bottom(-1e6, -1e6);
      for (auto point : block) {
        left_top.first = min(left_top.first, point.first);
        left_top.second = min(left_top.second, point.second);
        right_bottom.first = max(right_bottom.first, point.first);
        right_bottom.second = max(right_bottom.second, point.second);
      }
      width.push_back(right_bottom.second - left_top.second);
      height.push_back(right_bottom.first - left_top.first);
    }
    int avg_width = AlgorithmUtil::getAverageValue(width);
    int avg_height = AlgorithmUtil::getAverageValue(height);
    for (int i = 0; i < blocks.size(); i++) {
      if ((width[i] < avg_width * 0.5 && height[i] < avg_height * 0.5)) {
        for (auto pix : blocks[i]) {
          src.at<uchar>(pix.first, pix.second) = 255;
        }
      } else {
        new_blocks.push_back(blocks[i]);
      }
    }

    blocks = new_blocks;

    new_blocks.clear(), width.clear(), height.clear();
    // reomve large noise points and line
    for (auto block : blocks) {
      pair<int, int> left_top(1e6, 1e6), right_bottom(-1e6, -1e6);
      for (auto point : block) {
        left_top.first = min(left_top.first, point.first);
        left_top.second = min(left_top.second, point.second);
        right_bottom.first = max(right_bottom.first, point.first);
        right_bottom.second = max(right_bottom.second, point.second);
      }
      width.push_back(right_bottom.second - left_top.second);
      height.push_back(right_bottom.first - left_top.first);
    }
    avg_width = AlgorithmUtil::getAverageValue(width);
    avg_height = AlgorithmUtil::getAverageValue(height);
    for (int i = 0; i < blocks.size(); i++) {
      if ((width[i] > 5 * avg_width) || (height[i] > 5 * avg_height)) {
        for (auto pix : blocks[i]) {
          src.at<uchar>(pix.first, pix.second) = 255;
        }
      } else {
        new_blocks.push_back(blocks[i]);
      }
    }
    blocks = new_blocks;
    new_blocks.clear(), width.clear(), height.clear();
  }
}
