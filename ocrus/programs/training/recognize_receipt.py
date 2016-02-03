# coding=utf-8
'''
Recognize receipts

Copyright (C) 2016 Works Applications, all rights reserved
'''

import sys
import json
import codecs
import os
import shutil

import cv2
import numpy as np

from ocrus.neural_network import load_data, network3
from ocrus.neural_network.mnist_format import gray_img_to_mnist_array
from ocrus.neural_network.mnist_format import pretty_print_char
from ocrus.neural_network.mnist_format import remove_white_border
from ocrus.neural_network.net_v3 import create_network, NET_ID


import locale
sys.stdout = codecs.getwriter(locale.getpreferredencoding())(sys.stdout)

TEST_406 = True


PATH_IMAGE_LIST = '/home/csuncs89/0-priceless/programming/ocrus0_build/406.txt'
PATH_PARAMS = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_jpn_param.pkl' % NET_ID
PATH_CHARS_ID = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_chars_id.json' % NET_ID

PATH_DATA = '/home/csuncs89/0-priceless/programming/ocrus0_build/%s_jpn_expanded.pkl.gz' % NET_ID

if TEST_406:
    PATH_DATA = PATH_IMAGE_LIST + '.pkl.gz'

PATH_IMG_NAMES = PATH_IMAGE_LIST + '_test_imgs.json'

SIMPLE_REPLACE = {
    u'o': u'0',
    u'O': u'0',
    u's': u'5',
    u'S': u'5',
    u'z': u'2',
    u'Z': u'2',
    u'l': u'1',
    u'ヨ': u'3',
    u'Y': u'￥',
    u'。': u'0',
    u'z': u'2',
    u'Z': u'2',
    u'ゔ': u'5',
}


def predict_print(test_data, f=sys.stdout):
    result = net.predict_xs(test_data[0].get_value())
    img_names = json.load(open(PATH_IMG_NAMES))

    count_correct = 0
    for i, (test_x, test_y) in enumerate(zip(test_data[0].get_value(),
                                             test_data[1].eval())):
        y_out = result[0][i]
        if id_to_char[y_out] in SIMPLE_REPLACE:
            y_out = char_to_id[SIMPLE_REPLACE[id_to_char[y_out]]]
        prob_y = result[1][i]

        k_top = 10
        top_y_out = prob_y.argsort()[-k_top:][::-1]

        img_name = img_names[i]

        if y_out != test_y:
            print >> f
            pretty_print_char(test_x, f)
            print >> f
            print >> f, img_name
            print >> f, y_out, id_to_char[y_out]
            for y_o in top_y_out:
                print >> f, id_to_char[y_o], prob_y[y_o]
            print >> f
        else:
            count_correct += 1
    print 'predict_print, Accuracy = %.4f (%d/%d)' % \
        (float(count_correct) / len(result[0]),
         count_correct, len(result[0]))


def predict_img(path_gray_img):
    result = net.predict_img(path_gray_img)
    print id_to_char[result[0]]

chars_id, char_to_id, id_to_char = load_data.load_chars_id(PATH_CHARS_ID)

# data1, data2, data3 = load_data.load_mnist_data(PATH_DATA)
training_data, validation_data, test_data = network3.load_data_shared(
    PATH_DATA)

mini_batch_size = 50
net = create_network(PATH_PARAMS, mini_batch_size)


def train():
    num_epoch = 500
    print 'Train for %s epoches ...' % num_epoch
    net.SGD(training_data, num_epoch, mini_batch_size, 0.03,
            validation_data, test_data)

# net.SGD(training_data, 40, mini_batch_size, 0.03,
#        validation_data, test_data)

if __name__ == "__main__":
    if TEST_406:

        predict_print(
            test_data,
            codecs.open(
                '/home/csuncs89/0-priceless/programming/ocrus0_build/406.txt_out.txt',
                'w',
                'utf-8'))

        print net.accuracy(test_data)

        path_image_list = sys.argv[1].rstrip('/')

        for path_image in open(path_image_list):
            path_image = path_image.strip()
            if path_image == '':
                continue
            print 'Processing', path_image, '...'

            path_txt = '{path_image}_symbol.txt'.\
                format(path_image=path_image)
            path_txt_tess = '{path_image}_symbol_tess.txt'.\
                format(path_image=path_image)

            if not os.path.exists(path_txt_tess):
                shutil.copy(path_txt, path_txt_tess)
            path_binarize = '{path_image}_binarize.png'.format(
                path_image=path_image)

            img = cv2.imread(path_binarize, cv2.IMREAD_GRAYSCALE)
            ocr_chars = []
            for line in codecs.open(path_txt_tess, encoding='utf-8'):
                from ocrus.ocr_result import parse_line_v1
                d = parse_line_v1(line)
                ocr_chars.append(d)

            float_imgs = []
            for ocr_char in ocr_chars:
                bbox = ocr_char['bounding_box']
                crop_img = img[bbox[1]:bbox[3], bbox[0]:bbox[2]]
                float_img = gray_img_to_mnist_array(
                    remove_white_border(crop_img))
                float_imgs.append(float_img)
            float_imgs = np.asarray(float_imgs)

            f = codecs.open(path_txt, 'w', encoding='utf-8')
            result = net.predict_xs(float_imgs)
            for i, ocr_char in enumerate(ocr_chars):
                y_out = result[0][i]
                text = id_to_char[y_out]
                bbox = ocr_char['bounding_box']
                confidence = result[1][i][y_out] * 100
                print >> f, "word: '%s';\tconf: %.2f; bounding_box: %d,%d,%d,%d;" % (
                    text, confidence, bbox[0], bbox[1], bbox[2], bbox[3])
            f.close()
    else:
        train()
    pass
