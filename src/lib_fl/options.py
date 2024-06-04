#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Python version: 3.6

import argparse

def args_parser():
    parser = argparse.ArgumentParser()

    # Attack!
    parser.add_argument('--attack_type', type=str, default='none',
                        help="attack type ('none', 'gauss', 'labelflip', 'backdoor')")
    parser.add_argument('--attack_degree', type=float, default=1,
                    help="Gaussian: variance (=1);")
    parser.add_argument('--adversary', type=float, default=0.49,
                    help="Percent of adversary clients (0.49 for majority; 0.33 for Byzantine collusion)")
    parser.add_argument('--gauss_var', type=float, default=0.01,
                    help="Variance of Gaussian attack (0.01, 0.04, 0.16, 0.64)")                    

    # DEBUG
    parser.add_argument('--debug', type=int, default=0,
                        help="turn on debug messages with '1'")

    # MPI
    parser.add_argument('--n_cpu', type=int, default=40,
                        help="number of cores per node")

    # homomorphic encryption
    parser.add_argument('--enable_hec', type=int, default=0,
                        help="enable homomorhpic encryption caching")

    # federated arguments (Notation for the arguments followed from paper)
    parser.add_argument('--aggr', type=str, default="fedavg",
                        help="aggregation algorithm ('fedavg', 'multikrum', 'defed'")
    parser.add_argument('--epochs', type=int, default=10, # 10 default
                        help="number of rounds of training")
    # Warning: if --num_users is very large, training set will be too small to split
    parser.add_argument('--num_users', type=int, default=1000, 
                        help="number of users: K")
    parser.add_argument('--frac', type=float, default=0.1, # 0.1 default
                        help='the fraction of clients: C')
    parser.add_argument('--local_ep', type=int, default=20, # 10 default
                        help="the number of local epochs: E")
    parser.add_argument('--local_bs', type=int, default=100, # 10 default
                        help="local batch size: B")
    parser.add_argument('--lr', type=float, default=0.05, # 0.01 default
                        help='learning rate')
    parser.add_argument('--momentum', type=float, default=0.5,
                        help='SGD momentum (default: 0.5)')

    # model arguments
    parser.add_argument('--n_labels', type=int, default=10, help="number of labels")
    parser.add_argument('--model', type=str, default='mlp', help='model name: cnn or mlp')
    parser.add_argument('--kernel_num', type=int, default=9,
                        help='number of each kind of kernel')
    parser.add_argument('--kernel_sizes', type=str, default='3,4,5',
                        help='comma-separated kernel size to \
                        use for convolution')
    parser.add_argument('--num_channels', type=int, default=1, help="number \
                        of channels of imgs")
    parser.add_argument('--norm', type=str, default='batch_norm',
                        help="batch_norm, layer_norm, or None")
    parser.add_argument('--num_filters', type=int, default=32,
                        help="number of filters for conv nets -- 32 for \
                        mini-imagenet, 64 for omiglot.")
    parser.add_argument('--max_pool', type=str, default='True',
                        help="Whether use max pooling rather than \
                        strided convolutions")

    # other arguments
    parser.add_argument('--dataset', type=str, default='mnist', help="name \
                        of dataset: mnist, fmnist, cifar, or svhn")
    parser.add_argument('--num_classes', type=int, default=10, help="number \
                        of classes")
    parser.add_argument('--gpu', default=None, help="To use cuda, set \
                        to a specific GPU ID. Default set to use CPU.")
    parser.add_argument('--optimizer', type=str, default='sgd', help="type \
                        of optimizer")
    parser.add_argument('--iid', type=int, default=1,
                        help='Default set to IID. Set to 0 for non-IID.')
    parser.add_argument('--unequal', type=int, default=0,
                        help='whether to use unequal data splits for  \
                        non-i.i.d setting (use 0 for equal splits)')
    parser.add_argument('--stopping_rounds', type=int, default=10,
                        help='rounds of early stopping')
    parser.add_argument('--verbose', type=int, default=1, help='verbose')
    parser.add_argument('--seed', type=int, default=1, help='random seed')
    args = parser.parse_args()
    return args
