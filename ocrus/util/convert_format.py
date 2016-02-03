'''
Convert between all kinds of data structure

Copyright (C) 2016 Works Applications, all rights reserved
'''


def to_chars(unicode_s):
    '''
    Extract non-repeated and non-blank chars from a unicode string

    @param unicode_s: A unicode string
    @return: A list of chars
    '''

    chars = []
    chars_set = set()

    for ch in unicode_s:
        ch = ch.strip()
        if ch and ch not in chars_set:
            chars.append(ch)
            chars_set.add(ch)
    return chars


def rect_to_ltrb(rect):
    '''
    Convert from rect to left, top, right, bot tuple

    @param rect: A rect
    @return: (left, top, right, bot)
    '''
    left, top = rect[0], rect[1]
    right, bot = rect[0] + rect[2], rect[1] + rect[3]
    return left, top, right, bot
