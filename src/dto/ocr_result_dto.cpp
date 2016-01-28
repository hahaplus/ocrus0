/*
 * ocr_result_dto.cpp
 *
 *  Created on: Dec 2, 2015
 *      Author: michael
 */
#include "ocr_result_dto.h"
#include <set>
const int OcrDetailResult::SYMBOL = 0;
const int OcrDetailResult::TEXT = 1;


void OcrDetailResult::mergeOcrResult(cv::Mat &main_img, cv::Mat assit_img,
                            OcrDetailResult* main_result,
                            OcrDetailResult* assit_result)
 {
   // write the main_result into main_map
   vector<vector<int> > main_map(main_img.rows, vector<int>(main_img.cols, -1));
   for (int i = 0; i < main_result->getResultSize(); i++) {
     ResultUnit ru = main_result->getResultAt(i);
     for (int j = 0; j < ru.getHeight(); j++)
       for (int k = 0; k < ru.getWidth(); k++) {
         main_map[j + ru.bounding_box[0].y][k + ru.bounding_box[0].x] = i;
       }
   }

   // check the assist_result
   // if it is added into new_result
   vector<bool> is_added(main_result->getResultSize(), false);
   map<int, vector<ResultUnit> >  add_map;
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

     if (index_set.size() != 0) {
       //assert(*index_set.begin() >=0 && *index_set.begin() < main_result->getResultSize() );
       double average = 0;
       for (set<int>::iterator it = index_set.begin(); it != index_set.end(); it++)
       {
         //cout << "index" << *it << " " << main_result->getResultSize()<< endl;
         ResultUnit ori_ru = main_result->getResultAt(*it);
         average += ori_ru.confidence;
       }
       average /= index_set.size();

       if (average <= ru.confidence) {
         //assert((ori_ru.getHeight() * ori_ru.getWidth()) != 0);
         // replace the result
         //new_result.push_back(ru);
         add_map[*index_set.begin()].push_back(ru);
         for (auto index: index_set)
           is_added[index] = true;
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
   // merge result
    vector<ResultUnit> new_result;
   for (int i = 0; i < main_result->getResultSize(); i++)
   {
       if (is_added[i] == false)
       {
         new_result.push_back(main_result->getResultAt(i));
       }
       else if (add_map.count(i))
       {
         for (auto &ru : add_map[i])
         {
           new_result.push_back(ru);
         }
       }
   }
   main_result->setResult(new_result);
 }
