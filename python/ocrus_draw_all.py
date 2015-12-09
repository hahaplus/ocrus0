#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys


prog_bounding_box = 'ocrus_bounding_box'
prog_draw_bbox = 'ocrus_draw_bbox.py'

if len(sys.argv) != 5:
    print '''Output bounding box result and draw the result to images

Usage: %s text|draw|both page_seg_mode symbol|word|both path_image_list
mode
    text: output bbox result to text file
    draw: draw the bounding box, recognized result and confidence
    both: both text and draw
page_seg_mode: An enum integer from Tesseract
level
    symbol: symbol only
    word: word only
    both: both symbol and word
path_image_list
    A list of path of images, one path in one line''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

mode = sys.argv[1]
page_seg_mode = sys.argv[2]
level = sys.argv[3]
path_image_list = sys.argv[4].rstrip('/')

levels = ['symbol']
if level == 'symbol':
    levels = ['symbol']
elif level == 'word':
    levels = ['word']
elif level == 'both':
    levels = ['symbol', 'word']

for path_image in open(path_image_list):
    path_image = path_image.strip()
    print 'Processing', path_image, '...'

    if not os.path.exists(path_image):
        continue

    for level in levels:
        if mode in ['text', 'both']:
            cmd = '''{prog_bounding_box} {page_seg_mode} {level} {path_image} \
> {path_image}_{level}.txt'''.\
                format(prog_bounding_box=prog_bounding_box,
                       page_seg_mode=page_seg_mode, level=level,
                       path_image=path_image)
            print 'Running cmd:', cmd
            os.system(cmd)

        if mode in ['draw', 'both']:
            cmd = '''{prog_draw_bbox} {path_image}_binarize.png \
{path_image}_{level}.txt {path_image}_{level}.png'''.\
                format(prog_draw_bbox=prog_draw_bbox, level=level,
                       path_image=path_image)
            print 'Running cmd:', cmd
            os.system(cmd)
