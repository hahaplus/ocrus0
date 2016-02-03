'''
Take the training images, and create an expanded set of
5x images, by displacing each training image up, down, left and
right, by one pixel.

Note that this program is memory intensive, and may not run on small
systems.

Copyright (C) 2016 Works Applications, all rights reserved
'''

from __future__ import print_function

# Standard library
import cPickle
import gzip
import os.path
import random
import sys

# Third-party libraries
import numpy as np

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('''Take the training images, and create an expanded set of 5x images

    Usage: %s path.pkl.gz

    path.pkl.gz
        Path of a file with the same format as mnist.pkl.gz''' %
              (os.path.basename(sys.argv[0])))
        sys.exit(0)

    path_pkl_gz = sys.argv[1]
    if not path_pkl_gz.endswith('.pkl.gz'):
        print('path.pkl.gz must ends with .pkl.gz')
        sys.exit(0)
    path_expanded = path_pkl_gz[:-len('.pkl.gz')] + '_expanded.pkl.gz'

    f = gzip.open(path_pkl_gz, 'rb')
    training_data, validation_data, test_data = cPickle.load(f)
    f.close()
    expanded_training_pairs = []
    j = 0  # counter
    for x, y in zip(training_data[0], training_data[1]):
        expanded_training_pairs.append((x, y))
        image = np.reshape(x, (-1, 28))
        j += 1
        if j % 1000 == 0:
            print("Expanding image number", j)
        # iterate over data telling us the details of how to
        # do the displacement
        for d, axis, index_position, index in [
                (1, 0, "first", 0),
                (-1, 0, "first", 27),
                (1, 1, "last", 0),
                (-1, 1, "last", 27)]:
            new_img = np.roll(image, d, axis)
            if index_position == "first":
                new_img[index, :] = np.zeros(28)
            else:
                new_img[:, index] = np.zeros(28)
            expanded_training_pairs.append((np.reshape(new_img, 784), y))
    random.shuffle(expanded_training_pairs)
    expanded_training_data = [list(d) for d in zip(*expanded_training_pairs)]
    print("Saving expanded data. This may take a few minutes.")
    f = gzip.open(path_expanded, "wb")
    cPickle.dump((expanded_training_data, validation_data, test_data), f)
    f.close()
