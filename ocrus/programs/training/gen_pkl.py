# coding=utf-8
'''
Generate pkl format data for receipts

Copyright (C) 2016 Works Applications, all rights reserved
'''

import json
import os
import sys
import cPickle
import gzip

import cv2
import numpy as np

from ocrus.util.mnist_format import pretty_print_char
from ocrus.util.mnist_format import remove_white_border

SUFFIX_ID = 'full'
RATIO_TRAINING, RATIO_VALIDATION, RATIO_TEST = 0.0, 0.0, 1.0

PATH_CHARS_ID = '/home/csuncs89/0-priceless/programming/ocrus0_build/chars_id_%s.json' % SUFFIX_ID

if __name__ == '__main__':
    chars = [unichr(ord(u'0') + i) for i in range(10)]
    chars.extend([u'/', u'￥', u'円', u'年', u'月', u'日', u'-'])
    chars_id = json.load(open(PATH_CHARS_ID))

    char_to_id = {char_id[0]: char_id[1] for char_id in chars_id}

    if len(sys.argv) not in [2]:
        print '''Generate pkl format data for receipts

    Usage: %s path_image_list

    path_image_list
        A list of path of images, one path in one line''' % \
            (os.path.basename(sys.argv[0]))
        sys.exit(0)

    path_image_list = sys.argv[1].rstrip('/')

    ratios = (RATIO_TRAINING, RATIO_VALIDATION, RATIO_TEST)
    all_data = ([], [])
    img_names = []

    '''
    format for training_data
    ((Img1, Img2, ...),
     (0, 4, ...      ))
    Img is a numpy array of 784 float numbers, value 0->1 means white->black
    '''
    count_chars = 0
    for path_image in open(path_image_list):
        path_image = path_image.strip()
        if path_image == '':
            continue
        print path_image
        path_gt = '{path_image}_gt.json'.format(path_image=path_image)
        ocr_lines_gt = json.load(open(path_gt))
        path_binarize = '{path_image}_binarize.png'.format(
            path_image=path_image)
        img = cv2.imread(path_binarize, cv2.IMREAD_GRAYSCALE)
        pos_in_image = 0
        for ocr_line in ocr_lines_gt:
            for ch in ocr_line['chars']:
                bbox = ch['bounding_box']
                crop_img = img[bbox[1]:bbox[3], bbox[0]:bbox[2]]

                height, width = crop_img.shape
                # print crop_img.shape,

                crop_img = remove_white_border(crop_img)

                height, width = crop_img.shape
                # print crop_img.shape

                m = max(height, width)
                square_img = np.empty((m, m), np.uint8)
                square_img.fill(255)
                offset_y = (m - height) / 2
                offset_x = (m - width) / 2

                square_img[offset_y:offset_y + height,
                           offset_x:offset_x + width] = crop_img

                square_img = cv2.resize(square_img, (28, 28))
                saved_img = np.copy(square_img)

                float_img = np.empty((28, 28), np.float32)
                for x, row in enumerate(square_img):
                    for y, v in enumerate(row):
                        float_img[x][y] = (255 - v) / 255.0
                float_img = np.reshape(float_img, -1)
                # pretty_print_char(float_img)

                all_data[0].append(float_img)
                all_data[1].append(char_to_id[ch['text']])
                pos_in_image += 1
                img_names.append((os.path.basename(path_image), pos_in_image))
                count_chars += 1

                #cv2.namedWindow("saved_img", cv2.WINDOW_NORMAL)
                #cv2.imshow("saved_img", saved_img)
                # cv2.waitKey()

    training_data, validation_data, test_data = ([], []), ([], []), ([], [])
    ends = [ratios[0] * count_chars, (ratios[0] + ratios[1]) * count_chars]
    ends = map(int, ends)

    training_data = map(np.asarray,
                        (all_data[0][:ends[0]], all_data[1][:ends[0]]))
    validation_data = map(np.asarray,
                          (all_data[0][ends[0]:ends[1]],
                           all_data[1][ends[0]:ends[1]]))
    test_data = map(np.asarray, (all_data[0][ends[1]:], all_data[1][ends[1]:]))

    path_pkl_gz = path_image_list + '.pkl.gz'
    f = gzip.open(path_pkl_gz, 'wb')
    cPickle.dump((training_data, validation_data, test_data), f)

    test_img_names = img_names[ends[1]:]
    path_test_imgs = path_image_list + '_test_imgs.json'
    json.dump(test_img_names, open(path_test_imgs, 'w'), indent=4)

    print map(len, (training_data[0], validation_data[0], test_data[0]))
    # print training_data
    # print validation_data
    # print test_data

    # for char_array in training_data[0]:
    #    pretty_print_char(char_array)
    #    print
