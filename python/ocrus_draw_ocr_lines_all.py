#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
from ocrus.ocr_drawing import draw_ocr_lines

if len(sys.argv) != 3:
    print '''Draw OCR lines to Photo-0000 - Photo-0027 images.

Usage: %s dir_ocr_lines dir_binary_img''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

dir_ocr_lines = sys.argv[1].rstrip('/')
dir_binary_img = sys.argv[2].rstrip('/')

num_photos = range(0, 28)
num_photos.remove(2)

num_photos = map(lambda x: '%04d' % x, num_photos)

for num_photo in num_photos:
    path_ocr_lines = '{dir_ocr_lines}/Photo-{num_photo}_symbol_ocr_lines.json'.\
        format(dir_ocr_lines=dir_ocr_lines, num_photo=num_photo)
    path_img = '{dir_binary_img}/Photo-{num_photo}.jpg_binarize.png'.format(
        dir_binary_img=dir_binary_img,
        num_photo=num_photo)
    path_output_img = '{dir_ocr_lines}/Photo-{num_photo}_ocr_lines.png'.format(
        dir_ocr_lines=dir_ocr_lines,
        num_photo=num_photo)
    print 'Draw', num_photo, '...'
    draw_ocr_lines(path_ocr_lines, path_img, path_output_img)
