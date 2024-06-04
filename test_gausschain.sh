#!/usr/bin/bash

# Update the following two arguments
n_core=100 # Max number of cores. Do NOT make it too large, see 'src/lib_fl/options.py'
dataset='mnist' # mnist, fmnist, or cifar
aggr='fedavg'
attack='none'
pct_attack=0.49
gauss_var=0.01
debug=0

if [[ $# -gt 0 ]]
then
  n_core=$1
fi

if [[ $# -gt 1 ]]
then
  dataset=$2
fi

if [[ $# -gt 2 ]]
then
  aggr=$3
fi

if [[ $# -gt 3 ]]
then
  attack=$4
fi

if [[ $# -gt 4 ]]
then
  pct_attack=$5
fi

if [[ $# -gt 5 ]]
then
  gauss_var=$6
fi

if [[ $# -gt 6 ]]
then
  debug=$7
fi

n_users=$(( 1 * n_core )) # Assume 100% of clients participate in each round

model='cnn'
if [[ 'fmnist' == "$dataset" || 'svhn' == "$dataset" ]]
then 
  model='mlp'
fi

# add '--use-hwthread-cpus' for more cores
prun="mpirun --use-hwthread-cpus --mca btl ^openib -hostfile $HOME/node_list.txt -np $n_core"
n_cpu_hw=`grep -c ^processor /proc/cpuinfo`
n_cpu=$(( n_cpu_hw / 1 ))

echo "FL with $aggr aggregation and $n_users clients under $pct_attack $attack attack ($n_cpu cores/node) GaussVar = $gauss_var"

# if [[ $# -gt 2 && $3 == 'gpu' ]] 
# then
#   $prun ~/system-0/src/defed.py --frac=1 --n_cpu=$n_cpu --dataset=$dataset --model=$model --num_users="$n_users" --gpu=cuda:0
# else
$prun ~/system-0/src/gausschain.py --frac=1 --n_cpu=$n_cpu --dataset=$dataset --model=$model \
    --num_users="$n_users" --debug=$debug --aggr=$aggr --attack_type=$attack --adversary=$pct_attack \
    --gauss_var=$gauss_var
# fi
