#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import os
import sys

from ocrus.statistic.stats_analysis import pretty_print_stats


if len(sys.argv) not in [2, 3]:
    print '''Show accuracy for images.

Usage: %s path_stats_json

path_stats_json
    From which the stats are loaded''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)


path_stats = sys.argv[1]

stats = json.load(open(path_stats))

num_correct_lines = sum([stat['num_correct_lines'] for stat in stats])
num_lines = sum([stat['num_lines'] for stat in stats])

stats.sort(key=lambda x: x['num_wrong_lines'])
pretty_print_stats(stats)

accuracy = float(num_correct_lines) / num_lines
print 'Accuracy: %.3f (%d/%d)' % (accuracy, num_correct_lines, num_lines)
