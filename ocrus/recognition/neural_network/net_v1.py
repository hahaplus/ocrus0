'''
Definition for network net_v1

Copyright (C) 2016 Works Applications, all rights reserved
'''

from network3 import ConvPoolLayer
from network3 import FullyConnectedLayer
from network3 import Network
from network3 import ReLU
from network3 import SoftmaxLayer

NET_ID = 'net_v1'


def create_network(path_params, mini_batch_size=1):
    '''
    Create a network
    @param path_params: Path of file for network parameters
    @param mini_batch_size: Number of images as a batch to the network
    @return: A network
    '''
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
        path_params)

    return net
