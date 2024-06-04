#!/users/donzhao/anaconda3/bin/python

#
# Created on Sun Mar 6 2022
#
# run_batch.py
# Copyright (C) 2022 dzhao@uw.edu
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import numpy as np
import subprocess as sp

# Control the repeation and faults
number_repeat = 3
ratio_max_fault = 0.8 #0.8
ratio_fault = 1 #0.2; Set it to 1 to disable any faulty nodes

# List of total numbers of miners
list_size_world = [1008] #[16, 32, 64, 128]

# Specify the number of transactions
list_number_txn = [100]  #[100, 200, 400, 800, 1600]

# List the protocols
list_protocol = ["PS", "WN", "TC"]  #["PS", "WN", "TC"]

# "Batch" sizes
size_min_simplex = 4 #4
number_min_simplex = 2 #4

# Print the header
print("[Miner-Chain-Txn-Fault]\t", end='', flush=True)
for protocol in list_protocol:
    for cnt in range(number_repeat):
        print("[" + protocol + "-" + str(1 + cnt) + "]\t", end='', flush=True)
print(flush=True)

command = "/nfs/fl_blockchain_nfs/src/lib_xbft/src/run_single.sh"  # The main executable
for size_world in list_size_world:
    command_1 = command + " " + str(size_world)

    # Construct the number of chains
    list_number_simplex = []
    tmp_size = int(size_world / size_min_simplex)
    while (tmp_size >= number_min_simplex):
        list_number_simplex.insert(0, tmp_size)
        tmp_size >>= 1

    # We can override the list of numbers of simplices
    list_number_simplex = [4, 16, 48, 144]

    for number_simplex in list_number_simplex:
        command_2 = command_1 + " " + str(number_simplex)

        for number_txn in list_number_txn:
            command_3 = command_2 + " " + str(number_txn)

            # number_max_fault = min(int(size_world / number_simplex), number_txn)
            # Allowing for repeated failures
            number_max_fault = number_txn
            for number_faults in range(
                    0,
                    1 + int(number_max_fault * ratio_max_fault),
                    int(number_max_fault * ratio_fault)):
                command_4 = command_3 + " " + str(number_faults)

                exp_id = str(size_world) + "-" + str(number_simplex) \
                        + "-" + str(number_txn) + "-" + str(number_faults)
                print(exp_id + "\t\t", end='', flush=True)

                for protocol in list_protocol:
                    command_5 = command_4 + " " + str(protocol) + " 0"

                    for cnt in range(number_repeat):
                        res = sp.getoutput(command_5)
                        print(res + "\t", end='', flush=True)

                print(flush=True)
