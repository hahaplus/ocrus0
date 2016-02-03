'''
Deal with mnist data format

Copyright (C) 2016 Works Applications, all rights reserved
'''

import sys

import numpy as np
import cv2

from img_proc import remove_white_border


def gray_img_to_mnist_array(gray_img):
    '''
    Convert a gray image to mnist array, 255 for background
    @param gray_img: A gray image
    @return: A mnist float image array
    '''
    height, width = gray_img.shape

    m = max(height, width)
    square_img = np.empty((m, m), np.uint8)
    square_img.fill(255)
    offset_y = (m - height) / 2
    offset_x = (m - width) / 2

    square_img[offset_y:offset_y + height,
               offset_x:offset_x + width] = gray_img

    square_img = cv2.resize(square_img, (28, 28))

    float_img = np.empty((28, 28), np.float32)
    for x, row in enumerate(square_img):
        for y, v in enumerate(row):
            float_img[x][y] = (255 - v) / 255.0
    float_img = np.reshape(float_img, -1)

    return float_img


def remove_white_to_mnist_array(gray_img):
    '''
    Remove white border and convert to mnist array, 255 for background
    @param gray_img: A gray image to process
    @return: A mnist float image array
    '''
    return gray_img_to_mnist_array(remove_white_border(gray_img))


def mnist_array_to_gray_img(mnist_array):
    '''
    Convert mnist array to gray image, 255 for background
    @param mnist_arry: A mnist array image
    '''
    gray_img = np.empty((28, 28), np.uint8)
    for i, v in enumerate(mnist_array):
        gray_img[i / 28][i % 28] = int((1 - v) * 255)
    return gray_img


def pretty_print_char(float_img, f=sys.stdout):
    '''
    Pretty print a char of a mnist float img
    @param float_img: A mnist float img
    '''
    for i, x in enumerate(float_img):
        print >> f, ' ' if abs(x) < 1e-5 else 'x',
        if (i + 1) % 28 == 0:
            print >> f
