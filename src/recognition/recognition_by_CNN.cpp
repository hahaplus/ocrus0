/*
 * RecognitionByCNN.cpp
 *
 *  Created on: Jan 22, 2016
 *      Author: michael
 */

#include <recognition/recognition_by_CNN.h>
#include <fstream>

using namespace std;
using namespace cv;
vector<string> RecognitionByCNN::dict;
string RecognitionByCNN::dict_path =
    "/home/michael/workspace/ocrus0_build/networkModel/chars_id_full.txt";
//for the deeplearning
PyObject* RecognitionByCNN::pMod = NULL;
PyObject* RecognitionByCNN::single_img_func = NULL,
    *RecognitionByCNN::multi_img_func = NULL;
PyObject* RecognitionByCNN::pDict = NULL;

RecognitionByCNN::RecognitionByCNN() {
  // TODO Auto-generated constructor stub

}
void RecognitionByCNN::init() {
  loadDict();
  // Load the network module
  Py_Initialize();
  pMod = PyImport_ImportModule("ocrus.neural_network.network_api");
  pDict = PyModule_GetDict(pMod);
  single_img_func = PyDict_GetItemString(pDict, "recognition_img");
  multi_img_func = PyDict_GetItemString(pDict, "recognition_img_list");
}
void RecognitionByCNN::release()
{
  Py_Finalize();
}
void RecognitionByCNN::loadDict() {
  ifstream dict_file(dict_path.c_str());
  while (!dict_file.eof()) {
    int id;
    string val;
    dict_file >> id >> val;
    dict.push_back(val);
    //cout << val << endl;
  }
  dict_file.close();
}
void RecognitionByCNN::recognition(const cv::Mat &img, ResultUnit &result) {
  // format the img
  // make the image to 28 * 28 without distortion
  if (!pMod) {
    init();
  }
  int square_size = max(img.rows, img.cols);
  Mat square(square_size, square_size, CV_8UC1, 255);
  int offset_x = (square_size - img.cols) / 2;
  int offset_y = (square_size - img.rows) / 2;
  img.copyTo(square(cv::Rect(offset_x, offset_y, img.cols, img.rows)));
  cv::resize(square, square, Size(28, 28));
  // sharpen
  Mat kernel(3, 3, CV_32F, Scalar(-1)), sharp_result;
  kernel.at<float>(1, 1) = 9;
  // for (int i = 0; i < 3; i++)
  filter2D(square, square, square.depth(), kernel);
  //General::showImage(square);
  // convert the image to  1d vector and rescale the value in range between (0, 1)
  // 0 is white,overlap 1 is black
  vector<float> input_feature;
  for (int r = 0; r < square.rows; r++)
    for (int c = 0; c < square.cols; c++) {
      input_feature.push_back((255.0 - square.at<uchar>(r, c)) / 255.0);
    }
  // send the feature vector to the neutral network
  if (!single_img_func)
    exit(-2);
  if (PyCallable_Check(single_img_func)) {
    PyObject* pParm_tuple = PyTuple_New(input_feature.size());
    for (int i = 0; i < input_feature.size(); i++) {
      PyObject* pValue = Py_BuildValue("f", input_feature[i]);
      if (!pValue) {
        PyErr_Print();
        return;
      }
      PyTuple_SetItem(pParm_tuple, i, pValue);
    }
    PyObject*ret_list = PyObject_CallObject(single_img_func, pParm_tuple);
    //printf("here3");
    int target_id = 0;
    for (int i = 0; i < PyTuple_GET_SIZE(ret_list); i++) {
      PyObject* value = PyTuple_GetItem(ret_list, i);
      float item_value = 0;
      if (i == 0) {
        float characer_id;
        PyArg_Parse(value, "f", &characer_id);
        result.content = dict[int(characer_id + 1e-6)];
      } else if (i - 1 == target_id) {
        PyArg_Parse(value, "f", &item_value);
        result.confidence = item_value * 100;
      }
      //result.candidates.push_back(pair(valueint))
    }
    printf("%s %f\n", result.content.c_str(), result.confidence);
  }
}
void RecognitionByCNN::recognition(const std::vector<cv::Mat> &img_list,
                                   std::vector<ResultUnit> &result) {
  if (!pMod) {
    init();
  }
  vector<float> input_feature;
  result.resize(img_list.size());
  Size img_size(28, 28);
  for (auto img : img_list) {
    int square_size = max(img.rows, img.cols);
    Mat square(square_size, square_size, CV_8UC1, 255);
    int offset_x = (square_size - img.cols) / 2;
    int offset_y = (square_size - img.rows) / 2;
    img.copyTo(square(cv::Rect(offset_x, offset_y, img.cols, img.rows)));
    cv::resize(square, square, img_size);
    // sharpen
    Mat kernel(3, 3, CV_32F, Scalar(-1)), sharp_result;
    kernel.at<float>(1, 1) = 9;
    // for (int i = 0; i < 3; i++)
    filter2D(square, square, square.depth(), kernel);
    //General::showImage(square);
    // convert the image to  1d vector and rescale the value in range between (0, 1)
    // 0 is white, 1 is black

    for (int r = 0; r < square.rows; r++)
      for (int c = 0; c < square.cols; c++) {
        input_feature.push_back((255.0 - square.at<uchar>(r, c)) / 255.0);
      }
  }
  // send the feature vector to the neutral network
  if (!multi_img_func)
    exit(-2);
  if (PyCallable_Check(multi_img_func)) {
    PyObject* pParm_tuple = PyTuple_New(input_feature.size() + 1);

    for (int i = 0; i < input_feature.size(); i++) {
      PyObject* pValue = Py_BuildValue("f", input_feature[i]);
      if (!pValue) {
        PyErr_Print();
        return;
      }
      PyTuple_SetItem(pParm_tuple, i, pValue);
    }
    // input the number of image
    PyTuple_SetItem(pParm_tuple, input_feature.size(),
                    Py_BuildValue("i", img_size.height * img_size.width));
    PyObject*ret_list = PyObject_CallObject(multi_img_func, pParm_tuple);
    //printf("here3");
    int target_id = 0;
    for (int i = 0; i < PyTuple_GET_SIZE(ret_list); i++) {
      PyObject* value = PyTuple_GetItem(ret_list, i);
      float item_value = 0;
      if ((i % 2) == 0) {
        float characer_id;
        PyArg_Parse(value, "f", &characer_id);
        result[i / 2].content = dict[int(characer_id + 1e-6)];
      } else {
        PyArg_Parse(value, "f", &item_value);
        result[i / 2].confidence = item_value * 100;
      }
      //result.candidates.push_back(pair(valueint))
    }
    //for (auto ru : result)
    //    printf("%s %f\n", ru.content.c_str(), ru.confidence);
  }
}
RecognitionByCNN::~RecognitionByCNN() {
  // TODO Auto-generated destructor stub
}

