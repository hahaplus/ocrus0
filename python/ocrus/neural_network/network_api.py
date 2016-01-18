import network3
from network3 import *
import json
mini_batch_size = 1
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
    SoftmaxLayer(n_in=1000, n_out=3394, p_dropout=0.5)],

    mini_batch_size,
    path_params='/home/michael/workspace/ocrus0_build/networkModel/jpn_all2_param.pkl')

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


def recognition_img(*mat):
    '''
    Predict the y_out and probability for each y
    @param mat: the matrix of img in 1-d vector
    @return a tuple (y_out, prob_y1, prob_y2, ...)
    '''
    # print mat
    ret = net.predict_img_by_mat(mat)
    result = []
    result.append(ret[0])
    for prob in ret[1]:
        result.append(prob)

    #print (result[0], result[result[0] + 1])
    return (result[0], result[result[0] + 1])
    # else:
    #   return ("a", 1)
#f = gzip.open('/home/michael/workspace_python/japanese_traindata.pkl.gz', 'rb')
#training_data, validation_data, test_data = cPickle.load(f)
# f.close()
# print recognition_img(training_data[0][0])
