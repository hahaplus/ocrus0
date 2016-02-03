'''
Image binarization

Copyright (C) 2016 Works Applications, all rights reserved
'''

import os

PROG_BINARIZE_IMG = 'ocrus_binarize_img'
PROG_PREPROCESS_IMG = 'ocrus_preprocess_img'


def binarize_img(path_img, path_img_binary, k):
    '''
    Binarize an image

    @param path_img: Path of image to binarize
    @param path_img_binary: Path of the binarized image
    @param k: 0~1.0, parameter in WolfJolion binarization algorithm
    '''
    cmd = '%s "%s" "%s" "%s"' % (PROG_BINARIZE_IMG,
                                 path_img, path_img_binary, k)
    os.system(cmd)


def preprocess_img(path_img, path_img_binary, k, e):
    '''
    Preprocess a color image (remove noise, binarize and enhance)

    @param path_img: Path of image to preprocess
    @param path_img_binary: Path of the preprocessed image
    @param k: 0~1.0, parameter in WolfJolion binarization algorithm
    @param e: 0~1.0, larger value gives stronger enhancement, 0: no enhancement
    '''
    cmd = '%s "%s" "%s" %s %s' % (PROG_PREPROCESS_IMG,
                                  path_img, path_img_binary, k, e)
    os.system(cmd)
