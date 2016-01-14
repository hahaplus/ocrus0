'''
A Theano-based program for training and running simple neural
networks.

Copyright (C) 2016 Works Applications, all rights reserved

Adapted from
    https://github.com/mnielsen/neural-networks-and-deep-learning.git
    commit 86b38f5ba4d402b60167a82d638616f5e16f2f51

---
Below is the original comment

network3.py
~~~~~~~~~~~~~~

A Theano-based program for training and running simple neural
networks.

Supports several layer types (fully connected, convolutional, max
pooling, softmax), and activation functions (sigmoid, tanh, and
rectified linear units, with more easily added).

When run on a CPU, this program is much faster than network.py and
network2.py.  However, unlike network.py and network2.py it can also
be run on a GPU, which makes it faster still.

Because the code is based on Theano, the code is different in many
ways from network.py and network2.py.  However, where possible I have
tried to maintain consistency with the earlier programs.  In
particular, the API is similar to network2.py.  Note that I have
focused on making the code simple, easily readable, and easily
modifiable.  It is not optimized, and omits many desirable features.

This program incorporates ideas from the Theano documentation on
convolutional neural nets (notably,
http://deeplearning.net/tutorial/lenet.html ), from Misha Denil's
implementation of dropout (https://github.com/mdenil/dropout ), and
from Chris Olah (http://colah.github.io ).
'''

# Standard library
import cPickle
import gzip

# Third-party libraries
import cv2
import numpy as np
import theano
import theano.tensor as T
from theano.tensor.nnet import conv
from theano.tensor.nnet import softmax
from theano.tensor import shared_randomstreams
from theano.tensor.signal import downsample
from theano.tensor.nnet import sigmoid
# from theano.tensor import tanh

from ocrus.neural_network.mnist_format import gray_img_to_mnist_array
from ocrus.neural_network.mnist_format import remove_white_border


# Constants
GPU = False


def linear(z):
    '''
    Linear activation function
    '''
    return z


def ReLU(z):
    '''
    Rectified linear activation function
    '''
    return T.maximum(0.0, z)


if GPU:
    print '''Trying to run under a GPU. If this is not desired, then modify
network3.py to set the GPU flag to False.'''
    try:
        theano.config.device = 'gpu'
    except:
        pass  # it's already set
    theano.config.floatX = 'float32'
else:
    print '''Running with a CPU. If this is not desired, then modify
network3.py to set the GPU flag to True.'''


def load_data_shared(filename):
    '''
    Load the dataset
    @param filename: path of a file with mnist format
    '''
    f = gzip.open(filename, 'rb')
    training_data, validation_data, test_data = cPickle.load(f)
    f.close()

    def shared(data):
        '''
        Place the data into shared variables.  This allows Theano to copy
        the data to the GPU, if one is available.
        @param data: ((imgs, ...), (1, ...)) in mnist format
        '''
        shared_x = theano.shared(
            np.asarray(data[0], dtype=theano.config.floatX), borrow=True)
        shared_y = theano.shared(
            np.asarray(data[1], dtype=theano.config.floatX), borrow=True)
        return shared_x, T.cast(shared_y, "int32")

    return [shared(training_data), shared(validation_data), shared(test_data)]


class Network(object):
    '''
    Main class used to construct and train networks
    '''

    def __init__(self, layers, mini_batch_size, path_params=None):
        '''
        Takes a list of `layers`, describing the network architecture, and
        a value for the `mini_batch_size` to be used during training
        by stochastic gradient descent.
        @param layers: A list of layers
        @param mini_batch_size: Used in stochastic gradient descent
        '''
        self.layers = layers

        if path_params:
            params = cPickle.load(open(path_params, 'rb'))
            for i, layer in enumerate(self.layers):
                layer.w = params[i][0]
                layer.b = params[i][1]
                layer.params = [layer.w, layer.b]

        self.mini_batch_size = mini_batch_size
        self.params = [param
                       for layer in self.layers
                       for param in layer.params]
        self.x = T.matrix("x")
        self.y = T.ivector("y")

        self.layers[0].set_inpt(self.x, self.x, self.mini_batch_size)

        for j in xrange(1, len(self.layers)):
            prev_layer, layer = self.layers[j - 1], self.layers[j]
            layer.set_inpt(
                prev_layer.output, prev_layer.output_dropout,
                self.mini_batch_size)

        self.output = self.layers[-1].output
        self.output_dropout = self.layers[-1].output_dropout

    def __load_params(self, path_params):
        '''
        Load the params w and b into the network
        Please load parameters before any real work
        @param path_params: Path to the params file
        '''
        params = cPickle.load(open(path_params, 'rb'))
        for i, layer in enumerate(self.layers):
            layer.w = params[i][0]
            layer.b = params[i][1]
            layer.params = [layer.w, layer.b]

    def save_params(self, path_params):
        '''
        Save the params w and b to file
        @param path_params: Path to the params file
        '''
        params = [layer.params for layer in self.layers]
        cPickle.dump(params, open(path_params, 'wb'))

    def accuracy(self, shared_test_data):
        '''
        Accuracy of the by giving a shared_test_data
        @param shared_test_data: The test data wrapped in shared
        @return: The accuracy
        '''
        num_test_batches = size(shared_test_data) / self.mini_batch_size
        test_x, test_y = shared_test_data
        i = T.lscalar()
        test_mb_accuracy = theano.function(
            [i], self.layers[-1].accuracy(self.y),
            givens={
                self.x:
                test_x[
                    i * self.mini_batch_size:
                    (i + 1) * self.mini_batch_size],
                self.y:
                test_y[
                    i * self.mini_batch_size:
                    (i + 1) * self.mini_batch_size]
            })
        test_accuracy = np.mean(
            [test_mb_accuracy(j)
             for j in xrange(num_test_batches)])
        return test_accuracy

    def predict_x(self, single_x):
        '''
        Predict the y_out and probability for each y
        @param single_x: A single x (A mnist float img)
        @return: (y_out, (prob_y1, prob_y2, ...))
        '''
        data_x = np.empty((self.mini_batch_size, 784), np.float32)
        data_x.fill(0)
        data_x[0] = single_x
        shared_x = theano.shared(
            np.asarray(data_x, dtype=theano.config.floatX), borrow=True)
        f_y_out = theano.function(
            [], self.layers[-1].y_out,
            givens={
                self.x:
                shared_x
            })
        f_prob = theano.function(
            [], self.layers[-1].output,
            givens={
                self.x:
                shared_x
            })
        return f_y_out()[0], f_prob()[0]

    def predict_xs(self, xs):
        '''
        Predcit the ys_out, plural version of predict_x
        @param xs: An array of x (x is a mnist float img)
        @return: ((y_out,...), ((prob_y1, ...), ...))
        '''

        '''
        len: 01-10:  1  [10-19]
        len: 11-20:  2  [20-29]
        ...
        '''
        len_xs = xs.shape[0]
        num_batch = (len_xs + self.mini_batch_size - 1) / self.mini_batch_size
        print 'num_batch:', num_batch
        data_x = np.empty((num_batch * self.mini_batch_size, 784), np.float32)
        data_x.fill(0)
        data_x[:xs.shape[0]] = xs
        test_x = theano.shared(
            np.asarray(data_x, dtype=theano.config.floatX), borrow=True)

        i = T.lscalar()
        f_y_out = theano.function(
            [i], self.layers[-1].y_out,
            givens={
                self.x:
                test_x[
                    i * self.mini_batch_size:
                    (i + 1) * self.mini_batch_size],
            })
        f_prob = theano.function(
            [i], self.layers[-1].output,
            givens={
                self.x:
                test_x[
                    i * self.mini_batch_size:
                    (i + 1) * self.mini_batch_size],
            })

        y_out_all = []
        prob_all = []
        for i in range(num_batch):
            for y_out in f_y_out(i):
                y_out_all.append(y_out)
            for prob_y in f_prob(i):
                prob_all.append(prob_y)

        return y_out_all[:len_xs], prob_all[:len_xs]

    def predict_img(self, path_binary_img):
        '''
        Predict the y_out and probability for each y
        @param path_binary_img: Path to a binary img (0 for foreground)
        @return (y_out, (prob_y1, prob_y2, ...))
        '''
        binary_img = cv2.imread(path_binary_img, cv2.IMREAD_GRAYSCALE)
        binary_img = remove_white_border(binary_img)
        return self.predict_x(gray_img_to_mnist_array(binary_img))
    # add by chenyuanqin

    def predict_img_by_mat(self, img):
        '''
        Predict the y_out and probability for each y
        @param img: the matrix of img in 1-d vector
        @return (y_out, (prob_y1, prob_y2, ...))
        '''

    def SGD(self, training_data, epochs, mini_batch_size, eta,
            validation_data, test_data, lmbda=0.0):
        '''
        Train the network using mini-batch stochastic gradient descent.
        @param training_data: Training data, should wrapped in shared
        @param epochs: Number of rounds to run
        @param mini_batch_size: Used in stochastic gradient descent
        @param eta: Speed in SGD
        @param validation_data: Validation data, should wrapped in shared
        @param test_data: Test data, should wrapped in shared
        @param lmbda: Parameter in regularization term
        '''
        training_x, training_y = training_data
        validation_x, validation_y = validation_data
        test_x, test_y = test_data

        # compute number of minibatches for training, validation and testing
        num_training_batches = size(training_data) / mini_batch_size
        num_validation_batches = size(validation_data) / mini_batch_size
        num_test_batches = size(test_data) / mini_batch_size

        # define the (regularized) cost function, symbolic gradients, and
        # updates
        l2_norm_squared = sum([(layer.w ** 2).sum() for layer in self.layers])

        # Why divided by num_training_batches instead of size(training_data)?
        cost = self.layers[-1].cost(self) + \
            0.5 * lmbda * l2_norm_squared / num_training_batches

        grads = T.grad(cost, self.params)

        updates = [(param, param - eta * grad)
                   for param, grad in zip(self.params, grads)]

        # define functions to train a mini-batch, and to compute the
        # accuracy in validation and test mini-batches.
        i = T.lscalar()  # mini-batch index
        train_mb = theano.function(
            [i], cost, updates=updates,
            givens={
                self.x:
                training_x[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size],
                self.y:
                training_y[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size]
            })
        validate_mb_accuracy = theano.function(
            [i], self.layers[-1].accuracy(self.y),
            givens={
                self.x:
                validation_x[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size],
                self.y:
                validation_y[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size]
            })
        test_mb_accuracy = theano.function(
            [i], self.layers[-1].accuracy(self.y),
            givens={
                self.x:
                test_x[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size],
                self.y:
                test_y[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size]
            })
        self.test_mb_predictions = theano.function(
            [i], self.layers[-1].y_out,
            givens={
                self.x:
                test_x[
                    i * self.mini_batch_size: (i + 1) * self.mini_batch_size]
            })
        # Do the actual training
        best_validation_accuracy = 0.0
        for epoch in xrange(epochs):
            for minibatch_index in xrange(num_training_batches):
                iteration = num_training_batches * epoch + minibatch_index
                if iteration % 1000 == 0:
                    print("Training mini-batch number {0}".format(iteration))
                cost_ij = train_mb(minibatch_index)
                if (iteration + 1) % num_training_batches == 0:
                    validation_accuracy = np.mean(
                        [validate_mb_accuracy(j)
                         for j in xrange(num_validation_batches)])
                    print("Epoch {0}: validation accuracy {1:.2%}".format(
                        epoch, validation_accuracy))
                    if validation_accuracy >= best_validation_accuracy:
                        print("This is the best validation accuracy to date.")
                        best_validation_accuracy = validation_accuracy
                        best_iteration = iteration
                        if test_data:
                            test_accuracy = np.mean(
                                [test_mb_accuracy(j)
                                 for j in xrange(num_test_batches)])
                            print(
                                'The corresponding test accuracy is {0:.2%}'.format(test_accuracy))
        print("Finished training network.")
        print(
            "Best validation accuracy of {0:.2%} obtained at iteration {1}".format(
                best_validation_accuracy,
                best_iteration))
        print("Corresponding test accuracy of {0:.2%}".format(test_accuracy))

# Define layer types


class ConvPoolLayer(object):
    """Used to create a combination of a convolutional and a max-pooling
    layer.  A more sophisticated implementation would separate the
    two, but for our purposes we'll always use them together, and it
    simplifies the code, so it makes sense to combine them.

    """

    def __init__(self, filter_shape, image_shape, poolsize=(2, 2),
                 activation_fn=sigmoid):
        """`filter_shape` is a tuple of length 4, whose entries are the number
        of filters, the number of input feature maps, the filter height, and
        the filter width.

        `image_shape` is a tuple of length 4, whose entries are the
        mini-batch size, the number of input feature maps, the image
        height, and the image width.

        `poolsize` is a tuple of length 2, whose entries are the y and
        x pooling sizes.

        """
        self.filter_shape = filter_shape
        self.image_shape = image_shape
        self.poolsize = poolsize
        self.activation_fn = activation_fn
        # initialize weights and biases
        n_out = (
            filter_shape[0] * np.prod(filter_shape[2:]) / np.prod(poolsize))
        self.w = theano.shared(
            np.asarray(
                np.random.normal(
                    loc=0, scale=np.sqrt(1.0 / n_out), size=filter_shape),
                dtype=theano.config.floatX),
            borrow=True)
        self.b = theano.shared(
            np.asarray(
                np.random.normal(loc=0, scale=1.0, size=(filter_shape[0],)),
                dtype=theano.config.floatX),
            borrow=True)
        self.params = [self.w, self.b]

    def set_inpt(self, inpt, inpt_dropout, mini_batch_size):
        self.inpt = inpt.reshape(self.image_shape)
        conv_out = conv.conv2d(
            input=self.inpt, filters=self.w, filter_shape=self.filter_shape,
            image_shape=self.image_shape)
        pooled_out = downsample.max_pool_2d(
            input=conv_out, ds=self.poolsize, ignore_border=True)
        self.output = self.activation_fn(
            pooled_out + self.b.dimshuffle('x', 0, 'x', 'x'))
        # no dropout in the convolutional layers
        self.output_dropout = self.output


class FullyConnectedLayer(object):

    def __init__(self, n_in, n_out, activation_fn=sigmoid, p_dropout=0.0):
        self.n_in = n_in
        self.n_out = n_out
        self.activation_fn = activation_fn
        self.p_dropout = p_dropout
        # Initialize weights and biases
        self.w = theano.shared(
            np.asarray(
                np.random.normal(
                    loc=0.0, scale=np.sqrt(1.0 / n_out), size=(n_in, n_out)),
                dtype=theano.config.floatX),
            name='w', borrow=True)
        self.b = theano.shared(
            np.asarray(np.random.normal(loc=0.0, scale=1.0, size=(n_out,)),
                       dtype=theano.config.floatX),
            name='b', borrow=True)
        self.params = [self.w, self.b]

    def set_inpt(self, inpt, inpt_dropout, mini_batch_size):
        self.inpt = inpt.reshape((mini_batch_size, self.n_in))
        self.output = self.activation_fn(
            (1 - self.p_dropout) * T.dot(self.inpt, self.w) + self.b)
        self.y_out = T.argmax(self.output, axis=1)
        self.inpt_dropout = dropout_layer(
            inpt_dropout.reshape((mini_batch_size, self.n_in)), self.p_dropout)
        self.output_dropout = self.activation_fn(
            T.dot(self.inpt_dropout, self.w) + self.b)

    def accuracy(self, y):
        "Return the accuracy for the mini-batch."
        return T.mean(T.eq(y, self.y_out))


class SoftmaxLayer(object):

    def __init__(self, n_in, n_out, p_dropout=0.0):
        self.n_in = n_in
        self.n_out = n_out
        self.p_dropout = p_dropout
        # Initialize weights and biases
        self.w = theano.shared(
            np.zeros((n_in, n_out), dtype=theano.config.floatX),
            name='w', borrow=True)
        self.b = theano.shared(
            np.zeros((n_out,), dtype=theano.config.floatX),
            name='b', borrow=True)
        self.params = [self.w, self.b]

    def set_inpt(self, inpt, inpt_dropout, mini_batch_size):
        self.inpt = inpt.reshape((mini_batch_size, self.n_in))
        self.output = softmax(
            (1 - self.p_dropout) * T.dot(self.inpt, self.w) + self.b)
        self.y_out = T.argmax(self.output, axis=1)
        self.inpt_dropout = dropout_layer(
            inpt_dropout.reshape((mini_batch_size, self.n_in)), self.p_dropout)
        self.output_dropout = softmax(
            T.dot(self.inpt_dropout, self.w) + self.b)

    def cost(self, net):
        "Return the log-likelihood cost."
        return -T.mean(T.log(self.output_dropout)[T.arange(net.y.shape[0]),
                                                  net.y])

    def accuracy(self, y):
        "Return the accuracy for the mini-batch."
        return T.mean(T.eq(y, self.y_out))


# Miscellanea
def size(data):
    "Return the size of the dataset `data`."
    return data[0].get_value(borrow=True).shape[0]


def dropout_layer(layer, p_dropout):
    srng = shared_randomstreams.RandomStreams(
        np.random.RandomState(0).randint(999999))
    mask = srng.binomial(n=1, p=1 - p_dropout, size=layer.shape)
    return layer * T.cast(mask, theano.config.floatX)
