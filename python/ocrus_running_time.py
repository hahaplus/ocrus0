#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import json


if len(sys.argv) not in [2]:
    print '''Calculate accuracy for images.

Usage: %s path_running_time

path_running_time
    Path of the result of running time''' % \
        (os.path.basename(sys.argv[0]))
    sys.exit(0)

time_stats = []

for line in open(sys.argv[1]):
    if line.startswith('{"cmd":'):
        print line
        time_stats.append(json.loads(line))

print 'Running time'
time_stats.sort(key=lambda x: x['time_spent'])
time_all = sum([time_stat['time_spent'] for time_stat in time_stats])
for time_stat in time_stats:
    print "%20s %5.2f sec" % (
        os.path.basename(time_stat['cmd'].split()[3]), time_stat['time_spent'])
print 'Total: %.2f sec, Average: %.2f sec' % (
    time_all, time_all / len(time_stats))
