#!/usr/bin/env python

'''
Provide for dealing with all kinds of OCR result formats

Copyright (C) 2015 Works Applications, all rights reserved

Created on Dec 6, 2015

@author: Chang Sun
'''

from distutils.core import setup

setup(name='OCRus',
      version='1.0',
      description='OCRus OCR CORE',
      packages=['ocrus', 'ocrus.api',
                'ocrus.data_structure',
                'ocrus.postprocessing',
                'ocrus.preprocessing',
                'ocrus.recognition',
                'ocrus.recognition.neural_network',
                'ocrus.segmentation',
                'ocrus.statistic',
                'ocrus.util', ],

      scripts=['ocrus/programs/draw/ocrus_draw_bbox.py',
               'ocrus/programs/draw/ocrus_draw_all.py',
               'ocrus/programs/draw/ocrus_draw_ocr_lines_all.py',
               'ocrus/programs/draw/ocrus_draw_gt_all.py',
               'ocrus/programs/convert/ocrus_to_ocr_lines.py',
               'ocrus/programs/convert/ocrus_to_gt.py',
               'ocrus/programs/stats/ocrus_calc_accuracy.py',
               'ocrus/programs/stats/ocrus_cmp_stats.py',
               'ocrus/programs/stats/ocrus_show_accuracy.py',
               'ocrus/programs/stats/ocrus_running_time.py', ]
      )
