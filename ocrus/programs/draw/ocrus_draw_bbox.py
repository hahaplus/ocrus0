#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import codecs
import os
from PIL import Image, ImageDraw, ImageFont
from ocrus.postprocessing.ocr_result import parse_line_v1
from ocrus.util.ocr_drawing import FONT_PATH


WORD_COLOR = (255, 0, 0)
CONFIDENCE_COLOR = (0, 128, 0)
BOX_COLOR = (0, 0, 255)

if len(sys.argv) != 4:
    print '''Draw the bounding box result to image

Usage: %s path_img path_txt path_output_img

path_output_img should end with '.png'
Example line of path_txt:
    word: 'g';      conf: 61.91; bounding_box: 411,1778,431,1906;''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

path_img = sys.argv[1]
path_txt = sys.argv[2]
path_output_img = sys.argv[3]
ext_output_img = os.path.splitext(path_output_img)[1][1:]
font_word = ImageFont.truetype(FONT_PATH, 24, encoding='utf-8')
font_confidence = ImageFont.truetype(FONT_PATH, 12, encoding='utf-8')

img = Image.open(path_img)
img.load()
draw = ImageDraw.Draw(img)

for line in codecs.open(path_txt, encoding='utf-8'):
    d = parse_line_v1(line)

    if all(v is not None for v in d.values()):
        '''
        x1(left), top, right, bottom

        x1(left), y1(top)    x2, y1
                --------------
                |            |
                --------------
            x1, y2    x2(right), y2(bottom)
        '''
        word, box, confidence = d['text'], d['bounding_box'], d['confidence']
        x1, y1, x2, y2 = box[0], box[1], box[2], box[3]

        draw.line((x1, y1, x2, y1), BOX_COLOR, 1)
        draw.line((x2, y1, x2, y2), BOX_COLOR, 1)
        draw.line((x2, y2, x1, y2), BOX_COLOR, 1)
        draw.line((x1, y2, x1, y1), BOX_COLOR, 1)

        draw.text(((x1 + x2) / 2 - 5, y2 - 5),
                  word, WORD_COLOR, font=font_word)
        draw.text(((x1 + x2) / 2 - 5, y1 - 15), '%.0f' % confidence,
                  CONFIDENCE_COLOR, font=font_confidence)

img.save(path_output_img)
