/*
 * Recognition functionalities
 *
 * Copyright (C) 2015 Works Applications, all rights reserved
 *
 * Written by Chang Sun
 */

#ifndef OCRUS_RECOGNITION_H_
#define OCRUS_RECOGNITION_H_

#include <string>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "dto/ocr_result_dto.h"

namespace ocrus {

/**
 * OCR a grayscale image and print the word, confidence and bounding box
 * line by line to stdout.
 * An example line:
 * word: 'g';      conf: 61.91; BoundingBox: 411,1778,431,1906;
 *
 * @param src A grayscale image
 * @param page_seg_mode Page segmentation mode in Tesseract
 * @param level This value can be tesseract::RIL_WORD or tesseract::RIL_SYMBOL
 * @param lang Language setting that tesseract accepts, eg. eng+jpn
 */
void ocrPrintBoundingBox(const cv::Mat& src,
                         tesseract::PageSegMode page_seg_mode,
                         tesseract::PageIteratorLevel level,
                         const std::string& lang);

/**
 * OCR a grayscale image and print the word, confidence, bounding box and
 * candidates
 *
 * @param gray_image: A grayscale image
 * @param lang: Language setting that tesseract accepts, eg. eng+jpn
 */
void ocrPrintCandidates(const cv::Mat& gray_img, const std::string& lang);

/**
 * Draw the OCR result to image
 *
 * @param src_img Image to draw on
 * @param result OCR result
 * @param out_img Image with result drawn on
 */
void drawOcrResult(const cv::Mat &src_img, const OcrDetailResult& result,
                   cv::Mat *out_img);

}  // namespace ocrus

#endif  // OCRUS_RECOGNITION_H_
