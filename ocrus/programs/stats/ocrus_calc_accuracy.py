#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json
import shutil
from PIL import Image

from ocrus.statistic.stats_analysis import pretty_print_stats, calc_ok_in_lines1

TAG_RECALL = 'recall'
TAG_PRECISION = 'precision'
TAGS = [TAG_RECALL, TAG_PRECISION]

SUFFIX_BAD_LINE = 'bad_lines'
SUFFIX_BAD = 'bad'
SUFFIX_GOOD = 'good'
SUFFIXES = [SUFFIX_BAD_LINE, SUFFIX_BAD, SUFFIX_GOOD]

folder_config = {}
for tag in TAGS:
    folder_config[tag] = {}
    for suffix in SUFFIXES:
        folder_config[tag][suffix] = tag + '_' + suffix

folders = [folder_config[tag][suffix]
           for tag in TAGS
           for suffix in SUFFIXES]


if len(sys.argv) not in [2, 3, 4]:
    print '''Calculate recall/precision for images.

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

stats_recall = []
stats_precision = []


dirs_to_create = set()
for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue
    for folder in folders:
        dirs_to_create.add(os.path.join(os.path.dirname(path_image), folder))

for dir_ in dirs_to_create:
    if os.path.exists(dir_):
        shutil.rmtree(dir_)
    os.mkdir(dir_)


def calc_crop_single(id_image, ocr_lines1, ocr_lines2, stats, tag):
    num_lines = len(ocr_lines1)
    num_ok = calc_ok_in_lines1(ocr_lines1, ocr_lines2)

    path_symbol_img = path_image + '_symbol.png'
    dir_bad_line = os.path.join(os.path.dirname(path_image),
                                folder_config[tag][SUFFIX_BAD_LINE])

    if not all([ocr_line1['ok'] for ocr_line1 in ocr_lines1]):
        shutil.copy(
            path_image + '_ocr_lines.json',
            os.path.join(os.path.dirname(path_image),
                         folder_config[tag][SUFFIX_BAD]))
        shutil.copy(
            path_image + '_ocr_lines.png',
            os.path.join(os.path.dirname(path_image),
                         folder_config[tag][SUFFIX_BAD]))
        img = Image.open(path_symbol_img)
        img.load()
        bad_line_id = 1
        for ocr_line1 in ocr_lines1:
            left_min, top_min = float('inf'), float('inf')
            right_max, bottom_max = 0, 0
            if not ocr_line1['ok']:
                base_line_img = '%s_%s.png' % (os.path.basename(path_image),
                                               bad_line_id)
                path_line_img = os.path.join(dir_bad_line, base_line_img)
                print 'Crop bad line to', path_line_img, '...'
                for ch in ocr_line1['chars']:
                    left_min = min(left_min, ch['bounding_box'][0])
                    top_min = min(top_min, ch['bounding_box'][1])
                    right_max = max(right_max, ch['bounding_box'][2])
                    bottom_max = max(bottom_max, ch['bounding_box'][3])
                img.crop([left_min - 30, top_min - 30,
                          right_max + 30, bottom_max + 30]).save(path_line_img)
                bad_line_id += 1
    else:
        shutil.copy(path_image + '_ocr_lines.json',
                    os.path.join(os.path.dirname(path_image),
                                 folder_config[tag][SUFFIX_GOOD]))
        shutil.copy(path_image + '_ocr_lines.png',
                    os.path.join(os.path.dirname(path_image),
                                 folder_config[tag][SUFFIX_GOOD]))

    accuracy = float(num_ok) / num_lines if num_lines != 0 else 0.0

    stats.append({'id_image': id_image,
                  'num_lines': num_lines,
                  'num_correct_lines': num_ok,
                  'num_wrong_lines': num_lines - num_ok,
                  'accuracy': accuracy})


def dump_print_stats(stats, base_stats, tag):
    stats.sort(key=lambda x: x['num_wrong_lines'])

    f_stats_txt = open(base_stats + '_' + tag + '.txt', 'w')

    pretty_print_stats(stats)
    pretty_print_stats(stats, f=f_stats_txt)
    if base_stats:
        json.dump(stats, open(base_stats, 'w'), indent=2)

    num_lines = sum([stat['num_lines'] for stat in stats])
    num_correct_lines = sum([stat['num_correct_lines'] for stat in stats])

    accuracy = float(num_correct_lines) / num_lines

    if line_type is not None:
        print "Line type:", line_type
        print >> f_stats_txt, "Line type:", line_type
    print '%s: %.3f (%d/%d)' % (tag, accuracy, num_correct_lines, num_lines)
    print >> f_stats_txt, '%s: %.3f (%d/%d)' % (
        tag, accuracy, num_correct_lines, num_lines)


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

    calc_crop_single(id_image, ocr_lines_gt, ocr_lines, stats_recall,
                     TAG_RECALL)
    calc_crop_single(id_image, ocr_lines, ocr_lines_gt, stats_precision,
                     TAG_PRECISION)


dump_print_stats(stats_recall, base_stats, TAG_RECALL)
dump_print_stats(stats_precision, base_stats, TAG_PRECISION)
