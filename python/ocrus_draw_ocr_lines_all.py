#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from ocrus.ocr_drawing import draw_ocr_lines

if len(sys.argv) != 2:
    print '''Draw OCR lines to images

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

    path_ocr_lines = '{path_image}_ocr_lines.json'.\
        format(path_image=path_image)
    path_img = '{path_image}_binarize.png'.\
        format(path_image=path_image)
    path_output_img = '{path_image}_ocr_lines.png'.\
        format(path_image=path_image)

    draw_ocr_lines(path_ocr_lines, path_img, path_output_img)
