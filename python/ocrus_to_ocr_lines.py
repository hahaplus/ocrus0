#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import codecs
import json
import io

from ocrus.ocr_result import parse_line_v1
from ocrus.ocr_result import to_ocr_lines


if len(sys.argv) != 3:
    print '''Save the money and date lines in json format

Usage: %s symbol|word path_image_list
path_image_list
    A list of path of images, one path in one line''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

level = sys.argv[1]
path_image_list = sys.argv[2].rstrip('/')

for path_image in open(path_image_list):
    path_image = path_image.strip()
    if path_image == '':
        continue
    print 'Processing', path_image, '...'

    path_txt = '{path_image}_{level}.txt'.\
        format(path_image=path_image, level=level)
    path_ocr_chars = '{path_image}_ocr_chars.json'.\
        format(path_image=path_image)
    path_ocr_lines = '{path_image}_ocr_lines.json'.\
        format(path_image=path_image)

    ocr_chars = []
    for line in codecs.open(path_txt, encoding='utf-8'):
        d = parse_line_v1(line)
        ocr_chars.append(d)

    ocr_lines = to_ocr_lines(ocr_chars)

    with io.open(path_ocr_chars, 'w', encoding='utf-8') as json_file:
        data = json.dumps(ocr_chars, ensure_ascii=False, indent=2)
        json_file.write(unicode(data))

    with io.open(path_ocr_lines, 'w', encoding='utf-8') as json_file:
        data = json.dumps(ocr_lines, ensure_ascii=False, indent=2)
        json_file.write(unicode(data))
