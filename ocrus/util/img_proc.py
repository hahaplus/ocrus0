'''
Image processing

Copyright (C) 2016 Works Applications, all rights reserved
'''


def remove_white_border(gray_img, bg_value=255):
    '''
    Remove white border of gray image

    @param gray_img: A gray image to remove upon
    @return: A gray image with white border removed
    '''
    height, width = gray_img.shape

    y1 = 0
    while y1 < height and all(gray_img[y1, :] == bg_value):
        y1 += 1

    y2 = height - 1
    while y2 >= 0 and all(gray_img[y2, :] == bg_value):
        y2 -= 1

    x1 = 0
    while x1 < width and all(gray_img[:, x1] == bg_value):
        x1 += 1

    x2 = width - 1
    while x2 >= 0 and all(gray_img[:, x2] == bg_value):
        x2 -= 1

    if y1 <= y2:
        gray_img = gray_img[y1: y2 + 1, :]
    if x1 <= x2:
        gray_img = gray_img[:, x1: x2 + 1]

    return gray_img


def get_img_roi(img, rect):
    '''
    Return roi image by giving a rect

    @param img: A image to take roi
    @param rect: A rect of (x, y, w, h)
    @return: A roi image
    '''
    return img[rect[1]: rect[1] + rect[3], rect[0]: rect[0] + rect[2]]
