#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json
import shutil
from PIL import Image

from ocrus.stats_analysis import calc_correct_lines, pretty_print_stats

if len(sys.argv) not in [2, 3, 4]:
    print '''Calculate accuracy for images.

Usage: %s path_image_list [path_stats_json] [date|money]

path_image_list
    A list of path of images, one path in one line
path_stats_json
    If specified, the stats will be dumped in path_stats_json
date|money
    If specified, stats is calcuated among date or money only''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

path_image_list = sys.argv[1].rstrip('/')
base_stats = None
line_type = None
if len(sys.argv) == 3:
    base_stats = sys.argv[2]
elif len(sys.argv) == 4:
    base_stats = sys.argv[2]
    line_type = sys.argv[3]

each_num_lines = {}
each_num_correct_lines = {}
each_accuracy = {}

num_lines = 0
num_correct_lines = 0

stats = []

dirs_bad_line = set()
dirs_good = set()
dirs_bad = set()
for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue

    dir_bad_line = os.path.join(os.path.dirname(path_image), 'bad_lines')
    dirs_bad_line.add(dir_bad_line)

    dir_good = os.path.join(os.path.dirname(path_image), 'good')
    dirs_good.add(dir_good)

    dir_bad = os.path.join(os.path.dirname(path_image), 'bad')
    dirs_bad.add(dir_bad)

for dirs in [dirs_bad_line, dirs_good, dirs_bad]:
    for dir_ in dirs:
        if os.path.exists(dir_):
            shutil.rmtree(dir_)
        os.mkdir(dir_)

for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue

    id_image = os.path.basename(path_image)

    path_gt = '{path_image}_gt.json'.\
        format(path_image=path_image)

    path_ocr_lines = '{path_image}_ocr_lines.json'.\
        format(path_image=path_image)

    ocr_lines_gt = json.load(open(path_gt))
    ocr_lines = json.load(open(path_ocr_lines))

    if line_type is not None:
        ocr_lines_gt = [ocr_line for ocr_line in ocr_lines_gt
                        if ocr_line['type'] == line_type]
        ocr_lines = [ocr_line for ocr_line in ocr_lines
                     if ocr_line['type'] == line_type]

    each_num_lines[id_image] = len(ocr_lines_gt)
    each_num_correct_lines[id_image] = calc_correct_lines(
        ocr_lines_gt, ocr_lines)

    path_symbol_img = path_image + '_symbol.png'
    dir_bad_line = os.path.join(os.path.dirname(path_image), 'bad_lines')

    if not all([ocr_line_gt['recognized'] for ocr_line_gt in ocr_lines_gt]):
        shutil.copy(path_image + '_ocr_lines.json',
                    os.path.join(os.path.dirname(path_image), 'bad'))
        shutil.copy(path_image + '_ocr_lines.png',
                    os.path.join(os.path.dirname(path_image), 'bad'))
        img = Image.open(path_symbol_img)
        img.load()
        bad_line_id = 1
        for ocr_line_gt in ocr_lines_gt:
            left_min, top_min = float('inf'), float('inf')
            right_max, bottom_max = 0, 0
            if not ocr_line_gt['recognized']:
                base_line_img = '%s_%s.png' % (os.path.basename(path_image),
                                               bad_line_id)
                path_line_img = os.path.join(dir_bad_line, base_line_img)
                print 'Crop bad line to', path_line_img, '...'
                for ch in ocr_line_gt['chars']:
                    left_min = min(left_min, ch['bounding_box'][0])
                    top_min = min(top_min, ch['bounding_box'][1])
                    right_max = max(right_max, ch['bounding_box'][2])
                    bottom_max = max(bottom_max, ch['bounding_box'][3])
                img.crop([left_min - 30, top_min - 30,
                          right_max + 30, bottom_max + 30]).save(path_line_img)
                bad_line_id += 1
    else:
        shutil.copy(path_image + '_ocr_lines.json',
                    os.path.join(os.path.dirname(path_image), 'good'))
        shutil.copy(path_image + '_ocr_lines.png',
                    os.path.join(os.path.dirname(path_image), 'good'))

    num_lines += len(ocr_lines_gt)
    num_correct_lines += each_num_correct_lines[id_image]

    if each_num_lines[id_image] != 0:
        each_accuracy[id_image] = float(
            each_num_correct_lines[id_image]) / each_num_lines[id_image]
    else:
        each_accuracy[id_image] = 0.0

    stats.append({'id_image': id_image,
                  'num_lines': each_num_lines[id_image],
                  'num_correct_lines': each_num_correct_lines[id_image],
                  'num_wrong_lines':
                  each_num_lines[id_image] -
                      each_num_correct_lines[id_image],
                  'accuracy': each_accuracy[id_image]})

stats.sort(key=lambda x: x['num_wrong_lines'])

f_stats_txt = open(base_stats + '.txt', 'w')

pretty_print_stats(stats)
pretty_print_stats(stats, f=f_stats_txt)
if base_stats:
    json.dump(stats, open(base_stats, 'w'), indent=2)

accuracy = float(num_correct_lines) / num_lines

if line_type is not None:
    print "Line type:", line_type
    print >> f_stats_txt, "Line type:", line_type
print 'Accuracy: %.3f (%d/%d)' % (accuracy, num_correct_lines, num_lines)
print >> f_stats_txt, 'Accuracy: %.3f (%d/%d)' % (
    accuracy, num_correct_lines, num_lines)
