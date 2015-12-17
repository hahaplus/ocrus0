/*
 * algorithm_util.h
 *
 *  Created on: Dec 3, 2015
 *      Author: michael chen
 */

#ifndef SRC_UTIL_ALGORITHM_UTIL_H_
#define SRC_UTIL_ALGORITHM_UTIL_H_
#include <vector>
#include <opencv2/opencv.hpp>
#include <queue>
#include <cmath>
using namespace std;
using namespace cv;

 class AlgorithmUtil {
 public:
  AlgorithmUtil()
  {}
  virtual ~AlgorithmUtil();
  /*
   * floodFill in a mat.
   * the pixel which get a distance between seed_value that is less or equal than connect_threshold will be judged as a seed.
   * when the difference between pixel is less or equal than connect_threshold will be judged as adjacent.
   * T is related to the channels of the img, if img is a single channel img, please set T uchar, and if img has three channels
   * please set T as Vec3b
   *
   * distance = sqrt(sum((ai-bi)^2))  a, b are elemet T. i range(1, channels)
   *
   * AlgorithmUtil
   * the pair is (row, col)
   * return all adjacent location block
   */
  template <typename T>
  static std::vector<std::vector<std::pair<int, int> > > floodFillInMat(const cv::Mat &img, T seed_value, int connect_threshold);
  /*
    * get mean value of a vector
    */
   template <typename T>
   static T getAverageValue(vector<T>&);
private:
  static int connect_threshold;

  template <typename T>
  static bool isCon(T pixa, T pixb);
  /*
   * return the blocks of (row, col) location of the pixels that are similar with seed.
   *
   */
  template <typename T>
  static std::vector<std::pair<int,int> > bfs(std::vector<std::vector<bool> > &is_visited, const cv::Mat &img, std::pair<int, int> seed);

};

template <typename T>
bool AlgorithmUtil::isCon(T pixa, T pixb) {
     double sum = 0;
     for (int i = 0; i < pixa.channels; i++)
         sum += (pixa[i] - pixb[i]) * (pixa[i] - pixb[i]);
     return sqrt(sum) <= connect_threshold;
}
template <typename T>
vector<pair<int,int> > AlgorithmUtil::bfs(std::vector<std::vector<bool> > &is_visited,
                               const cv::Mat &img, std::pair<int, int> seed)
{
    vector<pair<int,int> > res;
    int dir[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};

    queue<pair<int,int> > que;
    que.push( seed );

    while (!que.empty())
    {
        pair<int,int> out = que.front();
        que.pop();
        if (is_visited[out.first][out.second]) continue;
        res.push_back(out); // return the (row, col) location of the pixel
        is_visited[out.first][out.second] = true;
        for (int i = 0; i < 4; i++)
        {
            pair<int,int> new_pos(out.first + dir[i][0], out.second + dir[i][1]);
            if (new_pos.first >= img.rows || new_pos.first < 0
                || new_pos.second >= img.cols || new_pos.second < 0 || is_visited[new_pos.first][new_pos.second]
                || !isCon(img.at<T>(seed.first, seed.second), img.at<T>(new_pos.first, new_pos.second)))
            {
                continue; // out of bound or has visited or disconnected
            }
            que.push(new_pos);
        }
    }
    return res;
}

template <typename T>
std::vector<std::vector<std::pair<int, int> > > AlgorithmUtil::floodFillInMat(
    const cv::Mat &img, T seed_value, int connect_threshold) {

  vector<vector<pair<int, int> > > blocks;
  AlgorithmUtil::connect_threshold = connect_threshold;
  vector<vector<bool> > is_visited(img.rows, vector<bool>(img.cols, false));
  for (int i = 0; i < img.rows; i++)
    for (int j = 0; j < img.cols; j++) {
        if (!is_visited[i][j] && isCon(seed_value, img.at<T>(i, j)))
        {
              blocks.push_back(bfs<T>(is_visited, img, make_pair(i,j)));
        }
    }
  return blocks;
}
template <typename T>
T AlgorithmUtil::getAverageValue(vector<T> &list)
{
    T res = 0;
    for (auto a :list)
    {
        res += a;
    }
    res /= list.size();
    return res;
}
#endif /* SRC_UTIL_ALGORITHM_UTIL_H_ */
