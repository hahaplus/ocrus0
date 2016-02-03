'''
Extract character rows on image

Copyright (C) 2016 Works Applications, all rights reserved
'''

import cv2
import numpy as np

from ocrus.util.convert_format import to_chars
from ocrus.util.geometric import rect2_in_rect1
from ocrus.util.geometric import rect_after_merge
from ocrus.util.geometric import rects_near, rects_overlap_in_x
from ocrus.util.geometric import overlap_ratio_in_second
from ocrus.util.img_proc import get_img_roi
from ocrus.util.mnist_format import remove_white_to_mnist_array

W_AT_LEAST_GOOD = 5
H_AT_LEAST_GOOD = 5

ALPHA = 0.2
OVERLAP_AT_LEAST = 0.3
OVERLAP_PREV2_AT_LEAST = 0.8

K_TOP_SAVE = 50

PROB_CHAR = 0.95

# Relax and get more bad chars, when update_bbox_type
RATIO_RELAX_SMALL_W = 0.8  # <= 1.0
RATIO_RELAX_SMALL_H = 1.0  # <= 1.0
RATIO_RELAX_BIG_W = 1.2  # >= 1.0
RATIO_RELAX_BIG_H = 1.2  # >= 1.0

# Get more key chars when add_bbox_to_row
RATIO_TO_SMALL = 0.8  # <= 1.0
RATIO_TO_BIG = 1.2  # >= 1.0

# Two rects are near if pixels between them are <= this value
PIXEL_NEAR = -2

# Two rects are overlap if overlap ratio is > this value
RATIO_OVERLAP_X = 0.3

COLOR_CONTOUR = (180, 180, 180)
color_config = {
    'good': (0, 200, 0),
    'bad': (0, 200, 200),
    'small': (255, 255, 255),
    'big': (0, 0, 255)
}
COLOR_LINE = (200, 0, 200)
COLOR_CIRCLE = (255, 0, 255)

simple_char_line = u'''
0
/ - ,  ， 。 ' "
l o
・ ー
一
'''
simple_chars = to_chars([simple_char_line])


def delete_inner_bbox(bboxes):
    '''
    Delete small, good or bad that are inside good or bad
    Note: the bboxes is not changed

    @param bboxes: Bounding boxes
    @return: bboxes
    '''

    for bbox in bboxes:
        bbox['delete'] = False

    bboxes_uniq = []
    rect_set = set()
    for bbox in bboxes:
        if bbox['rect'] not in rect_set:
            rect_set.add(bbox['rect'])
            bboxes_uniq.append(bbox)
    bboxes = bboxes_uniq

    for i, bbox1 in enumerate(bboxes):
        for j, bbox2 in enumerate(bboxes):
            if i != j:
                rect1, rect2 = bbox1['rect'], bbox2['rect']
                if bbox2['type'] in ['small', 'good', 'bad'] and \
                        bbox1['type'] in ['good', 'bad'] and \
                        rect2_in_rect1(rect1, rect2):
                    bbox2['delete'] = True

    bboxes = [bbox for bbox in bboxes if not bbox['delete']]
    return bboxes


def update_bbox_type(bboxes):
    '''
    Update bbox type in bboxes after good bboxes are changed
    Note: bboxes will be changed

    @param bboxes: Bounding boxes
    '''

    for bbox in bboxes:
        if (bbox['ch'] not in simple_chars and
                bbox['prob'] > PROB_CHAR and
                bbox['rect'][2] >= W_AT_LEAST_GOOD and
                bbox['rect'][3] >= H_AT_LEAST_GOOD):
            bbox['type'] = 'good'

    good_rect_ws = [bbox['rect'][2] for bbox in bboxes
                    if bbox['type'] == 'good']
    good_rect_hs = [bbox['rect'][3] for bbox in bboxes
                    if bbox['type'] == 'good']
    max_rect_w, min_rect_w = max(good_rect_ws or [0]), min(good_rect_ws or [0])
    max_rect_h, min_rect_h = max(good_rect_hs or [0]), min(good_rect_hs or [0])

    for bbox in bboxes:
        _, _, rect_w, rect_h = bbox['rect']

        if bbox['type'] == 'good':
            pass
        elif (rect_w < min_rect_w * RATIO_RELAX_SMALL_W or
              rect_h < min_rect_h * RATIO_RELAX_SMALL_H):
            bbox['type'] = 'small'
        elif rect_w > max_rect_w * RATIO_RELAX_BIG_W or \
                rect_h > max_rect_h * RATIO_RELAX_BIG_H:
            bbox['type'] = 'big'
        else:
            bbox['type'] = 'bad'


def inside_row(top_of_row, bot_of_row, top, bot):
    '''
    Return True if (top, bot) is inside (top_of_row, bot_of_row)

    @param top_of_row: Top of row
    @param bot_of_row: Bottom of row
    @param top: Top
    @param bot: Bottom
    @return: A bool value
    '''
    return top_of_row <= top and bot <= bot_of_row


def backward_create_rows(bboxes):
    '''
    Backward create rows

    @param bboxes: Bounding boxes
    @return: rows
    '''

    prepare_add_to_row(bboxes)
    rows = []
    for i in range(len(bboxes) - 1, -1, -1):
        add_bbox_to_row(rows, bboxes, i)

    return rows


def forward_create_rows(bboxes):
    '''
    Forward create rows

    @param bboxes: Bounding boxes
    @return: rows
    '''
    prepare_add_to_row(bboxes)
    rows = []
    for i in range(len(bboxes)):
        add_bbox_to_row(rows, bboxes, i)

    return rows


def add_bbox_to_row(rows, bboxes, i):
    '''
    Add the i-th bbox to existing rows

    @param rows: Rows
    @param bboxes: Bounding boxes
    @param i: Index of bbox in bboxes
    '''
    bbox = bboxes[i]
    if bbox['type'] in ['good', 'bad', 'small'] and not bbox['in_row']:
        ratio_max, row_best = 0.0, None

        for row in rows:
            ratio = overlap_ratio_in_second(row['top'], row['bot'],
                                            bbox['top'], bbox['bot'])
            if ratio > OVERLAP_AT_LEAST and ratio > ratio_max:
                ratio_max, row_best = ratio, row

            if len(row['pos_bboxes']) >= 2:
                bbox_prev2 = bboxes[row['pos_bboxes'][-2]]
                ratio = overlap_ratio_in_second(
                    bbox_prev2['top_of_row'],
                    bbox_prev2['bot_of_row'],
                    bbox['top'],
                    bbox['bot'])
                if ratio > OVERLAP_PREV2_AT_LEAST and ratio > ratio_max:
                    ratio_max, row_best = ratio, row

        if bbox['type'] == 'good':
            bbox['key_bbox'] = True

        if row_best is not None and ratio_max > OVERLAP_AT_LEAST:
            row_h = row_best['bot'] - row_best['top']
            h_min = min(row_h, row_best['h_min']) * RATIO_TO_SMALL
            h_max = max(row_h, row_best['h_max']) * RATIO_TO_BIG

            row_best['pos_bboxes'].append(i)
            bbox['in_row'] = True

            bbox['top_of_row'] = row_best['top']
            bbox['bot_of_row'] = row_best['bot']

            if bbox['type'] == 'bad':
                if h_min <= bbox['h'] <= h_max:
                    bbox['key_bbox'] = True

            if bbox['key_bbox'] and bbox['type'] in ['good', 'bad']:
                row_best['top'] = ALPHA * row_best['top'] + \
                    (1 - ALPHA) * bbox['top']
                row_best['bot'] = ALPHA * row_best['bot'] + \
                    (1 - ALPHA) * bbox['bot']

                row_best['h_min'] = min(row_best['h_min'], bbox['rect'][3])
                row_best['h_max'] = max(row_best['h_max'], bbox['rect'][3])

        elif bbox['key_bbox']:
            new_row = {'top': bbox['top'], 'bot': bbox['bot'],
                       'pos_bboxes': [],
                       'h_min': float('inf'), 'h_max': 0}

            new_row['pos_bboxes'].append(i)
            bbox['in_row'] = True

            bbox['top_of_row'] = bbox['rect'][1]
            bbox['bot_of_row'] = bbox['rect'][1] + bbox['rect'][3]

            if bbox['type'] in ['good', 'bad']:
                new_row['h_min'] = min(new_row['h_min'], bbox['rect'][3])
                new_row['h_max'] = max(new_row['h_max'], bbox['rect'][3])

            rows.append(new_row)


def rm_too_small_from_row(rows, bboxes):
    '''
    Remove bounding boxes that are too small from rows

    @param rows: Rows
    @param bboxes: Bounding boxes
    @return: New rows
    '''
    for row in rows:
        row['w_max'], row['w_min'] = 0, float('inf')
        row['h_max'], row['h_min'] = 0, float('inf')
        for pos in row['pos_bboxes']:
            if ('key_bbox' in bboxes[pos] and bboxes[pos]['key_bbox'] and
                    bboxes[pos]['type'] in ['good', 'bad']):
                row['w_max'] = max(row['w_max'], bboxes[pos]['rect'][2])
                row['w_min'] = min(row['w_min'], bboxes[pos]['rect'][2])
                row['h_max'] = max(row['h_max'], bboxes[pos]['rect'][3])
                row['h_min'] = min(row['h_min'], bboxes[pos]['rect'][3])

        pos_bboxes = []
        w_min = row['w_min'] * RATIO_TO_SMALL
        h_min = row['h_min'] * RATIO_TO_SMALL
        for i, pos in enumerate(row['pos_bboxes']):
            if bboxes[pos]['type'] in ['good', 'bad']:
                pos_bboxes.append(pos)
            elif bboxes[pos]['type'] == 'small':
                if bboxes[pos]['rect'][3] > h_min:
                    pos_bboxes.append(pos)
                elif bboxes[pos]['rect'][2] > w_min:
                    pos_list = row['pos_bboxes'][i: i + 2]
                    bbox_top = bboxes[pos]['rect'][1]
                    bbox_bot = bboxes[pos]['rect'][1] + bboxes[pos]['rect'][3]
                    for p in pos_list:
                        row_y_mid = (bboxes[p]['top_of_row'] +
                                     bboxes[p]['bot_of_row']) / 2
                        row_h = (bboxes[p]['bot_of_row'] -
                                 bboxes[p]['top_of_row'])
                        if (bbox_top < row_y_mid + row_h / 4 and
                            bbox_bot > row_y_mid - row_h / 4 and
                                overlap_ratio_in_second(
                                    bboxes[p]['top_of_row'],
                                    bboxes[p]['bot_of_row'],
                                    bbox_top, bbox_bot) > 0.99):
                            pos_bboxes.append(pos)
                            break

        row['pos_bboxes'] = pos_bboxes

    new_rows = []
    for row in rows:
        if len(row['pos_bboxes']) >= 1:
            new_rows.append(row)
    return new_rows


def recognize_bboxes(img_binary, bboxes, net, id_to_char):
    '''
    Recognize bounding boxes if not recognized

    @param img_binary: Binary image to get img roi from
    @param bboxes: Bounding boxes
    @param net: A network3 object
    @param net: A dict map from id to char
    '''
    pos_result_to_bboxes = {}
    float_imgs = []
    for bbox in bboxes:
        if 'float_img' not in bbox:
            img_char = get_img_roi(img_binary, bbox['rect'])
            float_img = remove_white_to_mnist_array(img_char)
            bbox['float_img'] = float_img
            bbox['type'] = None

    for i, bbox in enumerate(bboxes):
        if 'ch' not in bbox:
            float_imgs.append(bbox['float_img'])
            pos_result_to_bboxes[len(float_imgs) - 1] = i

    if len(float_imgs) > 0:
        result = net.predict_xs(np.asarray(float_imgs))
        for i, y_out in enumerate(result[0]):
            ch = id_to_char[y_out]
            prob = result[1][i][y_out]
            j = pos_result_to_bboxes[i]
            bboxes[j]['ch'], bboxes[j]['prob'] = ch, prob

            prob_y = result[1][i]
            top_y_out = prob_y.argsort()[-K_TOP_SAVE:][::-1]
            bboxes[j]['top_k'] = []
            for y_o in top_y_out:
                bboxes[j]['top_k'].append((id_to_char[y_o], prob_y[y_o]))


def prepare_add_to_row(bboxes):
    '''
    Prepare information and sort bboxes by left before add to row
    left, top, bot, h, in_row, key_bbox of each bbox are set

    @param bboxes: Bounding boxes
    '''
    for bbox in bboxes:
        bbox['left'] = bbox['rect'][0]
        bbox['top'] = bbox['rect'][1]
        bbox['bot'] = bbox['rect'][1] + bbox['rect'][3]
        bbox['h'] = bbox['rect'][3]
        bbox['in_row'] = False
        if 'key_bbox' not in bbox:
            bbox['key_bbox'] = (bbox['type'] == 'good')
    bboxes.sort(key=lambda b: b['left'])


def merge_absolute_in_row(rows, bboxes):
    '''
    Merge the bounding boxes that are absolutely should be merged

    @param rows: Rows
    @param bboxes: Bounding boxes
    @return: New bboxes
    '''

    # For each row, merge near good, bad, small chars
    new_bboxes = []
    for row in rows:
        bundles = [{'rect': bboxes[pos]['rect'], 'pos_bboxes': set([pos, ])}
                   for pos in row['pos_bboxes']
                   if bboxes[pos]['type'] in ['good', 'bad', 'small']]
        pos1, pos2 = 0, 1
        while pos1 < len(bundles) and pos2 < len(bundles):
            if pos1 == pos2:
                print 'Error: pos1 == pos2'
            if (rects_overlap_in_x(bundles[pos1]['rect'],
                                   bundles[pos2]['rect'], RATIO_OVERLAP_X) or
                rects_near(bundles[pos1]['rect'],
                           bundles[pos2]['rect'], PIXEL_NEAR)):

                bundles[pos1]['rect'] = rect_after_merge(
                    bundles[pos1]['rect'],
                    bundles[pos2]['rect'])

                for p in bundles[pos1]['pos_bboxes']:
                    bboxes[p]['merged'] = True
                for p in bundles[pos2]['pos_bboxes']:
                    bboxes[p]['merged'] = True

                bundles[pos1]['pos_bboxes'].update(bundles[pos2]['pos_bboxes'])

                del bundles[pos2]
                pos2 = pos1 + 1
            else:
                pos2 += 1

            if pos2 >= len(bundles):
                pos1 += 1
                pos2 = pos1 + 1

        for bundle in bundles:
            if len(bundle['pos_bboxes']) > 1:
                new_bbox = {'rect': bundle['rect'],
                            'type': None, 'merged': False, }
                for p in bundle['pos_bboxes']:
                    if 'key_bbox' in bboxes[p] and bboxes[p]['key_bbox']:
                        new_bbox['key_bbox'] = True
                        break
                new_bboxes.append(new_bbox)

    for bbox in bboxes:
        if not bbox['merged']:
            new_bboxes.append(bbox)

    return new_bboxes


def forward_create_rows_repeated(rows, bboxes, img_binary, net, id_to_char,
                                 max_iteration):
    '''
    Forward create rows repeatedly until the rows do not change

    @param rows: Rows
    @param bboxes: Bounding boxes
    @param img_binary: Binary image
    @param net: A network3 object
    @param net: A dict map from id to char
    @param max_iteration: Max number of iteration

    @return: rows, bboxes
    '''
    rows_saved = rows
    rows = forward_create_rows(bboxes)
    count_iteration = 0
    while count_iteration < max_iteration and rows != rows_saved:
        bboxes = merge_absolute_in_row(rows, bboxes)
        recognize_bboxes(img_binary, bboxes, net, id_to_char)
        update_bbox_type(bboxes)
        bboxes = delete_inner_bbox(bboxes)
        update_bbox_type(bboxes)

        rows_saved = rows
        rows = forward_create_rows(bboxes)

        count_iteration += 1
    return rows, bboxes


def backward_create_rows_twice(bboxes, img_binary, net, id_to_char):
    '''
    Backward create rows twice

    @param bboxes: Bounding boxes
    @param img_binary: Binary image
    @param net: A network3 object
    @param net: A dict map from id to char

    @return: rows, bboxes
    '''
    rows = backward_create_rows(bboxes)
    bboxes = merge_absolute_in_row(rows, bboxes)
    recognize_bboxes(img_binary, bboxes, net, id_to_char)
    update_bbox_type(bboxes)
    bboxes = delete_inner_bbox(bboxes)
    update_bbox_type(bboxes)

    rows = backward_create_rows(bboxes)

    for row in rows:
        for pos in reversed(row['pos_bboxes']):
            if (bboxes[pos]['rect'][2] >= W_AT_LEAST_GOOD and
                    bboxes[pos]['rect'][3] >= H_AT_LEAST_GOOD):
                bboxes[pos]['key_bbox'] = True
                break

    return rows, bboxes


def all_bad_bbox_are_in_row(bboxes):
    '''
    Determine whether all bad bboxes are in row

    @param bboxes: Bounding boxes
    @return: A bool value
    '''

    for bbox in bboxes:
        if bbox['type'] == 'bad' and not bbox['in_row']:
            return False
    return True


def select_a_bad_as_key(bboxes):
    '''
    Select the largest not in row bad bbox as key_bbox

    @param bboxes: Bounding boxes
    '''

    bad_bboxes = []
    for pos, bbox in enumerate(bboxes):
        if bbox['type'] == 'bad' and not bbox['in_row']:
            bad_bboxes.append(
                {'pos': pos, 'area': bbox['rect'][2] * bbox['rect'][3]})
    bad_bboxes.sort(key=lambda x: -x['area'])
    if len(bad_bboxes) > 0:
        bboxes[bad_bboxes[0]['pos']]['key_bbox'] = True


def select_remaining_bad_as_key(bboxes):
    '''
    Select the remaining not in row bad bbox as key_bbox

    @param bboxes: Bounding boxes
    '''

    for _, bbox in enumerate(bboxes):
        if bbox['type'] == 'bad' and not bbox['in_row']:
            bbox['key_bbox'] = True


def draw_save_contour_bboxes(path_img, bboxes, contours, w, h):
    '''
    Draw contours and bboxes, and save

    @param path_img: Path of image
    @param bboxes: Bounding boxes
    @param contours: Contours
    @param w: Width of image
    @param h: Height of image
    '''
    img = np.zeros((h, w, 3), np.uint8)
    draw_contour_bboxes(img, bboxes, contours, w, h)
    cv2.imwrite(path_img, img)


def draw_contour_bboxes(img, bboxes, contours, w, h):
    '''
    Draw contours and bboxes to img

    @param img: An image to draw
    @param bboxes: Bounding boxes
    @param contours: Contours
    @param w: Width of image
    @param h: Height of image
    '''

    for i, _ in enumerate(contours):
        cv2.drawContours(img, contours, i, COLOR_CONTOUR)

    for i, bbox in enumerate(bboxes):
        color = color_config[bbox['type']]

        rect_x, rect_y, rect_w, rect_h = bbox['rect']
        pt1 = (rect_x, rect_y)
        pt2 = (rect_x + rect_w, rect_y + rect_h)
        cv2.rectangle(img, pt1, pt2, color)


def draw_save_rows(path_img, bboxes, contours, rows, w, h):
    '''
    Draw contours, bboxes and rows, and save

    @param path_img: Path of image
    @param bboxes: Bounding boxes
    @param contours: Contours
    @param rows: Rows
    @param w: Width of image
    @param h: Height of image
    '''
    img = draw_rows(bboxes, contours, rows, w, h)
    cv2.imwrite(path_img, img)


def draw_rows(bboxes, contours, rows, w, h):
    '''
    Draw contours, bboxes and rows, and return the image

    @param bboxes: Bounding boxes
    @param contours: Contours
    @param rows: Rows
    @param w: Width of image
    @param h: Height of image
    '''
    img = np.zeros((h, w, 3), np.uint8)
    draw_contour_bboxes(img, bboxes, contours, w, h)
    for row in rows:
        if len(row['pos_bboxes']) >= 1:
            pos_bbox = row['pos_bboxes'][0]
            rect = bboxes[pos_bbox]['rect']
            pt_top1 = (rect[0] + rect[2] / 2), rect[1]
            pt_bot1 = (rect[0] + rect[2] / 2), rect[1] + rect[3]
            for pos_bbox in row['pos_bboxes'][1:]:
                # if ('key_bbox' in bboxes[pos_bbox] and
                #        bboxes[pos_bbox]['key_bbox']):
                rect = bboxes[pos_bbox]['rect']
                pt_top2 = rect[0] + rect[2] / 2, \
                    ALPHA * pt_top1[1] + (1 - ALPHA) * rect[1]
                pt_bot2 = rect[0] + rect[2] / 2, \
                    ALPHA * pt_bot1[1] + (1 - ALPHA) * (rect[1] + rect[3])

                cv2.line(img, tuple(map(int, pt_top1)), tuple(
                    map(int, pt_top2)), COLOR_LINE,
                    thickness=1, lineType=cv2.LINE_AA)
                cv2.line(img, tuple(map(int, pt_bot1)), tuple(
                    map(int, pt_bot2)), COLOR_LINE,
                    thickness=1, lineType=cv2.LINE_AA)
                cv2.circle(img, tuple(map(int, pt_top2)), 5, COLOR_LINE)
                cv2.circle(img, tuple(map(int, pt_bot2)), 5, COLOR_LINE)
                pt_top1, pt_bot1 = pt_top2, pt_bot2
    return img
