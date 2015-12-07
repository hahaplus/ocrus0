#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys


prog_bounding_box = 'ocrus_bounding_box'
prog_draw_bbox = 'ocrus_draw_bbox.py'

if len(sys.argv) != 5:
    print '''Process all the Photo-0000 to Photo-0027 images.
Output bounding box result and draw the result to image

Usage: %s text|draw|both page_seg_mode symbol|word|both dir_txt
mode
    text: output bbox result to text file
    draw: draw the bounding box, recognized result and confidence
    both: both text and draw
page_seg_mode: An enum integer from Tesseract
level
    symbol: symbol only
    word: word only
    both: both symbol and word
dir_txt
    Where the Photo-0000.jpg to Photo-0027 are stored''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

mode = sys.argv[1]
page_seg_mode = sys.argv[2]
level = sys.argv[3]
dir_txt = sys.argv[4].rstrip('/')

levels = ['symbol']
if level == 'symbol':
    levels = ['symbol']
elif level == 'word':
    levels = ['word']
elif level == 'both':
    levels = ['symbol', 'word']

num_photos = range(0, 28)
num_photos.remove(2)

num_photos = map(lambda x: '%04d' % x, num_photos)

for num_photo in num_photos:
    for level in levels:
        if mode in ['text', 'both']:
            cmd = '''{prog_bounding_box} {page_seg_mode} {level} {dir_txt}/Photo-{num_photo}.jpg \
> Photo-{num_photo}_{level}.txt
'''.format(prog_bounding_box=prog_bounding_box,
                page_seg_mode=page_seg_mode, level=level,
                dir_txt=dir_txt, num_photo=num_photo)
            print 'Running cmd:', cmd
            os.system(cmd)

        if mode in ['draw', 'both']:
            cmd = '''{prog_draw_bbox} {dir_txt}/Photo-{num_photo}.jpg_binarize.png \
Photo-{num_photo}_{level}.txt Photo-{num_photo}_{level}.png
'''.format(prog_draw_bbox=prog_draw_bbox, level=level,
                dir_txt=dir_txt, num_photo=num_photo)
            print 'Running cmd:', cmd
            os.system(cmd)
