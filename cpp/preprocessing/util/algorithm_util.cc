/*
 * algorithm_util.cpp
 *
 *  Created on: Jan 19, 2016
 *      Author: michael
 */
#include "algorithm_util.h"

int AlgorithmUtil::connect_threshold;

cv::Rect2i AlgorithmUtil::getBoundingBox(vector<pair<int,int> > &block)
{
    pair<int, int> left_top(1e6, 1e6), right_bottom(-1e6, -1e6);
      for (auto b : block) {
        left_top.second = min(left_top.second, b.second);
        right_bottom.second = max(right_bottom.second, b.second);
        left_top.first = min(left_top.first, b.first);
        right_bottom.first = max(right_bottom.first, b.first);
      }

    return  Rect2i (left_top.second, left_top.first, right_bottom.second - left_top.second + 1,
                      right_bottom.first - left_top.first + 1);;
}
