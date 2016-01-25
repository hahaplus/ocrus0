'''
Convert between all kinds of data format

Copyright (C) 2016 Works Applications, all rights reserved
'''


def to_chars(char_lines):
    chars = []
    chars_set = set()
    for char_line in char_lines:
        for ch in char_line:
            ch = ch.strip()
            if ch and ch not in chars_set:
                chars.append(ch)
                chars_set.add(ch)
    return chars
