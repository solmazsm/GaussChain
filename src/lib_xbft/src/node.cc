/*
 * Created on Sun Feb 13 2022
 *
 * node.cc
 * Copyright (C) 2022 
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <mpi.h>

#include "globals.h"
#include "node.h"
#include "util.h"

using namespace std;

/**
 * @brief Construct a new Node:: Node object
 *        Let's assume ranks {0, size_simplex, ..., (n_fault-1)*size_simplex} are faulty at 
 *        transaction #'s {1, 2, ..., n_fault}
 *
 * @param n_fault Input: number of faulty nodes; should be less than the number of ledgers and g_n_txn
 */
Node::Node(int n_fault)
{
    int size_world;
    MPI_Comm_size(MPI_COMM_WORLD, &size_world);
    int size_simplex = (int) size_world / g_n_chain;

    // if (n_fault > size_simplex - 1 || n_fault > g_n_txn - 1)
    if (n_fault > g_n_txn - 1) // Allowing repeated failures
    {
            cout << "Too many faulty nodes. " << endl;
            return;
    }

    list<int> node_status;
    for (int i = 0; i < g_n_txn; i++)
    {
        node_status.push_back(STATUS_NORMAL);
    }

    int cnt_fault = 0;
    for (int node_id = 0; node_id < size_world; node_id++)
    {
        if (0 != node_id % size_simplex || cnt_fault >= n_fault) // Not a proxy and is nonfaulty, or enough faults already happened
        {
            this->status.insert(pair<int, list<int>>(node_id, node_status));
        }
        else
        {
            list<int> node_status_faulty;
            for (int i = 0; i < g_n_txn; i++)
            {
                if (i - 1 == (int) node_id / size_simplex)
                {
                    node_status_faulty.push_back(STATUS_BYZANTINE);
                }
                else
                {
                    node_status_faulty.push_back(STATUS_NORMAL);
                }
            }
            this->status.insert(pair<int, list<int>>(node_id, node_status_faulty));

            cnt_fault++;
        }
    }
}

/**
 * @brief Node status during the execution
 *      Each node keeps a list of status flags, each of which corresponds to a transaction in a workload
 *          - By default, all nodes will be normal
 */
Node::Node() 
{
    int n_txns = g_n_txn;
    int n_node;
    MPI_Comm_size(MPI_COMM_WORLD, &n_node);

    // printf_debug("vd", "n_node", n_node);
    
    list<int> node_status;
    for (int i = 0; i < n_txns; i++)
    {
        node_status.push_back(STATUS_NORMAL);
    }
    
    for (int node_id = 0; node_id < n_node; node_id++)
    {
        // Compromised primary for the 2nd transaction (timestamp)
        int node_id_faulty = 0;
        int txn_id_faulty = 1;

        if (node_id != node_id_faulty)
        {
            // Do NOT forget the THIS pointer!
            this->status.insert(pair<int, list<int>>(node_id, node_status));
        }
        else
        {
            list<int> node_status_faulty;
            for (int i = 0; i < n_txns; i++)
            {
                if (i == txn_id_faulty)
                {
                    node_status_faulty.push_back(STATUS_BYZANTINE);
                }
                else 
                {
                    node_status_faulty.push_back(STATUS_NORMAL);
                }

            }
            this->status.insert(pair<int, list<int>>(node_id_faulty, node_status_faulty));
        }
    }  
}

/**
 * @brief Load node status from a file
 * 
 */
Node::Node(string fname)
{
    // TODO: Update the following trivial inheritance
    Node();
}
