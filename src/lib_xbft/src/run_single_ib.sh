#!/usr/bin/bash

#
# Created on Sun Feb 13 2022
#
# run_single.sh
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

n_miners=16 # Default number (16) of miners; Already tested up to 1008
n_chains=4    # Default number (4) of chains ; Already tested up to 144
n_txns=100     # Default number (2) of transactions; Already tested up to 1000 
n_faults=0    # Default number (0) of faulty nodes; Already tested up to 15
protocol="TC" # Default protocol; possible values "PS" (pair swap), "WN" (witness network), and "TC" (topological commit)  
is_debug=0    # Default flag for debug (0)

if [[ $# -gt 0 ]]
then
  n_miners=$1
fi

if [[ $# -gt 1 ]]
then
  n_chains=$2
fi

if [[ $# -gt 2 ]]
then
  n_txns=$3
fi

if [[ $# -gt 3 ]]
then
  n_faults=$4
fi

if [[ $# -gt 4 ]]
then
  protocol=$5
fi

if [[ $# -gt 5 ]]
then
  is_debug=$6
fi

# printf "[INFO] %s [# miners=%d] [# chains=%d] [# of transactions=%d] [# faults=%d] [protocol=%s] [is_debug=%d]\n" \
# $0 $n_miners $n_chains $n_txns $protocol $n_faults $is_debug

mpiexec --use-hwthread-cpus --mca btl ^openib,sm,self --mca btl_base_warn_component_unused 0 --hostfile /hpdic/fl_blockchain/src/lib_xbft/src/hosts_cloudlab.txt -n $n_miners /hpdic/fl_blockchain_stale/xbft $n_chains $n_txns $n_faults $protocol $is_debug
