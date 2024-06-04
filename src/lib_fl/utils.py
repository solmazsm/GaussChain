#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Python version: 3.6

from pathlib import Path
from time import sleep
import copy
from mpi4py import MPI
import torch
from torchvision import datasets, transforms
from lib_fl.sampling import mnist_iid, mnist_noniid, mnist_noniid_unequal
from lib_fl.sampling import cifar_iid, cifar_noniid


def get_dataset(args):
    """ Returns train and test datasets and a user group which is a dict where
    the keys are the user index and the values are the corresponding data for
    each of those users.
    """

    # On every physical node, only one rank download the data; others wait for 10 seconds
    # Or, simply let the error show up for the first time (data set is downloaded), then no need to wait
    # Or, manually download the data set
    wait_time = 0 #10 

    n_rank_per_node = args.n_cpu # Number of cores per physical node
    rank_mpi = MPI.COMM_WORLD.Get_rank()
    # size_mpi = MPI.COMM_WORLD.Get_size()
    # dir_home = str(Path.home())
    # n_node = sum(1 for line in open(dir_home + '/node_list.txt'))
    is_download = True if 0 == (rank_mpi % n_rank_per_node) else False

    if args.dataset == 'cifar':
        data_dir = '/tmp/data_fl_mpi/cifar'
        apply_transform = transforms.Compose(
            [transforms.ToTensor(),
             transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5))])

        # CIFAR dataset should have been downloaded manually
        train_dataset = datasets.CIFAR10(data_dir, train=True, download=False,
                                    transform=apply_transform)
        test_dataset = datasets.CIFAR10(data_dir, train=False, download=False,
                                    transform=apply_transform)                                           

        # sample training data amongst users
        if args.iid:
            # Sample IID user data from Mnist
            user_groups = cifar_iid(train_dataset, args.num_users)
        else:
            # Sample Non-IID user data from Mnist
            if args.unequal:
                # Chose uneuqal splits for every user
                raise NotImplementedError() # DFZ: LOL
            else:
                # Chose euqal splits for every user
                user_groups = cifar_noniid(train_dataset, args.num_users)

    elif args.dataset == 'mnist' or args.dataset == 'fmnist':
        if args.dataset == 'mnist':
            data_dir = '/tmp/data_fl_mpi/mnist'
        else:
            data_dir = '/tmp/data_fl_mpi/fmnist/'

        apply_transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize((0.1307,), (0.3081,))])

        # Only one rank will download the data for each physical node
        if is_download:
            if args.dataset == 'mnist':
                train_dataset = datasets.MNIST(data_dir, train=True, download=True,
                                            transform=apply_transform)

                test_dataset = datasets.MNIST(data_dir, train=False, download=True,
                                            transform=apply_transform)
            elif args.dataset == 'fmnist':
                train_dataset = datasets.FashionMNIST(data_dir, train=True, download=True,
                                                    transform=apply_transform)

                test_dataset = datasets.FashionMNIST(data_dir, train=False, download=True,
                                                    transform=apply_transform)
        MPI.COMM_WORLD.barrier()
        if not is_download:
            if args.dataset == 'mnist':
                train_dataset = datasets.MNIST(data_dir, train=True, download=False,
                                               transform=apply_transform)

                test_dataset = datasets.MNIST(data_dir, train=False, download=False,
                                              transform=apply_transform)
            elif args.dataset == 'fmnist':
                train_dataset = datasets.FashionMNIST(data_dir, train=True, download=False,
                                                      transform=apply_transform)

                test_dataset = datasets.FashionMNIST(data_dir, train=False, download=False,
                                                     transform=apply_transform)
        MPI.COMM_WORLD.barrier()

        # sample training data amongst users
        if args.iid:
            # Sample IID user data from Mnist
            user_groups = mnist_iid(train_dataset, args.num_users)
        else:
            # Sample Non-IID user data from Mnist
            if args.unequal:
                # Chose uneuqal splits for every user
                user_groups = mnist_noniid_unequal(train_dataset, args.num_users)
            else:
                # Chose euqal splits for every user
                user_groups = mnist_noniid(train_dataset, args.num_users)
    
    elif args.dataset == 'svhn':
        data_dir = '/tmp/data_fl_mpi/svhn'
        apply_transform = transforms.Compose(
            [transforms.ToTensor(),
             transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5))])

        # Only one rank will download the data for each physical node
        data_split = 'extra'  # change split='train' to split='extra' for 500+ clients
        if is_download:
            train_dataset = datasets.SVHN(data_dir, split=data_split, download=True,
                                          transform=apply_transform)
            test_dataset = datasets.SVHN(data_dir, split='test', download=True,
                                         transform=apply_transform)
        MPI.COMM_WORLD.barrier()
        if not is_download:
            train_dataset = datasets.SVHN(data_dir, split=data_split, download=False,
                                        transform=apply_transform)
            test_dataset = datasets.SVHN(data_dir, split='test', download=False,
                                        transform=apply_transform)
        MPI.COMM_WORLD.barrier()                                        

        # sample training data amongst users
        if args.iid:
            # Sample IID user data from Mnist
            user_groups = cifar_iid(train_dataset, args.num_users)
        else:
            # Sample Non-IID user data from Mnist
            if args.unequal:
                # Chose uneuqal splits for every user
                raise NotImplementedError()  # DFZ: LOL
            else:
                # Chose euqal splits for every user
                user_groups = cifar_noniid(train_dataset, args.num_users)
        pass

    return train_dataset, test_dataset, user_groups


def average_weights(w):
    """
    Returns the average of the weights.
    """
    w_avg = copy.deepcopy(w[0])
    for key in w_avg.keys():
        for i in range(1, len(w)):
            w_avg[key] += w[i][key]
        w_avg[key] = torch.div(w_avg[key], len(w))
    return w_avg


def exp_details(args):
    print('\nExperimental details:')
    print(f'    Model     : {args.model}')
    print(f'    Optimizer : {args.optimizer}')
    print(f'    Learning  : {args.lr}')
    print(f'    Global Rounds   : {args.epochs}\n')

    print('    Federated parameters:')
    if args.iid:
        print('    IID')
    else:
        print('    Non-IID')
    print(f'    Fraction of users  : {args.frac}')
    print(f'    Local Batch size   : {args.local_bs}')
    print(f'    Local Epochs       : {args.local_ep}\n')

    print('    Encryption details:')
    print(f'    Cache   : {args.enable_hec}\n')

    return
