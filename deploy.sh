#!/usr/bin/bash

# Broadcast the source code
parallel-ssh -h ~/worker_list.txt 'rm -fr ~/system-0; mkdir ~/system-0'
parallel-scp -h ~/worker_list.txt -r ~/system-0/* ~/system-0/.
