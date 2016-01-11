'''
Dealing with mnist data formt

Copyright (C) 2016 Works Applications, all rights reserved
'''

import numpy as np
import cv2


def binary_img_to_mnist_array(binary_img):
    '''
    Convert a binary image to mnist array.
    In the image, 255 for background and 0 for foreground
    @param binary_img: A binary image
    '''
    height, width = binary_img.shape

    m = max(height, width)
    square_img = np.empty((m, m), np.uint8)
    square_img.fill(255)
    offset_y = (m - height) / 2
    offset_x = (m - width) / 2

    square_img[offset_y:offset_y + height,
               offset_x:offset_x + width] = binary_img

    square_img = cv2.resize(square_img, (28, 28))

    float_img = np.empty((28, 28), np.float32)
    for x, row in enumerate(square_img):
        for y, v in enumerate(row):
            float_img[x][y] = (255 - v) / 255.0
    float_img = np.reshape(float_img, -1)

    return float_img


def pretty_print_char(float_img):
    '''
    Pretty print a char of a mnist float img
    @param float_img: A mnist float img
    '''
    for i, x in enumerate(float_img):
        print ' ' if x == 0.0 else 'x',
        if i % 28 == 0:
            print
