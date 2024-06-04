#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Python version: 3.6

from distutils.log import debug
import inspect
from threading import local
from cassandra.cluster import Cluster
from mpi4py import MPI

import sys
import os
import copy
import pathlib
import time
import pickle
import numpy as np
from tqdm import tqdm

import torch
from tensorboardX import SummaryWriter

from lib_fl.options import args_parser
from lib_fl.update import LocalUpdate, test_inference
from lib_fl.models import MLP, CNNMnist, CNNFashion_Mnist, CNNCifar
from lib_fl.utils import get_dataset, average_weights, exp_details

## HPDIC
from random import randrange
import datetime
import tenseal as ts
import hashlib

#####################
## Inc HE function ##
#####################

def hello_mpi():
    size = MPI.COMM_WORLD.Get_size()
    rank = MPI.COMM_WORLD.Get_rank()
    name = MPI.Get_processor_name()

    sys.stdout.write(
        "Hello, World! I am process %d of %d on %s.\n"
        % (rank, size, name))


def hello_cass():
    cluster = Cluster()
    session = cluster.connect('hpdic')
    rows = session.execute('SELECT * FROM student')
    for r in rows:
        print(r.name, r.id)

    cluster.shutdown()


def incHE(pt, cache):

    # Binary additive randomness
    idx_rnd = randrange(2)
    if idx_rnd == 1:
        # cache[-1] = cache[-1].add(cache[-1])
        cache[-1] += [idx_rnd]
        pass

    # Binary multiplicative randomness
    # idx_rnd = randrange(2)
    # cache[-1] = cache[-1] * [idx_rnd]

    ct_inc = cache[-1]
    idx_cur = 0
    while pt > 0:
        if 1 == pt % 2:
            ct_inc = ct_inc.add(cache[idx_cur])
            pass
        idx_cur += 1
        pt >>= 1
        pass
    pass

    return ct_inc

def he(userID, w):
    
    for idx_layer in range(2):
        if 0 == idx_layer:
            if args.model == 'cnn':
                w_name = 'conv1'
            elif args.model == 'mlp':
                w_name = 'layer_input'
        elif 1 == idx_layer:
            if args.model == 'cnn':
                w_name = 'conv2'
            elif args.model == 'mlp':
                w_name = 'layer_hidden'

        # print("===== HPDIC:", w[w_name+'.weight'].shape)

        w_input = w[w_name+'.weight'].numpy().flatten()
        cnt_elem = 0
        for elem in w_input:
            cnt_elem += 1
            # if (cnt_elem % 1000 == 0):
            #     print("\tPlaintext ID: %d" % cnt_elem)
            elem = int(abs(elem * 10000))
            if enable_hec:  # Inc HE
                ct = incHE(elem, cache)
            else:  # Vanilla HE
                ct = ts.ckks_vector(context, [elem])
        pass

# Return the number of leading duplicates for array input_ary with indices input_arg
def leading_dup(input_arg, input_ary):
    first_val = input_ary[input_arg[0]]
    res = 1
    for idx in (1, len(input_arg)):
        if first_val == input_ary[input_arg[res]]:
            res += 1
        else:
            break

    return res

# Return the gague from the training data set
def gauge_construct(dataset_input):

    rank_mpi = MPI.COMM_WORLD.Get_rank()
    # if 0 == rank_mpi:
    #     print("================\nWorking on dataset %s with model %s."
    #             % (inspect.stack()[0][3], args.model))
    #     print(type(dataset_input.data))
    #     print(dataset_input.data.shape)

    tbl = dataset_input.data
    args = args_parser()
    if args.dataset != 'cifar':
        tbl = tbl.numpy()
 
    n_row = len(tbl)

    # Default reference column
    res = range(len(tbl[0].flatten()))
    lead_dup_res = 0

    # Heuristic algorithm to optimize the compaction rate
    for row in range(n_row):
    
        img = tbl[row]
        ary = img.flatten()
        my_gauge = np.argsort(ary)
        
        lead_dup_my_gauge = leading_dup(my_gauge, ary)
        if lead_dup_my_gauge > lead_dup_res:
            res = my_gauge    
            lead_dup_res = lead_dup_my_gauge

    return res

if __name__ == '__main__':

    # define paths
    # path_project = os.path.abspath('..')
    # fpath = pathlib.Path(__file__).parent.resolve()
    # logger = SummaryWriter(str(fpath) + '/../logs/rank' + str(rank))
    mpi_overhead = 0
    
    mpi_rank = MPI.COMM_WORLD.Get_rank()
    logger = SummaryWriter('/tmp/fl_blockchain/logs/rank' + str(mpi_rank))

    args = args_parser()
    # exp_details(args)

    if 0 == mpi_rank:
        #     print(np.shape(tbl))
        print('Working on dataset {0} and model {1} \n'
              .format(args.dataset, args.model))

    mpi_size = MPI.COMM_WORLD.Get_size()
    m = max(int(args.frac * args.num_users), 1)
    if 0 != m % mpi_size:
        if 0 == mpi_rank:
            print("Please specify the number of processes as divisible of %d." % m)
        exit()
    elif m != mpi_size:
        if 0 == mpi_rank:
            print("=======================================================================")
            print("We suggest the same number of MPI ranks as that of round-wise users: %d" % m)
            print("=======================================================================")

    # hello_mpi()
    # hello_cass()        
    
    # # Connect to Cassandra
    # cluster = Cluster()
    # session = cluster.connect('hpdic')
    # if 0 == mpi_rank:
    #     session.execute('TRUNCATE hpdic.flbc')

    ########################################################################
    ################## Homomorphic encryption caching ######################
    enable_hec = args.enable_hec
    enable_enc = False # Override HE

    if enable_enc:
        context = ts.context(
            ts.SCHEME_TYPE.CKKS,
            poly_modulus_degree=8192,
            coeff_mod_bit_sizes=[60, 40, 40, 60]
        )
        context.generate_galois_keys()
        context.global_scale = 2**40
        if enable_hec:
            cache = []
            LAMBDA = 30
            for idx in range(LAMBDA):
                cache.append(ts.ckks_vector(context, [np.power(2, idx)]))
            cache.append(ts.ckks_vector(context, [0]))
    #########################################################################

    if args.gpu is not None:
        torch.cuda.set_device(args.gpu)
    device = 'cuda' if args.gpu is not None else 'cpu'

    # load dataset and user groups, but only once...
    train_dataset, test_dataset, user_groups = get_dataset(args)

    global_start_time = time.time()
    ###############################################################################
    ############################ Sync the user_groups #############################
    # for i in range(args.num_users): # Fix the group for debug
    #     user_groups[i] = set(range(int(len(train_dataset) / args.num_users)))
    local_start_time = time.time()
    for key in user_groups:
        new_val = np.array(list(user_groups[key]))
        MPI.COMM_WORLD.Bcast(new_val)
        user_groups[key] = set(new_val)
    mpi_overhead += time.time() - local_start_time
    # cnt = 0
    # for iter in user_groups[0]:
    #     print("rank", mpi_rank, "; user_groups[0]", iter)
    #     cnt = cnt + 1
    #     if cnt > 0:
    #         break
    ################################################################################

    # BUILD MODEL
    if args.model == 'cnn':
        # Convolutional neural netork
        if args.dataset == 'mnist':
            global_model = CNNMnist(args=args)
        elif args.dataset == 'fmnist':
            # global_model = CNNFashion_Mnist(args=args)
            global_model = CNNMnist(args=args)
        elif args.dataset == 'cifar':
            global_model = CNNCifar(args=args)
        elif args.dataset == 'svhn':
            global_model = CNNCifar(args=args)
            # raise NotImplementedError()

    elif args.model == 'mlp':
        # Multi-layer preceptron
        img_size = train_dataset[0][0].shape
        len_in = 1
        for x in img_size:
            len_in *= x
            global_model = MLP(dim_in=len_in, dim_hidden=64,
                               dim_out=args.num_classes)
    else:
        exit('Error: unrecognized model')

    # Set the model to train and send it to device.
    global_model.to(device)
    global_model.train()
    # print(global_model)

    # copy weights
    global_weights = global_model.state_dict()

    local_start_time = time.time()
    #####################################################################################
    ########## THIS IS VERY IMPORTANT: ##################################################
    ########## global_weights are randomized, so we need to sync it across MPI ranks ####
    for key in global_weights:
        if args.gpu is None:
            new_val = global_weights[key].numpy()
        else: 
            new_val = global_weights[key].cpu().numpy()
        MPI.COMM_WORLD.Bcast(new_val)
        global_weights[key] = torch.from_numpy(new_val)
    # first_key = list(global_weights.keys())[0]
    # print(global_weights[first_key].numpy().flatten()[0])
    #####################################################################################
    mpi_overhead += time.time() - local_start_time

    # Training
    train_loss, train_accuracy = [], []
    val_acc_list, net_list = [], []
    cv_loss, cv_acc = [], []
    print_every = 2
    val_loss_pre, counter = 0, 0

    # Bad clients
    bad_clients = np.zeros(mpi_size)
    if 0 == mpi_rank:
        idx_bad_clients = np.random.choice(
            mpi_size,
            int(mpi_size * args.adversary),
            replace=False)
        bad_clients[idx_bad_clients] = 1
    MPI.COMM_WORLD.Bcast(bad_clients)
    if 0 == mpi_rank and 1 == args.debug:
        print(bad_clients)

    # for epoch in tqdm(range(args.epochs)):
    if 0 == mpi_rank:
        print('Round-wise accuracy')
        for i in range(args.epochs):
            print(f'#{i+1}', end='')
            print() if i == args.epochs - 1 else print(end='\t')

    insert_interval = 0
    query_interval = 0
    gausshash_interval = 0
    blockchain_interval = 0
    acc_epoch = np.zeros(args.epochs)
    loss_epoch = np.zeros(args.epochs)
    for epoch in range(args.epochs):

        # if 0 == mpi_rank:
        #     if 0 == epoch % 5:
        #         print("Global epoch = {0} / {1}".format(epoch, args.epochs))

        test_acc, test_loss = test_inference(args, global_model, test_dataset)

        if 0 == mpi_rank and 1 == args.debug:
            print('Global epoch = {0:2d} / {1}, timestamp (s) = {2:3d}, accuracy = {3:6.2%}'
                .format(
                    epoch, 
                    args.epochs,
                    int(time.time()-global_start_time),
                    test_acc
                ))
        acc_epoch[epoch] = test_acc
        loss_epoch[epoch] = test_loss

        if 0 == mpi_rank:
            print(f'{(100 * test_acc):4.1f}', end=',\t', flush=True)
            # if args.dataset in ['mnist', 'fmnist']: 
            #     print(f'{(100 * test_acc):4.1f}', end=',\t', flush=True)
            # elif args.dataset in ['cifar', 'svhn']:
            #     scale = max(loss_epoch) - min(loss_epoch)
            #     print(f'{(test_loss - min(loss_epoch))/scale:6.1f}', end=',\t', flush=True)
            # else:
            #     print("Unknown dataset!")

        if epoch == args.epochs - 1:
            break

        local_weights, local_losses = [], []
        # global_model.train() # DFZ: any usefulness?
        m = max(int(args.frac * args.num_users), 1)

        local_start_time = time.time()
        ##############################################################################
        ######### Need to make sure the randomness happens only on MPI rank0 #########
        idxs_users = np.random.choice(range(args.num_users), m, replace=False)
        # idxs_users = np.array(range(m)) # fix the user selection
        MPI.COMM_WORLD.Bcast(idxs_users)
        # print("rank", mpi_rank, "; idxs_users", idxs_users)
        ##############################################################################
        mpi_overhead += time.time() - local_start_time

        cnt_user = 0
        for idx in idxs_users:
            
            if mpi_rank == cnt_user % mpi_size:

                start_time = datetime.datetime.now()

                if 1 == bad_clients[mpi_rank] and 1 == args.debug:
                    dirty_items = np.array(list(user_groups[idx]))
                    # print("train_dataset[{0}][1] = {1}"
                    #       .format(dirty_items[0], train_dataset[dirty_items[0]][1]))
                    # for i in dirty_items: # DFZ: this doesn't work since training data is immutable
                    #     train_dataset[i][1] += 1 % 10 
                    # print("train_dataset[{}}][1] = {}"
                    #       .format(dirty_items[0], train_dataset[dirty_items[0]][1]))

                local_model = LocalUpdate(args=args, dataset=train_dataset,
                                        idxs=user_groups[idx], logger=logger)

                w, loss = local_model.update_weights(
                    model=copy.deepcopy(global_model), global_round=epoch, bad_nodes=bad_clients)

                if 'gauss' == args.attack_type:  # Guassian attack!
                    if 1 == bad_clients[mpi_rank]:
                        # print("type(w) = {}, w = {}".format(type(w), w))
                        for key, value in w.items():
                            # print(key, value.shape)
                            value -= args.attack_degree * torch.randn(value.shape) * args.gauss_var
                            w[key] = value
                elif 'labelflip' == args.attack_type:  # LabelFlip attack is implemented in './lib_fl/update.py'
                    pass
                elif 'backdoor' == args.attack_type:
                    # if epoch == args.epochs - 2:  # backdoor attack only happens in the last round
                    if 1 == bad_clients[mpi_rank]:
                        for key, value in w.items():
                            # print(key, value.shape)
                            value = value / args.adversary * torch.randn(value.shape) * 0.01
                            w[key] = value                            
                elif 'none' != args.attack_type:
                    if 0 == mpi_rank:
                        print("Unknown attack type.")
                    exit(0)
                
                # Turn on homomorphic encryption
                if enable_enc:
                    he(idx, w)

                # Hash local model
                gausshash_start = time.time()
                gausshash = ''
                for _, v in w.items():
                    gausshash = hashlib.sha256((str(v)+gausshash).encode()).hexdigest()
                gausshash_interval += time.time() - gausshash_start

                # Persist on blockchain
                blockchain_start = time.time()
                tensor_gausshash = list(map(int, list(str(int(gausshash, 16)))))  
                lst_gausshash = MPI.COMM_WORLD.allgather(tensor_gausshash)  # MPI broadcast
                if 0 == mpi_rank:
                    os.system('$HOME/system-0/src/lib_xbft/src/run_single.sh ' 
                            + str(mpi_size) + ' 1 1 0 WN 0')  # Blockchain consensus
                blockchain_interval += time.time() - blockchain_start
                

                local_weights.append(copy.deepcopy(w))
                local_losses.append(copy.deepcopy(loss))

                end_time = datetime.datetime.now()
                time_diff = (end_time - start_time)
                execution_time = time_diff.total_seconds()

                he_method = "vanilla HE"
                if enable_hec:
                    he_method = "caching HE"
                # print("Node %d (iid = %d) of %s with %s time (second): %.2f" %
                #     (idx, args.iid, args.model, he_method, execution_time))

            cnt_user += 1

        MPI.COMM_WORLD.barrier()

        #######################################################################################
        ################ MPI Aggregate  #######################################################
        mpi_global_weights = {}
        # tmp_weight = local_weights[0]  # There should be exactly one weight per MPI rank
        
        krum_score = np.zeros(mpi_size)
        idx_cutoff = mpi_size - int(mpi_size * args.adversary)
        for tmp_weight in local_weights:
            print_once = True
            
            for key in tmp_weight:
                if args.gpu is None:
                    src = tmp_weight[key].numpy()
                else:
                    src = tmp_weight[key].cpu().numpy()
                dst = np.zeros_like(src)
                allreduce_start_time = time.time()
                MPI.COMM_WORLD.Allreduce(src, dst) # Changed to Allreduce so that all clients have the latest global model
                mpi_overhead += time.time() - allreduce_start_time

                # FedAvg:
                dst = dst / mpi_size / len(local_weights)

                # DeFed:
                if 'defed' == args.aggr:
                    lst_weight = np.array(MPI.COMM_WORLD.allgather(src))
                    lst_weight /= len(local_weights)
                    # if 0 == mpi_rank and 1 == args.debug:
                    #     print("len(lst_weight) = ", len(lst_weight),
                    #         "; lst_weight[0].shape = ", lst_weight[0].shape,
                    #         "dst.shape = ", dst.shape)
                    for i in range(len(krum_score)):
                        krum_score[i] = np.linalg.norm(dst - lst_weight[i])
                    idx_sort = np.argsort(krum_score)
                    avg_krum = np.zeros_like(dst)

                    # DeFed will collect N/2 + 1 weights
                    defed_cutoff = mpi_size // 2 + 1
                    
                    for i in range(defed_cutoff):
                        avg_krum += lst_weight[idx_sort[i]]
                    dst = avg_krum / defed_cutoff
                elif 'multikrum' == args.aggr:
                    lst_weight = np.array(MPI.COMM_WORLD.allgather(src))
                    lst_weight /= len(local_weights)
                    # if 0 == mpi_rank and 1 == args.debug:
                    #     print("len(lst_weight) = ", len(lst_weight),
                    #         "; lst_weight[0].shape = ", lst_weight[0].shape,
                    #         "dst.shape = ", dst.shape)
                    for i in range(len(krum_score)):
                        krum_score[i] = np.linalg.norm(dst - lst_weight[i])
                    idx_sort = np.argsort(krum_score)
                    avg_krum = np.zeros_like(dst)
                    cutoff_mk = mpi_size // 2 + 1 # MultiKrum cutoff is 51%
                    for i in range(cutoff_mk):  
                        avg_krum += lst_weight[idx_sort[-i]]
                    dst = avg_krum / cutoff_mk                   
                elif 'fedavg' != args.aggr:
                    if 0 == mpi_rank:
                        print("Unknown aggregation algorithm.")
                    exit(0)

                if print_once:
                    # print("rank", mpi_rank, ": src0 =", src.flatten()[0])
                    # # print("rank", mpi_rank, ": dst0 =", dst.flatten()[0])
                    # print("type(dst) = ", type(dst), "; dst.shape = ", dst.shape)
                    # print_once = False
                    pass

                # exit(0)
                if key not in mpi_global_weights: # New item
                    try:
                        mpi_global_weights[key] = torch.from_numpy(dst)
                    except:
                        if print_once and 0 == mpi_rank:
                            print("Somethig went wrong the data")
                            print_once = False
                        pass
                else:  # Add more values
                    mpi_global_weights[key] = torch.add( 
                        mpi_global_weights[key], torch.from_numpy(dst))
        ########################################################################################        

        # update global weights
        # global_weights = average_weights(local_weights)
        global_weights = mpi_global_weights

        global_model.load_state_dict(global_weights)

        # #####################################
        # ### Persist models into Cassandra ###
        # insert_start_time = time.time()
        # # if 0 == mpi_rank:
        # new_id = 0
        # rows = session.execute(
        #     'select count(*) from hpdic.flbc;')
        # for r in rows:
        #     new_id = int(r.count) + 1
        # # print("new_id =", new_id)

        # str_global_weights = str(global_weights).replace("'", "\"")

        # # print('len(str_global_weights) =', len(str_global_weights))
        # cql_cmd = "insert into hpdic.flbc (batch_id, weights) values ("
        # cql_cmd += str(new_id) + ", '" + str_global_weights + "');"
        # session.execute(cql_cmd)
        #     # print('Working on dataset {0} and model {1}; Insertion time: {2:0.4f}'.format(
        #     #     args.dataset, args.model, time.time()-insert_start_time))
        #     # pass
        # insert_interval += time.time() - insert_start_time

        # #####################################
        # ### Retrieve model from Cassandra ###
        # insert_start_time = time.time()
        # rows = session.execute(
        #     'select count(*) from hpdic.flbc;')
        # for r in rows:
        #     new_id = int(r.count) + 1
        # expected_id = mpi_rank % new_id
        # session.execute('select * from hpdic.flbc where batch_id = ' + str(expected_id))
        # query_interval += time.time() - insert_start_time

    # ####################################################
    # ###################  Gauge  ########################
    # gague_start_time = time.time()
    # gague = gauge_construct(train_dataset)
    # gague_interval = time.time() - gague_start_time

    # Test inference after completion of training
    # test_acc, test_loss = test_inference(args, global_model, test_dataset)

    if 0 == mpi_rank:

        print('\nRound-wise loss')
        scale = max(loss_epoch) - min(loss_epoch)
        for val in loss_epoch:
            myloss = (val - min(loss_epoch)) / scale
            myloss = max(myloss, 0.01)
            print(f'{myloss:0.2f}', end=',\t', flush=True)

        print('\n\nNumber of Clients, MPI Time (s), GaussHash Time (s), Blockchain Time (s), Total Time (s)')
        print('{0}, {1:0.2f}, {2:0.2f}, {3:0.2f}, {4:0.2f}'
              .format(
                  args.num_users,
                  mpi_overhead,
                  max(0.01, gausshash_interval),
                  blockchain_interval,
                  time.time()-global_start_time,
              ))
        # print('Epoch-wise accuracy')
        # for i in range(args.epochs):
        #     print(f'{(100 * acc_epoch[i]):4.1f}', end='')
        #     print() if i == args.epochs - 1 else print(', ', end='')
        print('=========================================')

    # cluster.shutdown()

    MPI.Finalize()
