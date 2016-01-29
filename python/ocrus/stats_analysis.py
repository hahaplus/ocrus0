# coding=utf-8

'''
Analysis and statistics for results

Copyright (C) 2015 Works Applications, all rights reserved

@author: Chang Sun
'''

import sys

OVERLAP_PERCENT = 0.1


def bbox_area(bbox):
    '''
    Return the area of the bounding box

    @param bbox: A bounding ox
    @return: Calcuated area
    '''

    '''
    x1,y1
    -------------
    |           |
    |           |
    |           |
    ------------x2,y2
    '''
    x1, y1, x2, y2 = bbox[0], bbox[1], bbox[2], bbox[3]
    return (x2 - x1) * (y2 - y1)


def bbox_almost_overlap(bbox1, bbox2, percent):
    '''
    Return True is two bounding box almost overlap, otherwise False
    Almost overlap means the overlapping area / (bbox1 + bbox2) > percent

    @param bbox1: Bounding box 1
    @param bbox2: Bounding box 2
    @return: A bool
    '''

    '''
    x1,y1
    -------------
    |  x3,y3    |
    |    -------------------
    |    |      |          |
    ------------x2,y2      |
         |                 |
         -------------------x4,y4
    '''
    x1, y1, x2, y2 = bbox1[0], bbox1[1], bbox1[2], bbox1[3]
    x3, y3, x4, y4 = bbox2[0], bbox2[1], bbox2[2], bbox2[3]
    bbox_overlap = max(x1, x3), max(y1, y3), min(x2, x4), min(y2, y4)
    return float(bbox_area(bbox_overlap)) / (bbox_area(bbox1) +
                                             bbox_area(bbox2)) > percent


def calc_correct_recognized_lines(ocr_lines_gt, ocr_lines):
    '''
    Compare ocr_lines_gt and ocr_lines,
        and set recognized in ocr_lines_gt, set correct in ocr_lines

    @param ocr_lines_gt: Ground truth OCR lines
    @param ocr_lines: OCR lines
    @return: (number_recognized, number_correct)
    '''
    num_recognized = calc_ok_in_lines1(ocr_lines_gt, ocr_lines)
    num_correct = calc_ok_in_lines1(ocr_lines, ocr_lines_gt)

    return num_recognized, num_correct


def calc_ok_in_lines1(ocr_lines1, ocr_lines2):
    '''
    Set ok in ocr_lines1 by giving ocr_lines2,
        if ocr_line1 is ok in ocr_lines2, then ocr_line1['ok'] is True

    @param ocr_lines1: OCR lines
    @param ocr_lines2: OCR lines
    @return: num_ok
    '''
    num_ok = 0
    for ocr_line1 in ocr_lines1:

        line_ok = ok_in_ocr_lines2(ocr_line1, ocr_lines2)

        if line_ok:
            num_ok += 1
            ocr_line1['ok'] = True
        else:
            ocr_line1['ok'] = False
    return num_ok


def ok_in_ocr_lines2(ocr_line1, ocr_lines2):
    '''
    Return True if ocr_line1 exists in ocr_lines2, otherwise Fase

    @param ocr_line1: OCR line
    @param ocr_lines2: OCR lines
    @return: A bool value
    '''
    line_ok = False

    for ocr_line2 in ocr_lines2:
        if ocr_line2['type'] != ocr_line1['type'] or \
                len(ocr_line2['chars']) != len(ocr_line1['chars']):
            continue

        texts = [ch['text'] for ch in ocr_line2['chars']]
        texts_gt = [ch['text'] for ch in ocr_line1['chars']]
        if texts != texts_gt:
            continue

        box_ok = True
        for pos in range(len(ocr_line2['chars'])):
            box = ocr_line2['chars'][pos]['bounding_box']
            box1 = ocr_line1['chars'][pos]['bounding_box']
            if not bbox_almost_overlap(box, box1, OVERLAP_PERCENT):
                box_ok = False
                break

        if box_ok:
            line_ok = True
            break

    return line_ok


def pretty_print_stats(stats, f=sys.stdout):
    '''
    Print stats to stdout in pretty format

    @param stats: The stats of OCR results
    '''

    print >> f, '%30s %5s %5s %8s %5s' % ('Img', 'C', 'T', 'Acc', 'W')
    for stat in stats:
        print >> f, '%30s %5d %5d %8.3f %5d' % (stat['id_image'],
                                                stat['num_correct_lines'],
                                                stat['num_lines'],
                                                stat['accuracy'],
                                                stat['num_wrong_lines'])
    print >> f, 'Img: Image, C: Correct, T: Total, Acc: Accuracy, W: Wrong'


def pretty_print_stats_cmp(stats_cmp):
    '''
    Print comparing stats to stdout in pretty format

    @param stats_cmp: The comparing stats of OCR results
    '''

    print '%30s %3s %3s %3s %6s %6s %3s %3s %3s' % \
        ('Img', 'C1', 'C2', 'T', 'Acc1', 'Acc2', 'W1', 'W2', 'C_D')
    for stat_cmp in stats_cmp:
        print '%30s %3s %3s %3s %6.3f %6.3f %3s %3s %3s' % \
            (stat_cmp['id_image'],
             stat_cmp['stat1']['num_correct_lines'],
             stat_cmp['stat2']['num_correct_lines'],
             stat_cmp['stat1']['num_lines'],
             stat_cmp['stat1']['accuracy'],
             stat_cmp['stat2']['accuracy'],
             stat_cmp['stat1']['num_wrong_lines'],
             stat_cmp['stat2']['num_wrong_lines'],
             stat_cmp['correct_delta'])
    print '''Img: Image, C: Correct1, T: Total,
Acc: Accuracy, W: Wrong, C_D: Correct Delta'''

if __name__ == '__main__':
    pass
