# coding=utf-8
'''
For drawing OCR results

Copyright (C) 2015 Works Applications, all rights reserved

Created on Dec 4, 2015

@author: Chang Sun
'''

from PIL import Image, ImageDraw, ImageFont
import os
import json
import sys

FONT_PATH = '/usr/share/fonts/truetype/fonts-japanese-gothic.ttf'
TEXT_COLOR = (255, 0, 0)
CONFIDENCE_COLOR = (0, 128, 0)
BOX_COLOR = (0, 0, 255)

font_text = ImageFont.truetype(FONT_PATH, 24, encoding='utf-8')
font_confidence = ImageFont.truetype(FONT_PATH, 12, encoding='utf-8')


def draw_ocr_lines(path_ocr_lines, path_img, path_output_img):
    '''
    Draw text, bounding_box and confidence to image

    @param path_ocr_lines: OCR result to draw
    @param path_image: Path of the image to draw on
    @param path_output_img: Image with the drawing on
    '''
    ext_output_img = os.path.splitext(path_output_img)[1][1:]

    img = Image.open(path_img)
    img.load()
    draw = ImageDraw.Draw(img)

    ocr_lines = json.load(open(path_ocr_lines))
    for ocr_line in ocr_lines:
        for ch in ocr_line['chars']:
            text = ch['text']
            box = ch['bounding_box']
            confidence = ch['confidence']

            '''
            x1(left), top, right, bottom

            x1(left), y1(top)    x2, y1
                    --------------
                    |            |
                    --------------
                x1, y2    x2(right), y2(bottom)
            '''
            x1, y1, x2, y2 = box[0], box[1], box[2], box[3]

            draw.line((x1, y1, x2, y1), BOX_COLOR, 1)
            draw.line((x2, y1, x2, y2), BOX_COLOR, 1)
            draw.line((x2, y2, x1, y2), BOX_COLOR, 1)
            draw.line((x1, y2, x1, y1), BOX_COLOR, 1)

            draw.text(
                ((x1 + x2) / 2 - 5, y2 - 5), text, TEXT_COLOR, font=font_text)
            if confidence:
                draw.text(((x1 + x2) / 2 - 5, y1 - 15), '%.1f' %
                          confidence, CONFIDENCE_COLOR, font=font_confidence)

    img.save(path_output_img, ext_output_img)


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print '''Usage: %s path_ocr_lines path_img path_output_img ''' % \
            os.path.basename(sys.argv[0])
        sys.exit(0)
    draw_ocr_lines(sys.argv[1], sys.argv[2], sys.argv[3])
    pass
