# Build the node list
rm -fr $HOME/node_list.txt
for i in $(seq 0 1 $((n_node-1)) ); do echo node$i >> $HOME/node_list.txt; done

# Create the worker list
cp ~/node_list.txt ~/worker_list.txt
sed -i 1d ~/worker_list.txt

# Set up local SSH
nfs_dir=/proj/dsdm-PG0/images/nfs_$(hostname -d | cut -c1-4)
cp $nfs_dir/ssh_cloudlab ~/.ssh/id_rsa
cp $nfs_dir/ssh_config ~/.ssh/config

# Set up Github
rm -fr $HOME/system-0
git clone git@github.com:hpdic/system-0.git; cd ~/system-0
git config --global core.editor "vim"

