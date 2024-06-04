#!/usr/bin/bash

# Variable shortcut
pssh="parallel-ssh -h $HOME/node_list.txt -t 0"
pscp="parallel-scp -h $HOME/node_list.txt"
inst='sudo apt -y install'
pipi='pip install'

# Sync up terms and SSH keys on all nodes
echo [Running] $pscp $HOME/system-0/config/.vimrc $HOME/.vimrc
$pscp $HOME/system-0/config/.vimrc $HOME/.vimrc

echo [Running] $pscp $HOME/system-0/config/.screenrc $HOME/.screenrc
$pscp $HOME/system-0/config/.screenrc $HOME/.screenrc

echo [Running] $pscp ~/.ssh/config ~/.ssh/config
$pscp ~/.ssh/config ~/.ssh/config

echo [Running] $pscp ~/.ssh/id_rsa ~/.ssh/id_rsa
$pscp ~/.ssh/id_rsa ~/.ssh/id_rsa

echo [Running] $pscp ~/node_list.txt ~/node_list.txt
$pscp ~/node_list.txt ~/node_list.txt

echo [Running] 'for i in $( seq 1 1 ${n_node} ) ; do parallel-ssh -h ~/node_list.txt -i hostname; done'
for i in $( seq 1 1 ${n_node} ) ; do parallel-ssh -h ~/node_list.txt hostname; done

# Sync up source code on all nodes
echo [Running] ./deploy.sh
./deploy.sh

# Install libs
echo [Running] $pssh sudo apt update
$pssh sudo apt update

echo [Running] $pssh $inst htop
$pssh $inst htop

echo [Running] $pssh $inst openmpi-bin 
$pssh $inst openmpi-bin 

echo [Running] $pssh $inst libopenmpi-dev
$pssh $inst libopenmpi-dev

echo [Running] $pssh $inst libboost-all-dev
$pssh $inst libboost-all-dev

echo [Running] $pssh $inst libgmp-dev
$pssh $inst libgmp-dev

echo [Running] $pssh $inst openjdk-11-jdk-headless
$pssh $inst openjdk-11-jdk-headless

echo [Running] $pssh $inst python3-pip
$pssh $inst python3-pip

echo [Running] $pssh $pipi pip --upgrade 
$pssh $pipi pip --upgrade

echo [Running] $pssh $pipi mpi4py
$pssh $pipi https://github.com/mpi4py/mpi4py/tarball/master

echo [Running] $pssh $pipi cryptography --upgrade
$pssh $pipi cryptography --upgrade

echo [Running] $pssh $pipi pyopenssl
$pssh $pipi pyopenssl

echo [Running] $pssh $pipi phe
$pssh $pipi phe

echo [Running] $pssh $pipi pyfhel
$pssh $pipi pyfhel

echo [Running] $pssh $pipi tenseal
$pssh $pipi tenseal

echo [Running] $pssh $pipi scipy
$pssh $pipi scipy

echo [Running] $pssh $pipi torchvision
$pssh $pipi torchvision

echo [Running] $pssh $pipi torchtext
$pssh $pipi torchtext

echo [Running] $pssh $pipi torchaudio
$pssh $pipi torchaudio

echo [Running] $pssh $pipi protobuf
$pssh $pipi protobuf

echo [Running] $pssh $pipi tensorboardX
$pssh $pipi tensorboardX

echo [Running] $pssh $pipi cassandra-driver
$pssh $pipi cassandra-driver

# Compile XBFT (C++)
cd ~/system-0/src/lib_xbft/src/; make clean; make
echo [Running] parallel-scp -h ~/node_list.txt ~/xbft_target/bin/xbft ~/xbft
parallel-scp -h ~/node_list.txt ~/xbft_target/bin/xbft ~/xbft
cd

# Prepare data sets
$pssh rm -fr $HOME/train
$pssh rm -fr $HOME/test
$pssh rm -fr $HOME/val
$pssh rm -fr $HOME/images.zip
# cifar
$pssh mkdir -p /tmp/data_fl_mpi/cifar
$pssh rm -fr /tmp/data_fl_mpi/cifar/*
$pssh wget -P /tmp/data_fl_mpi/cifar http://data.brainchip.com/dataset-mirror/cifar10/cifar-10-python.tar.gz
$pssh 'cd /tmp/data_fl_mpi/cifar; tar xzf cifar-10-python.tar.gz'

echo [Running] $pssh wget -P $HOME https://hpdic.github.io/download/images.zip
$pssh wget -P $HOME https://hpdic.github.io/download/images.zip

echo [Running] $pssh 'cd $HOME; unzip images.zip'
$pssh 'cd $HOME; unzip images.zip'

# Setup Cassandra
CASS_VER=apache-cassandra-4.0.5
CASS_URL=https://dlcdn.apache.org/cassandra/4.0.5/apache-cassandra-4.0.5-bin.tar.gz
$pssh rm -fr $HOME/$CASS_VER
$pssh rm -fr $HOME/$CASS_VER-bin.tar.gz

echo [Running] $pssh wget -P $HOME $CASS_URL
$pssh wget -P $HOME $CASS_URL

$pssh tar -xzvf $HOME/$CASS_VER-bin.tar.gz

$pscp $HOME/system-0/cass_vanilla/cassandra.yaml $HOME/$CASS_VER/conf/.
headnode=`head -n 1 ~/node_list.txt`
$pssh sed -i s/SEEDIP/$headnode/g $HOME/$CASS_VER/conf/cassandra.yaml
cmd='sed -i s/LOCALIP/"$(hostname -s)"/g'
cmd="$cmd $HOME/$CASS_VER/conf/cassandra.yaml"
$pssh $cmd
