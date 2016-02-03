'''
Image binarization

Copyright (C) 2016 Works Applications, all rights reserved
'''

import os

PROG_BINARIZE_IMG = 'ocrus_binarize_img'


def binarize_img(path_img, path_img_binary, k):
    '''
    Binarize an image

    @param path_img: Path of image to binarize
    @param path_img_binary: Path of the binarized image
    @param k: Parameter in WolfJolion algorithm
    '''
    cmd = '%s "%s" "%s" "%s"' % (PROG_BINARIZE_IMG,
                                 path_img, path_img_binary, k)
    os.system(cmd)


if __name__ == '__main__':
    path_img = '/home/csuncs89/0-priceless/programming/ocrus0_dataset/406/differentR/ipad/diffR_ipad_IMG_0007.JPG'
    path_img_binary = path_img + '_binarize.png'
    binarize_img(path_img, path_img_binary, 0.1)
