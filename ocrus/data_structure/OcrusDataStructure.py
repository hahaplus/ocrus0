
class Point:

    '''
        x,y is the coordinate in the image
        x,y is map the (row_index, clo_index)
    '''

    def __init__(self, x, y):
        self.x = x
        self.y = y


class OcrCandidate:
    text = ""
    confidence = 0

    def __init__(self, text, confi):
        self.text = text
        self.confidence = confi


class OcrChar:
    bounding_box = [Point(0, 0), Point(-1e6, -1e6)]
    text = ""
    confidence = 0
    candidates = []
    property = 0  # optional

    def __init__(self, boundingBox, text, confidence):
        self.boundingBox = boundingBox
        self.text = text
        self.confidence = confidence

    def getWidth(self):
        return self.bounding_box[1].x - self.bounding_box[0].x + 1

    def getHeight(self):
        return self.bounding_box[1].y - self.bounding_box[0].y + 1


class OcrLine:
    type = 0    # 0 is common, 1 is money, 2 is date
    chars = []  # the array of OcrChars
    money = 0
    date = {"year": 0, "month": 0, "day": 0}


class OcrusInternalResult:
    lines = []  # a array of OcrLine

    def convertToOcrResultDict(self):
        '''The format of ocrReulst in Dict is descriped in docs/
        ocr_result_format.txt'''
        ocrResultDict = []
        return ocrResultDict

    def loadFromOcrResultDict(self, ocr_result_dict):
        '''Load the dict into the OcrusInternalResult
        '''
        return
