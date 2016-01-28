import network3
from network3 import *
import numpy
import json
mini_batch_size = 100
net = Network([
    ConvPoolLayer(image_shape=(mini_batch_size, 1, 28, 28),
                  filter_shape=(20, 1, 5, 5),
                  poolsize=(2, 2),
                  activation_fn=ReLU),
    ConvPoolLayer(image_shape=(mini_batch_size, 20, 12, 12),
                  filter_shape=(40, 20, 5, 5),
                  poolsize=(2, 2),
                  activation_fn=ReLU),
    FullyConnectedLayer(
        n_in=40 * 4 * 4, n_out=1000, activation_fn=ReLU, p_dropout=0.5),
    FullyConnectedLayer(
        n_in=1000, n_out=1000, activation_fn=ReLU, p_dropout=0.5),
    SoftmaxLayer(n_in=1000, n_out=3451, p_dropout=0.5)],

    mini_batch_size,
    path_params='/home/michael/workspace/ocrus0_build/networkModel/jpn_full_param.pkl')

# fp = open(
#    '/home/michael/workspace/ocrus0_build/networkModel/chars_id.json', 'r')
#jsondict = json.load(fp)
# fp.close()
#josn_map = {}
# for cha in jsondict:
#    josn_map[cha[1]] = cha[0]
# f = open(
#   "/home/michael/workspace/ocrus0_build/networkModel/chars_id.txt", "wr")
# for cha in jsondict:
#    f.write(str(cha[1]) + " " + cha[0].encode('utf-8') + "\n")
# f.close()

#   recogintion a single character img


def recognition_img(*mat):
    '''
    Predict the y_out and probability for each y
    @param mat: the matrix of img in 1-d vector
    @return a tuple (y_out, prob_y)
    '''
    # print mat
    ret = net.predict_img_by_mat(mat)
    result = []
    result.append(ret[0])
    for prob in ret[1]:
        result.append(prob)

    #print (result[0], result[result[0] + 1])
    return (result[0], result[result[0] + 1])


#   recogintion a  character img list


def recognition_img_list(*mat):
    '''
    recognition a list of character image
    It's faster than recognition a character img one by one
    Predict the y_out and probability for each y
    @param mat: the matrix of img in 1-d vector
    @return a tuple (y_out, prob_y, y_out2, prob_y2, ...)
    '''
    # print mat
    img_size = mat[-1]
    img = []
    img_list = []
    for i in range(0, len(mat) - 1):
        img.append(mat[i])
        if (len(img) == img_size):
            img_list.append(img)
            img = []
    img_list = np.asarray(img_list)
    # print img_list
    ret = net.predict_xs(img_list)
    result = []
    for i in range(0, len(ret[0])):
        result.append(ret[0][i])
        result.append(ret[1][i][ret[0][i]])

    #print (result[0], result[result[0] + 1])
    return tuple(result)
    # else:
    #   return ("a", 1)
#f = gzip.open('/home/michael/workspace_python/japanese_traindata.pkl.gz', 'rb')
#training_data, validation_data, test_data = cPickle.load(f)
# f.close()
# input = np.concatenate(
#    (training_data[0][0], training_data[0][34], training_data[0][2], numpy.array([28 * 28])))
# print recognition_img_list(input)
