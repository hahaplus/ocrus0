#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json


if len(sys.argv) != 4:
    print '''Calculate accuracy for Photo-0000 to Photo-0027 images.

Usage: %s symbol|word dir_gt dir_ocr_lines
dir_gt
    Where the ground truth is stored
dir_ocr_lines
    Where the OCR lines result is stored''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

level = sys.argv[1]
dir_gt = sys.argv[2].rstrip('/')
dir_ocr_lines = sys.argv[3].rstrip('/')

num_photos = range(0, 28)
num_photos.remove(2)

num_photos = map(lambda x: '%04d' % x, num_photos)


def bbox_area(box):
    '''
    x1,y1
    -------------
    |           |
    |           |
    |           |
    ------------x2,y2
    '''
    x1, y1, x2, y2 = box[0], box[1], box[2], box[3]
    return (x2 - x1) * (y2 - y1)


def bbox_almost_overlap(bbox1, bbox2):
    '''
    x1,y1
    -------------
    |  x3,y3    |
    |    -------------------
    |    |      |          |
    ------------x2,y2      |
         |                 |
         -------------------x4,y4
    '''
    x1, y1, x2, y2 = bbox1[0], bbox1[1], bbox1[2], bbox1[3]
    x3, y3, x4, y4 = bbox2[0], bbox2[1], bbox2[2], bbox2[3]
    bbox_overlap = max(x1, x3), max(y1, y3), min(x2, x4), min(y2, y4)
    return float(bbox_area(bbox_overlap)) / (bbox_area(bbox1) +
                                             bbox_area(bbox2)) > 0.3

num_lines = 0
num_correct_lines = 0

for num_photo in num_photos:
    print 'Processing photo', num_photo, '...'

    path_gt = '{dir_gt}/Photo-{num_photo}_{level}_gt.json'.\
        format(dir_gt=dir_gt, num_photo=num_photo, level=level)

    path_ocr_lines = '{dir_txt}/Photo-{num_photo}_{level}_ocr_lines.json'.\
        format(dir_txt=dir_ocr_lines, num_photo=num_photo, level=level)

    ocr_lines_gt = json.load(open(path_gt))
    ocr_lines = json.load(open(path_ocr_lines))

    num_lines += len(ocr_lines_gt)

    for ocr_line in ocr_lines:

        line_ok = False

        for ocr_line_gt in ocr_lines_gt:
            if ocr_line['type'] != ocr_line_gt['type'] or \
                    len(ocr_line['chars']) != len(ocr_line_gt['chars']):
                continue

            texts = [ch['text'] for ch in ocr_line['chars']]
            texts_gt = [ch['text'] for ch in ocr_line_gt['chars']]
            if texts != texts_gt:
                continue

            box_ok = True
            for pos in range(len(ocr_line['chars'])):
                box = ocr_line['chars'][pos]['bounding_box']
                box_gt = ocr_line_gt['chars'][pos]['bounding_box']
                if not bbox_almost_overlap(box, box_gt):
                    box_ok = False
                    break

            if box_ok:
                line_ok = True
                break

        if line_ok:
            print 'Line correct: ', u''.join(texts)
            num_correct_lines += 1

accuracy = float(num_correct_lines) / num_lines
print 'Accuracy: %.2f%% (%d/%d)' % (accuracy, num_correct_lines, num_lines)
