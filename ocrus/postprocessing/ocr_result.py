# coding=utf-8
'''
Post-processing for OCR result

Copyright (C) 2016 Works Applications, all rights reserved
'''

import io
import json
import re
import math

# Replace table for digits
DIGITS_REPLACE_REG = {
    u'。': u'0', u'〇': u'0', u'o': u'0', u'O': u'0',  # u'U': u'0', u'u': u'0',
    u'囗': u'0', u'D': u'0', u'ロ': u'0',
    # u'ー': u'1', u'一': u'1',
    u'ュ': u'1', u'エ': u'1', u'ェ': u'1', u'L': u'1', u'l': u'1', u'‡': u'1',
    u'ユ': u'1', u'工': u'1', u'I': u'1', u'i': u'1', u'\'': u'1',
    u'ら': u'5', u'ヲ': u'5', u's': u'5', u'S': u'5',
    u'フ': u'7',
    u'B': u'8',
    u'g': u'9',

    # For CNN
    u'Q': u'0', u'り': u'0', u'd': u'0',
    u'j': u'1', u'J': u'1', u'l': u'1', u'r': u'1',
    u'z': u'2', u'Z': u'2',
    u'ヨ': u'3',
    u'ゔ': u'5', u'う': u'5', u'守': u'5',
    u'e': u'6', u'b': u'6',
    u'プ': u'7',
    u'ミ': u'8', u'尽': u'8',
}

DIGITS_REPLACE = dict(
    DIGITS_REPLACE_REG.items() + {u']': u'1', u'[': u'1', }.items())

# These characters are in digit and are treated as blank
BLANK_REPLACE = {
    u' ': u' ', u'~': u' ', u',': u' ', u'"': u' ', u'〟': u' ', u'丶': u' ',
    u'_': u' ', u'・': u' ', u'`': u' ', u'_': u' ', u'〝': u' ',

    # For CNN
    u'・': u' ', u'，': u' ', u"'": u' ',
}

DIGITS_BLANK_REPLACE = dict(DIGITS_REPLACE.items() + BLANK_REPLACE.items())

# Replace table for money prefix
MONEY_PREFIX_REPLACE_REG = {
    u'¥': u'￥',
    u'半': u'￥', u'芋': u'￥', u'斐': u'￥', u'韮': u'￥', u'輩': u'￥',
    u'一': u'-', u'ー': u'-',

    # For CNN
    u'ヤ': u'￥', u'ギ': u'￥', u'Y': u'￥', u'y': u'￥', u'辛': u'￥',
    u'卒': u'￥', u'洋': u'￥', u'藥': u'￥', u'薬': u'￥',
}

MONEY_PREFIX_REPLACE = dict(
    MONEY_PREFIX_REPLACE_REG.items() + {u'\\': u'￥', }.items())

# Replace table for money suffix
MONEY_SUFFIX_REPLACE = {
    u'川': u'円',

    # For CNN
    u'丹': u'円', u'凹': u'円', u'内': u'円', u'H': u'円', u'm': u'円',
}

# Replace table for year symbol
YEAR_SYM_REPLACE = {
    u'午': u'年', u'什': u'年',
}

# Replace table for month symbol
MONTH_SYM_REPLACE = {
    u'乃': u'月', u'刀': u'月', u'且': u'月', u'ノ': u'/',
}

# Replace table for day symbol
DAY_SYM_REPLACE = {
    u'巳': u'日', u'E': u'日', u'臼': u'日', u'曰': u'日',
    u'口': u'日', u'囗': u'日', u'8': u'日', u'g': u'日',
    u'ロ': u'日', u'6': u'日', u'凵': u'日',

    # For CNN
    u'H': u'日', u'甘': u'日', u'臼': u'日', u'H': u'日', u'B': u'日',
    u'旦': u'日',
}

digit = ur'0-9\[\]' + u''.join(DIGITS_REPLACE_REG)
blank = ur'\s' + u''.join(BLANK_REPLACE)
digitblank = digit + blank

digit1 = u'[' + digit + u']'
blank1 = u'[' + blank + u']'
digitblank1 = u'[' + digitblank + u']'

digit0_1 = digit1 + u'?'

digit_star = digit1 + u'*'
blank_star = blank1 + u'*'
digitblank_star = digitblank1 + u'*'

digit_plus = digit1 + u'+'
blank_plus = blank1 + u'+'
digitblank_plus = digitblank1 + u'+'


def parse_line_v1(line):
    '''
    Parse the line and fill the dict (See docs/ocr_result_format.txt)
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


def normalize_ocr_lines(path_ocr_lines):
    '''
    Convert the rect in char to bounding_box and replace original file

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


def replace_if_exist(unicode_s, table):
    '''
    Replace ch in unicode_s by table[ch] if ch in table, otherwise keep ch

    @param unicode_s: A unicode string
    @param table: A replacement table
    @return: A replaced unicode string
    '''
    return u''.join([table[ch] if ch in table else ch for ch in unicode_s])


def extract_date(s):
    '''
    Extract dates from a unicode string

    @param s: A unicode string
    @return: A list of (start_pos, end_pos, replaced_s)
    '''

    reg_s = ur'(' + digitblank_plus + u')' + \
        u'([年/' + u''.join(YEAR_SYM_REPLACE) + u'])' + \
        u'(' + blank_star + digit1 + blank_star + \
            digit0_1 + blank_star + u')' + \
        u'([月/' + u''.join(MONTH_SYM_REPLACE) + u'])' + \
        u'(' + blank_star + digit1 + blank_star + \
            digit0_1 + blank_star + u')' + \
        u'([日' + u''.join(DAY_SYM_REPLACE) + u']?)'

    matches = re.finditer(reg_s, s, re.X)
    result = []
    if matches:
        for m in matches:
            if m:
                pos = 0
                replaced = []
                for i, g in enumerate(m.groups()):
                    if g:
                        if i == 0:
                            pos = len(g) - 1
                            count_digit = 0
                            while pos >= 0 and count_digit < 4:
                                if not g[pos] in BLANK_REPLACE:
                                    count_digit += 1
                                pos -= 1
                            pos += 1
                            replaced.append(
                                replace_if_exist(g[pos:],
                                                 DIGITS_BLANK_REPLACE))
                        elif i in [2, 4]:
                            replaced.append(
                                replace_if_exist(g, DIGITS_BLANK_REPLACE))
                        elif i == 1:
                            replaced.append(
                                replace_if_exist(g, YEAR_SYM_REPLACE))
                        elif i == 3:
                            replaced.append(
                                replace_if_exist(g, MONTH_SYM_REPLACE))
                        elif i == 5:
                            replaced.append(
                                replace_if_exist(g, DAY_SYM_REPLACE))

                print u'Pos %d-%d: %s, replaced as: %s' % (
                    m.start() + pos, m.end(), s[m.start() + pos: m.end()],
                    u''.join(replaced))
                result.append((m.start() + pos, m.end(), u''.join(replaced)))

    return result


def extract_money(s):
    '''
    Extract money amounts from a unicode string

    @param s: A unicode string
    @return: A list of (start_pos, end_pos, replaced_s)
    '''
    reg_s = ur'([ー一\-]?[\\￥' + u''.join(MONEY_PREFIX_REPLACE_REG) + u']?)' + \
        u'(' + blank_star + digit1 + digitblank_star + u')' + \
        u'([円' + u''.join(MONEY_SUFFIX_REPLACE) + u']?)'

    matches = re.finditer(reg_s, s)
    result = []
    if matches:
        for m in matches:
            if m.group(1) or m.group(3):
                replaced = []
                all_digits = True
                for i, g in enumerate(m.groups()):
                    if g:
                        if i == 0:
                            g_str = replace_if_exist(g, MONEY_PREFIX_REPLACE)
                            if u'￥' not in g_str:
                                pass
                            else:
                                all_digits = False
                            replaced.append(g_str)
                        elif i == 1:
                            replaced.append(
                                replace_if_exist(g, DIGITS_BLANK_REPLACE))
                        elif i == 2:
                            replaced.append(
                                replace_if_exist(g, MONEY_SUFFIX_REPLACE))
                            all_digits = False
                if not all_digits:
                    print u'Pos %d-%d: %s, replaced as: %s' % (
                        m.start(), m.end(), s[m.start():m.end()],
                        u''.join(replaced))
                    result.append((m.start(), m.end(), u''.join(replaced)))

    return result


def good_minus_symbol(bbox_minus, bbox_yen):
    '''
    Determine whether the bounding box of a minus symbol before a ￥ symbol
    is good enough

    @param bbox_minus: Bounding box of a minus symbol
    @param bbox_yen: Bounding box of ￥ symbol
    @return: A bool value
    '''
    w_yen = bbox_yen[2] - bbox_yen[0]
    w_minus = bbox_minus[2] - bbox_minus[0]
    top_yen, bot_yen = bbox_yen[1], bbox_yen[3]
    mid_yen = (top_yen + bot_yen) / 2

    top_minus, bot_minus = bbox_minus[1], bbox_minus[3]
    return (w_yen * 0.6 < w_minus < w_yen * 1.7 and
            mid_yen - w_yen / 3 <= top_minus and
            bot_minus <= mid_yen + w_yen / 3)


def to_ocr_lines(ocr_chars):
    '''
    Convert ocr_chars to ocr_lines plus some post-processing

    @param ocr_chars: OCR chars
    @return: OCR lines
    '''

    # Make sure each ch['text'] contain 1 character
    for ch in ocr_chars:
        if len(ch['text']) == 0:
            ch['text'] = ' '
        elif len(ch['text']) > 1:
            ch['text'] = ch['text'][0]

    text = u''.join(ocr_char['text'] for ocr_char in ocr_chars)

    ocr_lines = []

    pos_dates = extract_date(text)
    for start_pos, end_pos, replaced_s in pos_dates:
        ocr_line = {'type': 'date',
                    'chars': []}
        for pos in range(start_pos, end_pos):
            ocr_chars[pos]['text'] = replaced_s[pos - start_pos]
            ocr_line['chars'].append(ocr_chars[pos])
        ocr_lines.append(ocr_line)

    pos_moneys = extract_money(text)
    for start_pos, end_pos, replaced_s in pos_moneys:
        ocr_line = {'type': 'money',
                    'chars': []}
        for pos in range(start_pos, end_pos):
            ocr_chars[pos]['text'] = replaced_s[pos - start_pos]
            ocr_line['chars'].append(ocr_chars[pos])
        ocr_lines.append(ocr_line)

    # Remove whitespace characters
    for ocr_line in ocr_lines:
        ocr_line['chars'] = [
            ch for ch in ocr_line['chars'] if ch['text'].strip() != '']

    # Remove characters that are far away from key characters
    '''
    111    2015年 03 月 02日
    11     2015/03/1      1
    一     -￥2323         11
    1      3,456 円

             pos_head      pos_tail
    0           1   2    3    4               5
    c1          c2  c3   c4   c5              c6
          0        1   2    3          4
    '''
    for ocr_line in ocr_lines:
        dists = []
        centers = []
        for ch in ocr_line['chars']:
            bbox = map(float, ch['bounding_box'])
            center_x, center_y = (bbox[0] + bbox[2]) / 2, (bbox[1] + bbox[3])
            centers.append((center_x, center_y))
        if len(centers) >= 2:
            for pos in range(1, len(centers)):
                dists.append(math.sqrt(
                    (centers[pos][0] - centers[pos - 1][0])**2 +
                    (centers[pos][1] - centers[pos - 1][1])**2))

            pos_min = len(ocr_line['chars']) - 1
            pos_max = 0
            for pos_ch, ch in enumerate(ocr_line['chars']):
                if ch['text'] in [u'年', u'月', u'日', u'/', u'￥', u'円']:
                    pos_min = min(pos_min, pos_ch)
                    pos_max = max(pos_max, pos_ch)

            pos_tail = min(pos_max + 1, len(ocr_line['chars']) - 1)
            while pos_tail < len(ocr_line['chars']) - 1:
                normal_dist = float(sum(dists[pos_min:pos_tail])) / \
                    len(dists[pos_min:pos_tail])
                abnormal_dist = dists[pos_tail]
                if abnormal_dist > normal_dist * 2.5:
                    break
                pos_tail += 1

            pos_head = max(0, pos_min)
            pos_head = min(pos_head, pos_tail - 1)
            while pos_head > 0:
                normal_dist = float(sum(dists[pos_head:pos_tail])) / \
                    len(dists[pos_head:pos_tail])
                abnormal_dist = dists[pos_head - 1]
                if abnormal_dist > normal_dist * 2.5:
                    break
                pos_head -= 1

            ocr_chars = []
            for pos in range(pos_head, pos_tail + 1):
                ocr_chars.append(ocr_line['chars'][pos])
            if (len(ocr_chars) > 0 and ocr_chars[0]['text'] == u'￥' and
                    pos_head > 0 and
                    ocr_line['chars'][pos_head - 1]['text'] == u'-'):
                bbox_yen = ocr_chars[0]['bounding_box']
                bbox_minus = ocr_line['chars'][pos_head - 1]['bounding_box']
                if good_minus_symbol(bbox_minus, bbox_yen):
                    ocr_chars.insert(0, ocr_line['chars'][pos_head - 1])

            ocr_line['chars'] = ocr_chars

            print 'pos_head, pos_tail: %s, %s' % (pos_head, pos_tail)

    # Remove bad minus symbol
    for ocr_line in ocr_lines:
        ocr_chars = ocr_line['chars']
        if (len(ocr_chars) >= 2 and ocr_chars[0]['text'] == u'-' and
                ocr_chars[1]['text'] == u'￥'):
            bbox_minus = ocr_chars[0]['bounding_box']
            bbox_yen = ocr_chars[1]['bounding_box']
            if not good_minus_symbol(bbox_minus, bbox_yen):
                del ocr_line['chars'][0]

    return ocr_lines

if __name__ == '__main__':
    pass
