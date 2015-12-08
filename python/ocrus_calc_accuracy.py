#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json

from ocrus.stats_analysis import calc_correct_lines, pretty_print_stats

if len(sys.argv) not in [2, 3]:
    print '''Calculate accuracy for Photo-0000 to Photo-0027 images.

Usage: %s path_image_list [base_stats_json]

path_image_list
    A list of path of images, one path in one line
base_stats_json
    If specified, the stats will be dumped in base_stats_json''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

path_image_list = sys.argv[1].rstrip('/')
base_stats = None
if len(sys.argv) == 3:
    base_stats = sys.argv[2]

each_num_lines = {}
each_num_correct_lines = {}
each_accuracy = {}

num_lines = 0
num_correct_lines = 0

stats = []

for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue
    print 'Processing', path_image, '...'

    id_image = os.path.basename(path_image)

    path_gt = '{path_image}_gt.json'.\
        format(path_image=path_image)

    path_ocr_lines = '{path_image}_ocr_lines.json'.\
        format(path_image=path_image)

    ocr_lines_gt = json.load(open(path_gt))
    ocr_lines = json.load(open(path_ocr_lines))

    each_num_lines[id_image] = len(ocr_lines_gt)
    each_num_correct_lines[id_image] = calc_correct_lines(
        ocr_lines_gt, ocr_lines)

    num_lines += len(ocr_lines_gt)
    num_correct_lines += each_num_correct_lines[id_image]

    each_accuracy[id_image] = float(
        each_num_correct_lines[id_image]) / each_num_lines[id_image]

    stats.append({'id_image': id_image,
                  'num_lines': each_num_lines[id_image],
                  'num_correct_lines': each_num_correct_lines[id_image],
                  'num_wrong_lines':
                  each_num_lines[id_image] -
                      each_num_correct_lines[id_image],
                  'accuracy': each_accuracy[id_image]})

stats.sort(key=lambda x: x['num_wrong_lines'])
pretty_print_stats(stats)
if base_stats:
    json.dump(stats, open(base_stats, 'w'), indent=2)

accuracy = float(num_correct_lines) / num_lines
print 'Accuracy: %.3f (%d/%d)' % (accuracy, num_correct_lines, num_lines)
