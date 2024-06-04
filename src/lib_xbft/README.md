# xbft: Cross-ledger BFT protocols

## Results on 6/21/2022
* 1008-node, 4-chain (`./run-single.sh 1008 4 100`)
    * TC = 11239 ms
    * WN = 29344 ms
    * PS = 22201 ms
* 1008-node, 16-chain (`./run-single.sh 1008 16 100`)
    * TC = 5742 ms
    * WN = 8854 ms
    * PS = 36419 ms
* 1008-node, 48-chain (`./run-single.sh 1008 48 100`)
    * TC = 5665 ms
    * WN = 7520 ms
    * PS = 100565 ms
* 1008-node, 144-chain (`./run-single.sh 1008 144 100`)
    * TC = 4137 ms
    * WN = 5966 ms
    * PS = 289778 ms    

## Results
- 3/5/2022 
    - For 128 nodes, 32 chains, 16 transactions, and 15 faults:
        - PS = 10411 ms
        - WN = 270--800 ms
        - TC = 220 ms
    - For 128 nodes, 4 chains, 1000 transactions, and 0 faults:
        - PS = 92 s
        - WN = 25 s
        - TC = 16 s    
    - Check ./src/result_sample.txt for more results

## Run
* For a single execution: `./src/run_single.sh`
* For a batch of executions: `python ./src/run_batch.py`

## Installation
* `sudo apt install libboost-all-dev`
* `sudo apt install libgmp-dev`
* `cd src`
* `make`

## Cluster setup (on CloudLab)
* Make sure that the code works fine on node0
    * `nfs_dir=nfs_$(hostname -d | cut -c1-4); ln -s /proj/dsdm-PG0/images/$nfs_dir /nfs`  
* Copy-n-paste node0's public key to CloudLab
* The following steps should all be taken on node0
* Clear SSH warnings on node0: `parallel-ssh -O StrictHostKeyChecking=no -h hosts_pssh.txt hostname`
* Setup the NFS on all other nodes: `nfs_dir=nfs_$(hostname -d | cut -c1-4); parallel-ssh -h hosts_pssh.txt sudo ln -s /proj/dsdm-PG0/images/$nfs_dir /nfs`
* Share the node hostname: `cp hosts_pssh.txt /nfs`
* Clear all SSH warnings: `parallel-ssh -h hosts_pssh.txt parallel-ssh -O StrictHostKeyChecking=no -h /nfs/hosts_pssh.txt hostname`
* We'll need to manually upload the public keys to CloudLab, one server after another
    * Sometimes the short hostname doesn't work, so we need to use the full name. 
        * In that case, create a list of full hostnames in a file, e.g., `hosts_pssh.txt`.
        * Then, use the following example (of node1) to handle each server: 
            * ``node=`sed '2!d' hosts_pssh.txt`; ssh $node ssh-keygen``
            * ``node=`sed '2!d' hosts_pssh.txt`; ssh $node cat ~/.ssh/id_rsa.pub``
            * Copy-and-paste the public key to CloudLab
* Install libraries
    * `parallel-ssh -h hosts_pssh.txt sudo apt update`
    * `parallel-ssh -h hosts_pssh.txt sudo apt -y upgrade`
        * If this fails (e.g., Ubuntu 18), then you need to manually upgrade each node. For example, on node1:
        * ``node=`sed '2!d' hosts_pssh.txt`; ssh $node sudo apt -y upgrade``
    * `parallel-ssh -h hosts_pssh.txt sudo apt -y install libgmp-dev`    
    * `parallel-ssh -h hosts_pssh.txt sudo apt -y install libboost-all-dev`
    * `parallel-ssh -h hosts_pssh.txt sudo apt -y install openmpi-bin`  
    * `parallel-ssh -h hosts_pssh.txt sudo apt -y install libopenmpi-dev`
* Compile
    * `cp -r /hpdic/xbft /nfs` 
    * `parallel-ssh -h hosts_pssh.txt sudo chmod 775 /hpdic`
    * `parallel-ssh -h hosts_pssh.txt cd /nfs/xbft/src; make clean; make`   
