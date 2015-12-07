#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import codecs
import json
import io

from ocrus.ocr_result import parse_line_v1
from ocrus.ocr_result import extract_date, extract_money
from ocrus.ocr_result import replace_if_exist
from ocrus.ocr_result import REPLACE_TABLE


if len(sys.argv) != 3:
    print '''Process all the Photo-0000 to Photo-0027 images.
Save the money and date lines in json format

Usage: %s symbol|word dir_txt
dir_txt
    Where the Photo-0000_symbol|word.txt
        to Photo-0027_symbol|word.txt are stored''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

level = sys.argv[1]
dir_txt = sys.argv[2].rstrip('/')

num_photos = range(0, 28)
num_photos.remove(2)

num_photos = map(lambda x: '%04d' % x, num_photos)

for num_photo in num_photos:
    print 'Processing photo', num_photo, '...'

    path_txt = '{dir_txt}/Photo-{num_photo}_{level}.txt'.format(
        dir_txt=dir_txt, num_photo=num_photo, level=level)
    path_ocr_chars = '{dir_txt}/Photo-{num_photo}_{level}_ocr_chars.json'.\
        format(dir_txt=dir_txt, num_photo=num_photo, level=level)
    path_ocr_lines = '{dir_txt}/Photo-{num_photo}_{level}_ocr_lines.json'.\
        format(dir_txt=dir_txt, num_photo=num_photo, level=level)

    ocr_chars = []
    for line in codecs.open(path_txt, encoding='utf-8'):
        d = parse_line_v1(line)
        ocr_chars.append(d)

    # Make sure each ch['text'] contain 1 character
    for ch in ocr_chars:
        if len(ch['text']) == 0:
            ch['text'] = ' '
        elif len(ch['text']) > 1:
            ch['text'] = ch['text'][0]

    text = u''.join(ocr_char['text'] for ocr_char in ocr_chars)

    ocr_lines = []

    pos_dates = extract_date(text)
    for start_pos, end_pos in pos_dates:
        ocr_line = {'type': 'date',
                    'chars': []}
        for pos in range(start_pos, end_pos):
            ocr_line['chars'].append(ocr_chars[pos])
        ocr_lines.append(ocr_line)

    pos_moneys = extract_money(text)
    for start_pos, end_pos in pos_moneys:
        ocr_line = {'type': 'money',
                    'chars': []}
        for pos in range(start_pos, end_pos):
            ocr_line['chars'].append(ocr_chars[pos])
        ocr_lines.append(ocr_line)

    # Make replacements
    for ocr_line in ocr_lines:
        for ch in ocr_line['chars']:
            ch['text'] = replace_if_exist(ch['text'], REPLACE_TABLE)

    # Remove whitespace characters
    for ocr_line in ocr_lines:
        ocr_line['chars'] = [
            ch for ch in ocr_line['chars'] if ch['text'].strip() != '']

    with io.open(path_ocr_chars, 'w', encoding='utf-8') as json_file:
        data = json.dumps(ocr_chars, ensure_ascii=False, indent=2)
        json_file.write(unicode(data))

    with io.open(path_ocr_lines, 'w', encoding='utf-8') as json_file:
        data = json.dumps(ocr_lines, ensure_ascii=False, indent=2)
        json_file.write(unicode(data))
