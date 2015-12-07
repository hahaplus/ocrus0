#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys

PAGE_SEG_MODES = ('PSM_AUTO_ONLY', 'PSM_AUTO',
                  'PSM_SINGLE_COLUMN', 'PSM_SINGLE_BLOCK')

ENUM_DEF = '''
Extracted from tesseract/ccstruct/publictypes.h
enum PageSegMode {
  PSM_OSD_ONLY,       ///< Orientation and script detection only.
  PSM_AUTO_OSD,       ///< Automatic page segmentation with orientation and
                      ///< script detection. (OSD)
  PSM_AUTO_ONLY,      ///< Automatic page segmentation, but no OSD, or OCR.
  PSM_AUTO,           ///< Fully automatic page segmentation, but no OSD.
  PSM_SINGLE_COLUMN,  ///< Assume a single column of text of variable sizes.
  PSM_SINGLE_BLOCK_VERT_TEXT,  ///< Assume a single uniform block of vertically
                               ///< aligned text.
  PSM_SINGLE_BLOCK,   ///< Assume a single uniform block of text. (Default.)
  PSM_SINGLE_LINE,    ///< Treat the image as a single text line.
  PSM_SINGLE_WORD,    ///< Treat the image as a single word.
  PSM_CIRCLE_WORD,    ///< Treat the image as a single word in a circle.
  PSM_SINGLE_CHAR,    ///< Treat the image as a single character.
  PSM_SPARSE_TEXT,    ///< Find as much text as possible in no particular order.
  PSM_SPARSE_TEXT_OSD,  ///< Sparse text with orientation and script det.
  PSM_RAW_LINE,       ///< Treat the image as a single text line, bypassing
                      ///< hacks that are Tesseract-specific.

  PSM_COUNT           ///< Number of enum entries.
};
'''

enums = {}
value = 0
for line in map(str.strip, ENUM_DEF.splitlines()):
    if line.startswith('PSM_'):
        enums[line.split(',')[0]] = value
        value += 1

prog_draw_all = 'ocrus_draw_all.py'

if len(sys.argv) != 4:
    print '''For all possible good page_seg_mode,
    process all the Photo-0000 to Photo-0027 images.

Usage: %s text|draw|both symbol|word|both dir_photos
mode
    text: output bbox result to text file
    draw: draw the bounding box, recognized result and confidence
    both: both text and draw
level
    symbol: symbol only
    word: word only
    both: both symbol and word
dir_photos
    Where the Photo-0000.jpg to Photo-0027 are stored''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

mode = sys.argv[1]
level = sys.argv[2]
dir_photos = sys.argv[3].rstrip('/')

for psm_str in PAGE_SEG_MODES:
    page_seg_mode = enums[psm_str]
    cmd = '''ocrus_draw_all.py {mode} {page_seg_mode} \
    {level} {dir_photos}'''.format(mode=mode,
                                   page_seg_mode=page_seg_mode,
                                   level=level,
                                   dir_photos=dir_photos)

    dir_name = '%d-%s' % (page_seg_mode, psm_str)
    try:
        os.mkdir(dir_name)
    except Exception:
        pass

    print 'cd %s' % dir_name
    os.chdir(dir_name)

    print 'Run cmd: %s' % cmd
    os.system(cmd)

    print 'cd ..'
    os.chdir('..')
