'''
The geometric calculation for various structures

Copyright (C) 2015 Works Applications, all rights reserved
'''

from convert_format import rect_to_ltrb


def bbox_area(bbox):
    '''
    Return the area of the bounding box

    @param bbox: A bounding box
    @return: The area
    '''

    '''
    x1,y1
    -------------
    |           |
    |           |
    |           |
    ------------x2,y2
    '''
    x1, y1, x2, y2 = bbox[0], bbox[1], bbox[2], bbox[3]
    return (x2 - x1) * (y2 - y1)


def bbox_almost_overlap(bbox1, bbox2, percent):
    '''
    Return True is two bounding box almost overlap, otherwise False
    Almost overlap means the overlapping area / (bbox1 + bbox2) > percent

    @param bbox1: Bounding box 1
    @param bbox2: Bounding box 2
    @return: A bool value
    '''

    '''
    x1,y1
    -------------
    |  x3,y3    |
    |    -------------------
    |    |      |          |
    ------------x2,y2      |
         |                 |
         -------------------x4,y4
    '''
    x1, y1, x2, y2 = bbox1[0], bbox1[1], bbox1[2], bbox1[3]
    x3, y3, x4, y4 = bbox2[0], bbox2[1], bbox2[2], bbox2[3]
    bbox_overlap = max(x1, x3), max(y1, y3), min(x2, x4), min(y2, y4)
    return float(bbox_area(bbox_overlap)) / (bbox_area(bbox1) +
                                             bbox_area(bbox2)) > percent


def rect2_in_rect1(rect1, rect2):
    '''
    Return True if rect1 != rect2 and rect2 is in rect1
       (r[0], r[1])
             ____________
            |            |
            |   ____     |
            |  |    |    |
            |  |____|    |
            |            |
            |____________|
               (r[0]+r[2], r[1]+r[3])

    @param rect1: Rect1
    @param rect2: Rect2
    @return: A bool value
    '''
    return (rect1 != rect2 and
            (rect1[0] <= rect2[0] and rect1[1] <= rect2[1] and
             rect1[0] + rect1[2] >= rect2[0] + rect2[2] and
             rect1[1] + rect1[3] >= rect2[1] + rect2[3]))


def overlap_ratio(start1, end1, start2, end2):
    '''
    Calculate overlapping ratio. Formula: overlap * 2 / (h1 + h2)
    All of the parameters are inclusive.

    start1                               start2

               start2   |   start1
                        |
                        |
    end1                |                end2

               end2         end1

    @param start1: Start1
    @param end1: End1
    @param start2: Start2
    @param end2: End2
    @return: The ratio
    '''
    if start2 > start1:
        overlap = end1 - start2 + 1
    else:
        overlap = end2 - start1 + 1

    overlap = 0 if overlap < 0 else overlap
    len1, len2 = end1 - start1 + 1, end2 - start2 + 1
    if len1 + len2 == 0:
        return 0.0
    else:
        return float(overlap) * 2 / (len1 + len2)


def overlap_ratio_in_second(start1, end1, start2, end2):
    '''
    Calculate overlapping ratio. Formula: overlap / len2.
    All of the parameters are inclusive.

    start1                               start2

               start2   |   start1
                        |
                        |
    end1                |                end2

               end2         end1

    @param start1: Top y1
    @param end1: Bottom y1
    @param start2: Top y2
    @param end2: Bottom y2
    @return: The ratio
    '''
    if start2 > start1:
        overlap = end1 - start2 + 1
    else:
        overlap = end2 - start1 + 1

    overlap = 0 if overlap < 0 else overlap
    len2 = end2 - start2 + 1
    if len2 == 0:
        return 0.0
    else:
        return float(overlap) / len2


def rects_dist(rect1, rect2):
    '''
    Compute distance between rect1 and rect2.
    Negative return value means they are overlapping.

    r1[0],r1[1]
    --------------
    |            |
    |            |
    |            |
    |            |
    |            |
    -------------- r1[0]+r1[2],r1[1]+r1[3]

    @param rect1: Rect1
    @param rect2: Rect2
    @return: Distance
    '''
    left1, top1, right1, bot1 = rect_to_ltrb(rect1)
    left2, top2, right2, bot2 = rect_to_ltrb(rect2)
    d_list = top1 - bot2, left1 - right2, top2 - bot1, left2 - right1

    return max(d_list)


def rects_dist_x(rect1, rect2):
    '''
    Compute the distance between rect1 and rect2 along x-axis

    left1--------right1
                            left2----------right2

    @param rect1: Rect1
    @param rect2: Rect2
    @return: Distance along x-axis
    '''
    left1, _, right1, _ = rect_to_ltrb(rect1)
    left2, _, right2, _ = rect_to_ltrb(rect2)
    d_list = left1 - right2, left2 - right1

    return max(d_list)


def rects_overlap_ratio_x(rect1, rect2):
    '''
    Compute the overlap ratio of two rects along x-axis

    @param rect1: Rect1
    @param rect2: Rect2
    @return: Overlap ratio along x-axis
    '''
    dist = rects_dist_x(rect1, rect2)
    w1, w2 = rect1[2], rect2[2]
    if w1 + w2 <= 0:
        return 0.0
    else:
        return float(dist) / (w1 + w2)


def rects_near(rect1, rect2, pixel_near):
    '''
    Determine whether two rects are near by a threshold

    @param rect1: Rect1
    @param rect2: Rect2
    @return: A bool value
    '''
    return rects_dist(rect1, rect2) < pixel_near


def rects_overlap_in_x(rect1, rect2, ratio_overlap_x):
    '''
    Determine whether two rects are overlapping along x-axis by a threshold

    @param rect1: Rect1
    @param rect2: Rect2
    @param ratio_overlap_x: Ratio threshold
    @return: A bool value
    '''

    left1, _, right1, _ = rect_to_ltrb(rect1)
    left2, _, right2, _ = rect_to_ltrb(rect2)
    return (
        overlap_ratio_in_second(
            left1, right1, left2, right2) > ratio_overlap_x or
        overlap_ratio_in_second(
            left2, right2, left1, right1) > ratio_overlap_x)


def rect_after_merge(rect1, rect2):
    '''
    Return a merged rect of two rects

    @param rect1: Rect1
    @param rect2: Rect2
    @return: A merged rect
    '''
    left1, top1, right1, bot1 = rect_to_ltrb(rect1)
    left2, top2, right2, bot2 = rect_to_ltrb(rect2)
    left, top = min(left1, left2), min(top1, top2)
    right, bot = max(right1, right2), max(bot1, bot2)
    rect = left, top, right - left, bot - top
    return rect
