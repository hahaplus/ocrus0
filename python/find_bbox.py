# coding=utf-8
'''
Recognize receipts

Copyright (C) 2016 Works Applications, all rights reserved
'''


import codecs
import locale
import random
import sys

import cv2
import numpy as np

from ocrus.neural_network import load_data
from ocrus.neural_network.net_v1 import create_network
from ocrus.neural_network.net_v1 import NET_ID
from ocrus.neural_network.mnist_format import pretty_print_char
from ocrus.neural_network.mnist_format import remove_white_to_mnist_array
from ocrus.neural_network.convert_format import to_chars


sys.stdout = codecs.getwriter(locale.getpreferredencoding())(sys.stdout)

PATH_PARAMS = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_jpn_param.pkl' % NET_ID
PATH_CHARS_ID = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_chars_id.json' % NET_ID

PROB_CHAR = 0.95

COLOR_CONTOUR = (180, 180, 180)
color_config = {
    'good': (0, 200, 0),
    'bad': (0, 200, 200),
    'small': (255, 255, 255),
    'big': (0, 0, 255)
}

simple_char_line = u'''
0
/ - ,  ， 。 ' "
l o
・ ー
一
'''
simple_chars = to_chars([simple_char_line])

ALPHA = 0.2
OVERLAP_AT_LEAST = 0.3

COLOR_LINE = (200, 0, 200)
COLOR_CIRCLE = (255, 0, 255)

# Relax and get more bad chars, when update_bbox_type
RATIO_RELAX_SMALL_W = 0.8  # <= 1.0
RATIO_RELAX_SMALL_H = 1.0  # <= 1.0
RATIO_RELAX_BIG_W = 1.2  # >= 1.0
RATIO_RELAX_BIG_H = 1.2  # >= 1.0

# Get more key chars when add_bbox_to_row
RATIO_TO_SMALL = 0.8  # <= 1.0
RATIO_TO_BIG = 1.2  # >= 1.0

# Two rects are near pixels between them are <= this value
PIXEL_NEAR = 0


def type_print(o):
    print 'type:', type(o)
    print o
    print


def msg_print(msg, o):
    print msg + ':'
    print o
    print


def get_img_roi(img, rect):
    '''
    Return roi for img by giving a rect
    @param img: A image to take roi
    @param rect: A rect of (x, y, w, h)
    @return: A roi image
    '''
    return img[rect[1]: rect[1] + rect[3], rect[0]: rect[0] + rect[2]]


def rect2_in_rect1(rect1, rect2):
    '''
    r1[0],r1[1]
    --------------
    |            |
    |  ------    |
    |  |    |    |
    |  ------    |
    |            |
    -------------- r1[0]+r1[2],r1[1]+r1[3]
    '''
    return rect1[0] <= rect2[0] and rect1[1] <= rect2[1] and \
        rect1[0] + rect1[2] >= rect2[0] + rect2[2] and \
        rect1[1] + rect1[3] >= rect2[1] + rect2[3]


def delete_inner_bbox(bboxes):
    '''
    Delete small, good or bad that are inside good or bad
    Note: the param bboxes is not changed
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

    good_rect_ws = [bbox['rect'][2] for bbox in bboxes
                    if bbox['type'] == 'good']
    good_rect_hs = [bbox['rect'][3] for bbox in bboxes
                    if bbox['type'] == 'good']
    max_rect_w, min_rect_w = max(good_rect_ws), min(good_rect_ws)
    max_rect_h, min_rect_h = max(good_rect_hs), min(good_rect_hs)

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


def draw_save_contour_bboxes(path_img, bboxes, contours, w, h):
    img = np.zeros((h, w, 3), np.uint8)
    draw_contour_bboxes(img, bboxes, contours, w, h)
    cv2.imwrite(path_img, img)


def draw_contour_bboxes(img, bboxes, contours, w, h):
    for i, _ in enumerate(contours):
        cv2.drawContours(img, contours, i, COLOR_CONTOUR)

    for i, bbox in enumerate(bboxes):
        color = color_config[bbox['type']]

        rect_x, rect_y, rect_w, rect_h = bbox['rect']
        pt1 = (rect_x, rect_y)
        pt2 = (rect_x + rect_w, rect_y + rect_h)
        cv2.rectangle(img, pt1, pt2, color)


def draw_save_rows(path_img, bboxes, contours, rows, w, h):
    img = np.zeros((h, w, 3), np.uint8)
    draw_contour_bboxes(img, bboxes, contours, w, h)
    for row in rows:
        pos_bbox = row['pos_bboxes'][0]
        rect = bboxes[pos_bbox]['rect']
        pt_top1 = (rect[0] + rect[2] / 2), rect[1]
        pt_bot1 = (rect[0] + rect[2] / 2), rect[1] + rect[3]
        for pos_bbox in row['pos_bboxes'][1:]:
            rect = bboxes[pos_bbox]['rect']
            pt_top2 = rect[0] + rect[2] / 2, \
                ALPHA * pt_top1[1] + (1 - ALPHA) * rect[1]
            pt_bot2 = rect[0] + rect[2] / 2, \
                ALPHA * pt_bot1[1] + (1 - ALPHA) * (rect[1] + rect[3])

            cv2.line(img, tuple(map(int, pt_top1)), tuple(
                map(int, pt_top2)), COLOR_LINE,
                thickness=2, lineType=cv2.LINE_AA)
            cv2.line(img, tuple(map(int, pt_bot1)), tuple(
                map(int, pt_bot2)), COLOR_LINE,
                thickness=2, lineType=cv2.LINE_AA)
            cv2.circle(img, tuple(map(int, pt_top2)), 5, COLOR_LINE)
            cv2.circle(img, tuple(map(int, pt_bot2)), 5, COLOR_LINE)
            pt_top1, pt_bot1 = pt_top2, pt_bot2
    cv2.imwrite(path_img, img)


def overlap_ratio(start1, end1, start2, end2):
    '''
    Calculate overlapping ratio
    Formula: overlap * 2 / (h1 + h2)
    start1                               start2

               start2   |   start1
                        |
                        |
    end1                |                end2

               end2         end1
    @param start1: Top y1
    @param end1: Bottom y1
    @param start2: Top y2
    @param end2: Bottom y2
    @return: The ratio
    '''
    if start2 > start1:
        overlap = end1 - start2
    else:
        overlap = end2 - start1

    overlap = 0 if overlap < 0 else overlap
    len1, len2 = end1 - start1 + 1, end2 - start2 + 1
    if len1 + len2 == 0:
        return 0.0
    else:
        return float(overlap) * 2 / (len1 + len2)


def overlap_ratio_in_second(start1, end1, start2, end2):
    '''
    Calculate overlapping ratio
    Formula: overlap * 2 / len2
    start1                               start2

               start2   |   start1
                        |
                        |
    end1                |                end2

               end2         end1
    @param start1: Top y1
    @param end1: Bottom y1
    @param start2: Top y2
    @param end2: Bottom y2
    @return: The ratio
    '''
    if start2 > start1:
        overlap = end1 - start2
    else:
        overlap = end2 - start1

    overlap = 0 if overlap < 0 else overlap
    len2 = end2 - start2 + 1
    if len2 == 0:
        return 0.0
    else:
        return float(overlap) * 2 / len2


def inside_row(y_top_row, y_bot_row, y_top, y_bot):
    return y_top_row <= y_top and y_bot <= y_bot_row


def backward_create_rows(bboxes):
    prepare_add_to_row(bboxes)
    rows = []
    for i in range(len(bboxes) - 1, -1, -1):
        add_bbox_to_row(rows, bboxes, i)
    return rows


def forward_create_rows(bboxes):
    prepare_add_to_row(bboxes)
    rows = []
    for i in range(len(bboxes)):
        add_bbox_to_row(rows, bboxes, i)
    return rows


def add_bbox_to_row(rows, bboxes, i):
    bbox = bboxes[i]
    if bbox['type'] in ['good', 'bad', 'small']:
        ratio_max, row_best = 0.0, None

        for row in rows:
            ratio = overlap_ratio_in_second(row['y_top'], row['y_bot'],
                                            bbox['y_top'], bbox['y_bot'])
            if ratio > ratio_max:
                ratio_max, row_best = ratio, row

        if bbox['type'] == 'good':
            bbox['key_bbox'] = True

        if row_best is not None and ratio_max > OVERLAP_AT_LEAST:
            row_h = row_best['y_bot'] - row_best['y_top']
            h_min = min(row_h, row_best['h_min']) * RATIO_TO_SMALL
            h_max = max(row_h, row_best['h_max']) * RATIO_TO_BIG

            row_best['pos_bboxes'].append(i)

            if bbox['type'] == 'bad':
                if h_min <= bbox['h'] <= h_max:
                    bbox['key_bbox'] = True
            elif bbox['type'] == 'small':
                bbox['row_y_top'] = row_best['y_top']
                bbox['row_y_bot'] = row_best['y_bot']

            if bbox['key_bbox']:
                row_best['y_top'] = ALPHA * row_best['y_top'] + \
                    (1 - ALPHA) * bbox['y_top']
                row_best['y_bot'] = ALPHA * row_best['y_bot'] + \
                    (1 - ALPHA) * bbox['y_bot']

                row_best['h_min'] = min(row_best['h_min'], bbox['rect'][3])
                row_best['h_max'] = max(row_best['h_max'], bbox['rect'][3])

        elif bbox['key_bbox']:
            new_row = {'y_top': bbox['y_top'], 'y_bot': bbox['y_bot'],
                       'pos_bboxes': [i, ],
                       'h_min': bbox['rect'][3], 'h_max': bbox['rect'][3]}
            rows.append(new_row)


def rm_too_small_from_row(rows, bboxes):
    for row in rows:
        row['w_max'], row['w_min'] = 0, float('inf')
        row['h_max'], row['h_min'] = 0, float('inf')
        for pos in row['pos_bboxes']:
            if 'key_bbox' in bboxes[pos] and bboxes[pos]['key_bbox']:
                row['w_max'] = max(row['w_max'], bboxes[pos]['rect'][2])
                row['w_min'] = min(row['w_min'], bboxes[pos]['rect'][2])
                row['h_max'] = max(row['h_max'], bboxes[pos]['rect'][3])
                row['h_min'] = min(row['h_min'], bboxes[pos]['rect'][3])

        pos_bboxes = []
        w_min = row['w_min'] * RATIO_TO_SMALL
        h_min = row['h_min'] * RATIO_TO_SMALL
        for pos in row['pos_bboxes']:
            if bboxes[pos]['type'] in ['good', 'bad']:
                pos_bboxes.append(pos)
            elif bboxes[pos]['type'] == 'small':
                if bboxes[pos]['rect'][3] > h_min:
                    pos_bboxes.append(pos)
                elif bboxes[pos]['rect'][2] > w_min:
                    row_y_mid = (
                        (bboxes[pos]['row_y_top'] +
                         bboxes[pos]['row_y_bot']) / 2)
                    row_h = (bboxes[pos]['row_y_bot'] -
                             bboxes[pos]['row_y_top'])
                    if (not (bboxes[pos]['rect'][1] > row_y_mid + row_h / 4 or
                             bboxes[pos]['rect'][1] +
                             bboxes[pos]['rect'][3] < row_y_mid - row_h / 4)):
                        pos_bboxes.append(pos)
        row['pos_bboxes'] = pos_bboxes


def rect_to_ltrb(rect):
    left, top = rect[0], rect[1]
    right, bot = rect[0] + rect[2], rect[1] + rect[3]
    return left, top, right, bot


def rects_distance(rect1, rect2):
    '''
    Negative return value means they are overlapping
    r1[0],r1[1]
    --------------
    |            |
    |            |
    |            |
    |            |
    |            |
    -------------- r1[0]+r1[2],r1[1]+r1[3]
    '''
    left1, top1, right1, bot1 = rect_to_ltrb(rect1)
    left2, top2, right2, bot2 = rect_to_ltrb(rect2)
    d_list = top1 - bot2, left1 - right2, top2 - bot1, left2 - right1
    if all([d < 0 for d in d_list]):
        dist = max(d_list)
    else:
        dist = max([d for d in d_list if d >= 0])

    return dist


def rects_are_near(rect1, rect2):
    return rects_distance(rect1, rect2) < PIXEL_NEAR


def rects_overlap_in_x(rect1, rect2):
    left1, _, right1, _ = rect_to_ltrb(rect1)
    left2, _, right2, _ = rect_to_ltrb(rect2)
    return (overlap_ratio_in_second(left1, right1, left2, right2) > 0.5 or
            overlap_ratio_in_second(left2, right2, left1, right1) > 0.5)


def rect_after_merge(rect1, rect2):
    left1, top1, right1, bot1 = rect_to_ltrb(rect1)
    left2, top2, right2, bot2 = rect_to_ltrb(rect2)
    left, top = min(left1, left2), min(top1, top2)
    right, bot = max(right1, right2), max(bot1, bot2)
    rect = left, top, right - left, bot - top
    return rect


def recognize_bboxes(img_binary, bboxes):
    pos_result_to_bboxes = {}
    float_imgs = []
    for i, bbox in enumerate(bboxes):
        if 'ch' not in bbox:
            img_char = get_img_roi(img_binary, bbox['rect'])
            float_img = remove_white_to_mnist_array(img_char)
            bbox['float_img'] = float_img
            bbox['type'] = None
            float_imgs.append(bbox['float_img'])
            pos_result_to_bboxes[len(float_imgs) - 1] = i

    if len(float_imgs) > 0:
        result = net.predict_xs(np.asarray(float_imgs))
        for i, y_out in enumerate(result[0]):
            ch = id_to_char[y_out]
            prob = result[1][i][y_out]
            j = pos_result_to_bboxes[i]
            bboxes[j]['ch'], bboxes[j]['prob'] = ch, prob
            if ch not in simple_chars and prob > PROB_CHAR:
                bboxes[j]['type'] = 'good'


def prepare_add_to_row(bboxes):
    for bbox in bboxes:
        bbox['x_left'] = bbox['rect'][0]
        bbox['y_top'] = bbox['rect'][1]
        bbox['y_bot'] = bbox['rect'][1] + bbox['rect'][3]
        bbox['h'] = bbox['rect'][3]
        if 'key_bbox' not in bbox:
            bbox['key_bbox'] = (bbox['type'] == 'good')
    bboxes.sort(key=lambda b: b['x_left'])


def merge_near_bboxes_in_row(rows, bboxes):
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
            if (rects_are_near(bundles[pos1]['rect'],
                               bundles[pos2]['rect']) or
                rects_overlap_in_x(bundles[pos1]['rect'],
                                   bundles[pos2]['rect'])):

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
                new_bboxes.append(
                    {'rect': bundle['rect'], 'type': None, 'merged': False})

    for bbox in bboxes:
        if not bbox['merged']:
            new_bboxes.append(bbox)

    return new_bboxes


def process_img(net, path_image, id_to_char):
    print 'Process image', path_image
    path_img_binary = '{path_image}_binarize.png'.format(
        path_image=path_image)
    path_img_contour = '{path_image}_contour.png'.format(
        path_image=path_image)
    path_img_contour2 = '{path_image}_contour2.png'.format(
        path_image=path_image)
    path_img_contour3 = '{path_image}_contour3.png'.format(
        path_image=path_image)
    path_img_contour4 = '{path_image}_contour4.png'.format(
        path_image=path_image)

    img_binary = cv2.imread(path_img_binary, cv2.IMREAD_GRAYSCALE)
    _, img = cv2.threshold(img_binary, 128, 255, cv2.THRESH_BINARY_INV)

    _, contours, _ = cv2.findContours(
        img.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

    bboxes = [{'rect': cv2.boundingRect(
        contour), 'type': None, 'merged': False} for contour in contours]
    h, w = img_binary.shape[:2]

    recognize_bboxes(img_binary, bboxes)
    update_bbox_type(bboxes)
    bboxes = delete_inner_bbox(bboxes)
    update_bbox_type(bboxes)

    # Backward add to row
    rows = backward_create_rows(bboxes)
    bboxes = merge_near_bboxes_in_row(rows, bboxes)
    recognize_bboxes(img_binary, bboxes)
    update_bbox_type(bboxes)
    bboxes = delete_inner_bbox(bboxes)
    update_bbox_type(bboxes)
    rows = backward_create_rows(bboxes)

    # Forward add to row
    rows = forward_create_rows(bboxes)
    bboxes = merge_near_bboxes_in_row(rows, bboxes)
    recognize_bboxes(img_binary, bboxes)
    update_bbox_type(bboxes)
    bboxes = delete_inner_bbox(bboxes)
    update_bbox_type(bboxes)
    rows = forward_create_rows(bboxes)

    rm_too_small_from_row(rows, bboxes)

    draw_save_rows(path_img_contour4, bboxes, contours, rows, w, h)

    path_symbol = '{path_image}_symbol.txt'.format(path_image=path_image)
    f_symbol = codecs.open(path_symbol, 'w', 'utf-8')
    for row in rows:
        for pos in row['pos_bboxes']:
            rect = bboxes[pos]['rect']
            line = "word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n" %\
                (bboxes[pos]['ch'], bboxes[pos]['prob'] * 100,
                 rect[0], rect[1], rect[0] + rect[2], rect[1] + rect[3])
            f_symbol.write(line)
    f_symbol.close()


path_image_list = sys.argv[1].rstrip('/')
list_path = [
    #'/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/2592_1936_ipadmini/ipadmini_IMG_0054.JPG',
    #'/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/differentR/ipadmini/diffR_ipadmini_IMG_0038.JPG',
    #'/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/2592_1936_ipadmini/ipadmini_IMG_0348.JPG',
    #'/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/2592_1936_ipadmini/ipadmini_IMG_0130.JPG',
    #'/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/2592_1936_ipad/ipad_IMG_0104.JPG',
    '/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/2592_1936_ipadmini/ipadmini_IMG_0078.JPG',
]
list_path = load_data.load_image_list(path_image_list)

mini_batch_size = 50
net = create_network(PATH_PARAMS, mini_batch_size)

chars_id, char_to_id, id_to_char = load_data.load_chars_id(PATH_CHARS_ID)

for path in list_path:
    process_img(net, path, id_to_char)
