# coding=utf-8
'''
Recognize receipts

Copyright (C) 2016 Works Applications, all rights reserved
'''

import cPickle
import codecs
import locale
import os
import sys

import cv2

from ocrus.preprocessing.binarize import binarize_img
from ocrus.recognition.neural_network.net_v1 import create_network, NET_ID
from ocrus.segmentation.row_analysis import all_bad_bbox_are_in_row
from ocrus.segmentation.row_analysis import backward_create_rows
from ocrus.segmentation.row_analysis import backward_create_rows_twice
from ocrus.segmentation.row_analysis import delete_inner_bbox
from ocrus.segmentation.row_analysis import draw_rows, draw_save_rows
from ocrus.segmentation.row_analysis import forward_create_rows_repeated
from ocrus.segmentation.row_analysis import recognize_bboxes
from ocrus.segmentation.row_analysis import rm_too_small_from_row
from ocrus.segmentation.row_analysis import select_a_bad_as_key
from ocrus.segmentation.row_analysis import update_bbox_type
from ocrus.util import load_data

sys.stdout = codecs.getwriter(locale.getpreferredencoding())(sys.stdout)

K_BINARIZE = 0.1
SHOW_CV2_IMG = False
DEBUG_SINGLE_IMG = False

PATH_PARAMS = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_jpn_param.pkl' % NET_ID
PATH_CHARS_ID = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_chars_id.json' % NET_ID


def process_img(path_img, net, id_to_char):
    print 'Process image', path_img
    path_img_binary = '{path_img}_binarize.png'.format(
        path_img=path_img)
    binarize_img(path_img, path_img_binary, K_BINARIZE)

    path_img_contour4 = '{path_img}_contour4.png'.format(
        path_img=path_img)
    path_bboxes = '{path_img}_bboxes.pkl'.format(
        path_img=path_img)

    img_binary = cv2.imread(path_img_binary, cv2.IMREAD_GRAYSCALE)
    _, img = cv2.threshold(img_binary, 128, 255, cv2.THRESH_BINARY_INV)

    _, contours, _ = cv2.findContours(
        img.copy(), cv2.RETR_LIST, cv2.CHAIN_APPROX_NONE)

    h, w = img_binary.shape[:2]

    bboxes = [{'rect': cv2.boundingRect(contour),
               'type': None,
               'merged': False}
              for contour in contours]
    loaded = False
    if os.path.exists(path_bboxes):
        try:
            bboxes = cPickle.load(open(path_bboxes, 'rb'))
            loaded = True
        except Exception:
            loaded = False
    if not loaded:
        recognize_bboxes(img_binary, bboxes, net, id_to_char)
        cPickle.dump(bboxes, open(path_bboxes, 'wb'))

    update_bbox_type(bboxes)
    bboxes = delete_inner_bbox(bboxes)
    update_bbox_type(bboxes)

    if SHOW_CV2_IMG:
        cv2.namedWindow("rows", cv2.WINDOW_NORMAL)

    if SHOW_CV2_IMG:
        rows = backward_create_rows(bboxes)
        print '  Backward once (for debug)'
        cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
        cv2.waitKey()

    print 'Create good rows ...'
    rows, bboxes = backward_create_rows_twice(bboxes, img_binary,
                                              net, id_to_char)
    if SHOW_CV2_IMG:
        print '  Backward twice'
        cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
        cv2.waitKey()

    rows, bboxes = forward_create_rows_repeated(rows, bboxes, img_binary,
                                                net, id_to_char)
    if SHOW_CV2_IMG:
        print '  Forward repeated'
        cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
        cv2.waitKey()

    print 'Create bad rows ...'
    while not all_bad_bbox_are_in_row(bboxes):
        select_a_bad_as_key(bboxes)
        rows, bboxes = backward_create_rows_twice(bboxes, img_binary,
                                                  net, id_to_char)
        if SHOW_CV2_IMG:
            print '  Backward twice'
            cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
            cv2.waitKey()

        rows, bboxes = forward_create_rows_repeated(rows, bboxes, img_binary,
                                                    net, id_to_char)
        if SHOW_CV2_IMG:
            print '  Forward repeated'
            cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
            cv2.waitKey()

    rows = rm_too_small_from_row(rows, bboxes)

    draw_save_rows(path_img_contour4, bboxes, contours, rows, w, h)

    if SHOW_CV2_IMG:
        print 'Small removed ...'
        cv2.imshow('rows', draw_rows(bboxes, contours, rows, w, h))
        cv2.waitKey()

    path_symbol = '{path_img}_symbol.txt'.format(path_img=path_img)
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
list_path = []
if not DEBUG_SINGLE_IMG:
    list_path = load_data.load_image_list(path_image_list)

mini_batch_size = 10
net = create_network(PATH_PARAMS, mini_batch_size)

chars_id, char_to_id, id_to_char = load_data.load_chars_id(PATH_CHARS_ID)

for path in list_path:
    process_img(path, net, id_to_char)
