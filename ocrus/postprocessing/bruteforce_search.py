'''
Brute-force search in the OCR result

Copyright (C) 2015 Works Applications, all rights reserved
'''

import codecs
import sys

from ocrus.postprocessing.ocr_result import extract_date, extract_money
from ocrus.util.geometric import rect_after_merge
from ocrus.segmentation.row_analysis import recognize_bboxes


DEBUG_SINGLE_IMG = False


def bf_search_date_money(img_binary, bboxes, rows,
                         path_image, net, id_to_char):
    '''
    Still experimental
    '''

    '''
    How to fix broken char problem?
        (See iphone_IMG_0091.JPG for an example)
    Build a graph for searching date and money (it is too complex)

    1. Do a simple histogram split along x-axis (omitted now)
    2. Do merge
    Merge conditions:
    1. bboxes are near along x-axis, and merge will not greatly degrade prob
    2. bboxes are not near along x-axis, after-merge prob is above some value,
       and merge can improve prob
    '''

    '''
    cv2.namedWindow('char', cv2.WINDOW_NORMAL)
    for row in rows:
        pos_bboxes = row['pos_bboxes']
        if len(pos_bboxes) >= 7 and bboxes[pos_bboxes[6]]['ch'] == u'射':
            for pos_bbox in pos_bboxes:
                img = get_img_roi(img_binary, bboxes[pos_bbox]['rect'])
                cv2.imshow('char', img)
                # print img
                print bboxes[pos_bbox]['ch']
                print ' '.join(map(lambda x: '%.1f' % (img.shape[0] - (x / 255)), np.sum(img, 0)))
    '''

    '''
    Format of row:
    rows = [row, ...]
    row = {'pos_bboxes': [10, ...], }
    '''

    def rect_merged(rects):
        if len(rects) > 0:
            r = rects[0]
            for rect in rects[1:]:
                r = rect_after_merge(r, rect)
            return r
        return [0, 0, 0, 0]

    path_symbol = '{path_image}_symbol.txt'.format(path_image=path_image)
    f_symbol = codecs.open(path_symbol, 'w', 'utf-8')

    for row in rows:
        pos_bboxes = row['pos_bboxes']
        recognized_set = set()

        def search_in_row(pos_start, loc_chars):
            if pos_start < len(pos_bboxes):
                for pos_end in range(pos_start, len(pos_bboxes)):
                    if (pos_start, pos_end) not in recognized_set:
                        r = rect_merged(
                            [bboxes[pos_bboxes[pos]]['rect']
                             for pos in range(pos_start, pos_end + 1)])

                        if (pos_end - pos_start >= 1 and
                                float(r[2]) / r[3] >= 1.44):
                            break

                        recognized_set.add((pos_start, pos_end))
                    loc_chars.append((pos_start, pos_end))
                    search_in_row(pos_end + 1, loc_chars)
                    loc_chars.pop()

        search_in_row(0, [])

        # print recognized_set

        recognized_results = {}
        row_bboxes = []
        for pos_start, pos_end in recognized_set:
            if pos_end - pos_start >= 1:
                r = rect_merged([bboxes[pos_bboxes[pos]]['rect']
                                 for pos in range(pos_start, pos_end + 1)])
                row_bboxes.append({'rect': r, 'loc': (pos_start, pos_end)})
            else:
                bboxes[pos_bboxes[pos_start]]['loc'] = (pos_start, pos_end)
                row_bboxes.append(bboxes[pos_bboxes[pos_start]])

        recognize_bboxes(img_binary, row_bboxes, net, id_to_char)
        for row_bbox in row_bboxes:
            recognized_results[row_bbox['loc']] = row_bbox

        locs = sorted(recognized_results.keys())

        if DEBUG_SINGLE_IMG:
            for loc in locs:
                print loc, recognized_results[loc]['rect'],
                for ch, prob in recognized_results[loc]['top_k'][:10]:
                    print ch, '%.2f  ' % prob,
                print

        def result_has_good_char(result, chars_configs):
            for config in chars_configs:
                for ch, prob in result[:config['k']]:
                    if ch in config['chars'] and prob > config['prob']:
                        return True

            return False

        def search_by_config(pos_start, loc_chars, loc_chars_list,
                             chars_configs, result):
            if pos_start < len(pos_bboxes):

                next_is_searched = False
                for pos_end in range(pos_start, len(pos_bboxes)):
                    loc = (pos_start, pos_end)
                    if loc not in recognized_results:
                        break

                    has_good_char = result_has_good_char(
                        recognized_results[loc]['top_k'], chars_configs)

                    if has_good_char:
                        loc_chars.append(loc)
                        search_by_config(
                            pos_end + 1, loc_chars, loc_chars_list,
                            chars_configs, result)
                        loc_chars.pop()

                    if pos_end == pos_start and has_good_char:
                        next_is_searched = True

                if not next_is_searched:
                    loc_chars_list.append(loc_chars)
                    loc_chars = []
                    search_by_config(pos_start + 1, loc_chars, loc_chars_list,
                                     chars_configs, result)
                    loc_chars_list.pop()

            else:
                '''
                [ # loc_chars_list
                  [(0, 0), (1, 1)], # loc_chars
                  [],
                  ...
                ]
                '''
                loc_chars_list.append(loc_chars)
                if any(loc_chars_list):
                    list_tmp = [loc_chars_tmp[:]
                                for loc_chars_tmp in loc_chars_list
                                if loc_chars_tmp]
                    if list_tmp:
                        result.append(list_tmp)
                loc_chars_list.pop()

        def fetch_by_config(loc, chars_configs):
            return fetch_ch_prob_by_config(loc, chars_configs)[0]

        def fetch_ch_prob_by_config(loc, chars_configs):
            prob_max, ch_best = 0.0, None
            for config in chars_configs:
                for ch, prob in recognized_results[loc]['top_k'][:config['k']]:
                    if ch in config['chars'] and prob > config['prob']:
                        if prob > prob_max:
                            prob_max, ch_best = prob, ch
            return ch_best, prob_max

        def print_result_search(result_search, chars_configs):
            for loc_chars_list in result_search:
                for loc_chars in loc_chars_list:
                    for loc in loc_chars:
                        sys.stdout.write(fetch_by_config(loc, chars_configs))
                    sys.stdout.write(' ')
                print

                for loc_chars in loc_chars_list:
                    print '[',
                    for loc in loc_chars:
                        print fetch_by_config(loc, chars_configs), loc,
                    print '],',
                print

        def extract_to_file(result_search, chars_configs, func_extract,
                            f=sys.stdout):
            for loc_chars_list in result_search:
                for loc_chars in loc_chars_list:
                    ch_prob = [fetch_ch_prob_by_config(loc, chars_configs)
                               for loc in loc_chars]
                    line = ''.join([ch for ch, prob in ch_prob])
                    result = func_extract(line)
                    for start_pos, end_pos, replaced_s in result:
                        for pos in range(start_pos, end_pos):
                            loc = loc_chars[pos]
                            rect = recognized_results[loc]['rect']
                            ch, prob = ch_prob[pos]
                            line_symbol = "word: '%s';  \tconf: %.2f; bounding_box: %d,%d,%d,%d;\n" %\
                                (ch, prob * 100,
                                 rect[0], rect[1],
                                 rect[0] + rect[2], rect[1] + rect[3])
                            f.write(line_symbol)
                        line_symbol = "word: '#';  \tconf: 0.0; bounding_box: 0,0,0,0;\n"
                        f.write(line_symbol)
            f.flush()

        digits = set([unichr(ord(u'0') + i) for i in range(10)])
        digits.update(set([u'。', u'o', u'O', u'ヨ']))

        config1 = [{'chars': digits, 'k': 5, 'prob': 0.01},
                   {'chars': set([u'年', u'月']), 'k': 10,
                    'prob': 0.001},
                   {'chars': set([u'日']), 'k': 10,
                    'prob': 0.0},
                   {'chars': set([u'甘', u'臼', u'旦']), 'k': 5,
                    'prob': 0.01}]
        result_search1 = []
        search_by_config(0, [], [], config1, result_search1)
        extract_to_file(result_search1, config1, extract_date, f_symbol)

        if DEBUG_SINGLE_IMG:
            print_result_search(result_search1, config1)
            print

        config2 = [{'chars': digits, 'k': 5, 'prob': 0.01},
                   {'chars': set([u'-']), 'k': 5, 'prob': 0.01}, ]
        result_search2 = []
        search_by_config(0, [], [], config2, result_search2)
        extract_to_file(result_search2, config2, extract_date, f_symbol)

        if DEBUG_SINGLE_IMG:
            print_result_search(result_search2, config2)
            print

        config3 = [{'chars': digits, 'k': 5, 'prob': 0.01},
                   {'chars': set([u'/']), 'k': 5, 'prob': 0.01}, ]
        result_search3 = []
        search_by_config(0, [], [], config3, result_search3)
        extract_to_file(result_search3, config3, extract_date, f_symbol)

        if DEBUG_SINGLE_IMG:
            print_result_search(result_search3, config3)
            print

        config4 = [{'chars': digits, 'k': 5, 'prob': 0.01},
                   {'chars': set([u'-', u',', u'，']), 'k': 5, 'prob': 0.01},
                   {'chars': set([u'￥', u'円']), 'k': 10, 'prob': 0.001},
                   {'chars': set([u'Y', ]), 'k': 5, 'prob': 0.01}, ]
        result_search4 = []
        search_by_config(0, [], [], config4, result_search4)
        extract_to_file(result_search4, config4, extract_money, f_symbol)

        if DEBUG_SINGLE_IMG:
            print_result_search(result_search4, config4)
            print

        # print '-' * 79

        #raw_input('Press enter to continue')

    f_symbol.close()
