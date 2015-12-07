# coding=utf-8
'''
Provide for dealing with all kinds of OCR result formats

Copyright (C) 2015 Works Applications, all rights reserved

Created on Dec 4, 2015

@author: Chang Sun
'''

import io
import json
import re


REPLACE_TABLE = {
    u'ー': u'1',
    u'一': u'1',
    u'。': u'0',
    u'〇': u'0',
    u'ら':  u'5',
    u'g': u'9',
    u'乃': u'月',
    u'巳': u'日',
    u'\\': u'￥',
    u'~': u' ',
}


def parse_line_v1(line):
    '''
    Parse the line and fill the dict (See doc/ocr_result_format.txt)
        {'text': None, 'bounding_box': None, 'confidence': None}
    Example line to parse:
        word: 'g';      conf: 61.91; bounding_box: 411,1778,431,1906;

    @param line: Line to parse
    @return: A dict
    '''
    d = {'text': None, 'bounding_box': None, 'confidence': None}
    for seg in map(unicode.strip, line.split(';')):
        if seg.startswith('word:'):
            d['text'] = unicode(seg[len('word:'):].strip()[1:-1])
        elif seg.startswith('bounding_box:'):
            d['bounding_box'] = map(int, seg[len('bounding_box:'):].split(','))
        elif seg.startswith('conf:'):
            d['confidence'] = float(seg[len('conf:'):].strip())
    return d


def replace_by_table(s, table):
    '''
    Replace a unicode string by a replacement table

    @param s: A unicode string
    @param table: A unicode table
    @return: A new replaced unicode string
    '''
    return u''.join(
        map(lambda x: table[x] if x in table else x,
            list(s)))


def normalize_ocr_lines(path_ocr_lines):
    '''
    Convert the rect in char to bounding_box
    @param path_ocr_lines: Path of ocr_lines
    '''
    ocr_lines = json.load(open(path_ocr_lines))
    for ocr_line in ocr_lines:
        for ch in ocr_line['chars']:
            if 'rect' in ch:
                left, top, width, height = ch['rect']
                ch['bounding_box'] = [left, top, left + width, top + height]

    with io.open(path_ocr_lines, 'w', encoding='utf-8') as json_file:
        data = json.dumps(ocr_lines, ensure_ascii=False, indent=2)
        json_file.write(unicode(data))


def replace_if_exist(ch, table):
    '''
    Replace ch by a substitute in table is ch exists in table, otherwise keep
    @param ch: A char
    @param table: A replacement table
    @return: A replaced char or ch itself
    '''
    return table[ch] if ch in table else ch


def extract_date(s):
    '''
    Extract dates from a unicode string

    @param s: A unicode string
    @return: A list of (start_pos, end_pos)
    '''
    matches = re.finditer(ur'''[0-9ー一。〇らg]?[0-9ー一。〇らg~\s]* # Year
                        [年]?
                        [0-9ー一。〇らg\s]+ # Month
                        [月乃]
                        [0-9ー一。〇らg\s]+ # Day
                        [日巳曰]''', s, re.X)
    result = []
    if matches:
        for m in matches:
            if m:
                print u'Pos %d-%d: %s' % (m.start(), m.end(),
                                          s[m.start(): m.end()])
                result.append((m.start(), m.end()))
    return result


def extract_money(s):
    '''
    Extract money amounts from a unicode string

    @param s: A unicode string
    @return: A list of (start_pos, end_pos)
    '''
    matches = re.finditer(
        ur'([\\]*[0-9ー一。〇らg][0-9ー一。〇らg~\s]*円)|([\\]+[0-9ー一。〇らg][0-9ー一。〇らg~\s]*)',
        s)
    result = []
    if matches:
        for m in matches:
            print u'Pos %d-%d: %s' % (m.start(), m.end(),
                                      s[m.start(): m.end()])
            result.append((m.start(), m.end()))
    return result


if __name__ == '__main__':
    extract_date(u'abab \t 2015年一2月 2一日 haha')
    pass
