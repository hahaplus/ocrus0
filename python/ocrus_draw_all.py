#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import multiprocessing


def run_cmd_list(cmd_list, count, count_all):
    print "Progress: %d/%d" % (count, count_all)
    for cmd in cmd_list:
        print 'Running cmd:', cmd
        os.system(cmd)

prog_bounding_box = 'ocrus_bounding_box'
prog_draw_bbox = 'ocrus_draw_bbox.py'

if len(sys.argv) != 6:
    print '''Output bounding box result and draw the result to images

Usage: %s prog_bounding_box text|draw|both page_seg_mode symbol|word|both path_image_list
prog_bounding_box
    Path of a program that accepts parameters as:
        page_seg_mode level path_image
    This program *MUST* output to stdout, an example line:
        word: 'g';      conf: 61.91; BoundingBox: 411,1778,431,1906;
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

prog_bounding_box = sys.argv[1]
mode = sys.argv[2]
page_seg_mode = sys.argv[3]
level = sys.argv[4]
path_image_list = sys.argv[5].rstrip('/')

levels = ['symbol']
if level == 'symbol':
    levels = ['symbol']
elif level == 'word':
    levels = ['word']
elif level == 'both':
    levels = ['symbol', 'word']

tasks = []

for path_image in open(path_image_list):
    path_image = path_image.strip()
    print 'Processing', path_image, '...'

    if not os.path.exists(path_image):
        continue

    cmd_list = []

    for level in levels:
        if mode in ['text', 'both']:
            cmd = '''{prog_bounding_box} {page_seg_mode} {level} {path_image} \
> {path_image}_{level}.txt'''.\
                format(prog_bounding_box=prog_bounding_box,
                       page_seg_mode=page_seg_mode, level=level,
                       path_image=path_image)
            cmd_list.append(cmd)

        if mode in ['draw', 'both']:
            cmd = '''{prog_draw_bbox} {path_image}_binarize.png \
{path_image}_{level}.txt {path_image}_{level}.png'''.\
                format(prog_draw_bbox=prog_draw_bbox, level=level,
                       path_image=path_image)
            cmd_list.append(cmd)

    tasks.append(cmd_list)

pool = multiprocessing.Pool()
for count, cmd_list in enumerate(tasks):
    pool.apply_async(run_cmd_list, [cmd_list, count + 1, len(tasks)])
pool.close()
pool.join()
