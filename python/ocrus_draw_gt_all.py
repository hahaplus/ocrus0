#!/usr/bin/python
# -*- coding: utf-8 -*-

import shutil
import os
import sys
from ocrus.ocr_drawing import draw_ocr_lines
from ocrus.ocr_result import normalize_ocr_lines

if len(sys.argv) != 2:
    print '''Draw OCR ground truth to images

Usage: %s path_image_list

path_image_list
    A list of path of images, one path in one line''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

path_image_list = sys.argv[1].rstrip('/')

for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue
    print 'Draw for', path_image, '...'

    path_ocr_lines = '{path_image}_gt.json'.\
        format(path_image=path_image)
    normalize_ocr_lines(path_ocr_lines)
    path_img = '{path_image}_binarize.png'.\
        format(path_image=path_image)
    path_output_img = '{path_image}_gt.png'.\
        format(path_image=path_image)

    draw_ocr_lines(path_ocr_lines, path_img, path_output_img)

    dir_gt = os.path.join(os.path.dirname(path_output_img), 'gt')
    if not os.path.exists(dir_gt):
        os.mkdir(dir_gt)
    shutil.copy(path_output_img, dir_gt)

    dir_gt_bad = os.path.join(os.path.dirname(path_output_img), 'gt_bad')
    if not os.path.exists(dir_gt_bad):
        os.mkdir(dir_gt_bad)
