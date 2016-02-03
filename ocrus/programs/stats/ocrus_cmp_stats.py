#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json
from ocrus.statistic.stats_analysis import pretty_print_stats_cmp

if len(sys.argv) != 3:
    print '''Compare stats and print comparing results

Usage: %s path_stats1 path_stats2''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

path_stats1 = sys.argv[1].rstrip('/')
path_stats2 = sys.argv[2].rstrip('/')

stats1 = json.load(open(path_stats1))
stats2 = json.load(open(path_stats2))

stats1_dict = {stat['id_image']: stat for stat in stats1}
stats2_dict = {stat['id_image']: stat for stat in stats2}

stats_cmp = [{'id_image': id_image,
              'stat1': stats1_dict[id_image],
              'stat2': stats2_dict[id_image],
              'correct_delta': stats2_dict[id_image]['num_correct_lines'] -
              stats1_dict[id_image]['num_correct_lines']}
             for id_image in stats1_dict]

stats_cmp.sort(key=lambda x: x['correct_delta'])
pretty_print_stats_cmp(stats_cmp)
