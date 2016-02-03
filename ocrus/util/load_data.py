'''
Load various kinds of data

Copyright (C) 2016 Works Applications, all rights reserved
'''

import cPickle
import gzip
import json


def load_chars_id(path):
    '''
    Load data from path_chars_id and return chars_id, char_to_id, id_to_char
    Format for chars_id:
        ((char, char_id), ...)
    Format for char_to_id:
        {char: char_id, ... }
    Format for id_char:
        {char_id: char, ...}
    @param path: Path to chars_id file
    @return: chars_id, char_to_id, id_to_char
    '''
    chars_id = json.load(open(path))
    char_to_id = {char_id[0]: char_id[1] for char_id in chars_id}
    id_to_char = {char_id[1]: char_id[0] for char_id in chars_id}

    return chars_id, char_to_id, id_to_char


def load_mnist_data(path):
    '''
    Load mnist data and return (training_data, validation_data, test_data)
    @param: Path of mnist data
    @return: ((imgs, ids), (imgs, ids), (imgs, ids))
    '''
    return cPickle.load(gzip.open(path, 'rb'))


def load_image_list(path):
    '''
    Load image list from path, the blank lines are ignored
    @param path: Path to image_list
    @return: [path1, path2, ]
    '''
    list_path = []
    for path_image in open(path):
        path_image = path_image.strip()
        if path_image != '':
            list_path.append(path_image)

    return list_path
