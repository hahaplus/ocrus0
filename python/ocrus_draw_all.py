#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

LEVELS = ['symbol', 'word']

prog_bounding_box = 'ocrus_bounding_box'
prog_draw_bbox = 'ocrus_draw_bbox.py'

if len(sys.argv) != 3:
    print '''Process all the Photo-0000 to Photo-0027 images.
Output bounding box result and draw the result to image

Usage: %s text|draw|both dir_photos
mode
    text: output bbox result to text file
    draw: draw the bounding box, recognized result and confidence
    both: both text and draw
dir_photos
    Where the Photo-0000.jpg to Photo-0027 are stored''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

mode = sys.argv[1]
dir_photos = sys.argv[2].rstrip('/')

num_photos = range(0, 28)
num_photos.remove(2)

num_photos = map(lambda x: '%04d' % x, num_photos)

for num_photo in num_photos:
    for level in LEVELS:
        if mode in ['text', 'both']:
            cmd = '''{prog_bounding_box} {level} {dir_photos}/Photo-{num_photo}.jpg \
> Photo-{num_photo}_{level}.txt
'''.format(prog_bounding_box=prog_bounding_box, level=level,
                dir_photos=dir_photos, num_photo=num_photo)
            print 'Running cmd:', cmd
            os.system(cmd)

        if mode in ['draw', 'both']:
            cmd = '''{prog_draw_bbox} {dir_photos}/Photo-{num_photo}.jpg_binarize.png \
Photo-{num_photo}_{level}.txt Photo-{num_photo}_{level}.png
'''.format(prog_draw_bbox=prog_draw_bbox, level=level,
                dir_photos=dir_photos, num_photo=num_photo)
            print 'Running cmd:', cmd
            os.system(cmd)
