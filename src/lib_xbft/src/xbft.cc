/*
 * Created on Sun Feb 13 2022
 *
 * xbft.cc
 * Copyright (C) 2022 dzhao@uw.edu
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

#include "globals.h"
#include "topology.h"
#include "workload.h"
#include "util.h"
#include "protocol.h"
#include "test.h"

using namespace std;

/**
 * @brief Entry point of cross-blockchain transactions!
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // if (1 == g_debug)
    // {
    //     cout << "Hello from rank " << world_rank << " out of " << world_size << " at " << processor_name << endl;
    // }

    // Set the number of chains
    if (argc > 1)
    {
        g_n_chain = stoi(argv[1]);
    }

    // Set up the number of transactions
    if (argc > 2)
    {
        g_n_txn = stoi(argv[2]);
    }

    // Set up the number of faulty nodes
    int n_fault = 0;
    if (argc > 3)
    {
        n_fault = stoi(argv[3]);
    }

    // Pick a protocol
    string protocol = "NA";
    if (argc > 4)
    {
        protocol = argv[4];
    }

    // Setup the debug flag
    if (argc > 5)
    {
        g_debug = stoi(argv[5]);
    }

    // Create multiple chains
    split();
    MPI_Barrier(MPI_COMM_WORLD);

    // Construct the workload, globally
    // xbft_txns = new Workload(g_n_txn);
    xbft_txns = new Workload(g_n_txn); // Replace smart pointers with raw pointers

    auto xbft_txn = xbft_txns->txns.begin();
    // if (1 == g_debug)
    // {
    //     while (xbft_txn != xbft_txns->txns.end())
    //     {
    //         auto entry = xbft_txn->txn.find(0);
    //         printf_debug("vdvd", "entry->first", entry->first, "entry->second", entry->second);
    //         xbft_txn++;
    //     }
    // }
    MPI_Barrier(MPI_COMM_WORLD);

    // cout << "File " << __FILE__ << ", Line " << __LINE__ << ": Rank " << world_rank << endl;
    // Test global <node, communicators> map
    // test_simplex_class();

    // Load the node status
    xbft_node = new Node(n_fault);
    // if (1 == g_debug && 0 == world_rank)
    // {
    //     print_map_int_list_int(xbft_node->status);
    // }

    /**
     * @brief Test individual features
     * 
     */
    // test_simplex_construct();
    // test_simplex_bcast();
    // test_pbft();

    /**
     * @brief Test baseline protocols
     * 
     */
    // test_baseline_pairswap();
    // test_baseline_witnet();

    // if (0 == world_rank) 
    // {
    //     cout << "xbft_node == nullptr ? " << (xbft_node == nullptr) << endl;
    // }

    // Run the proposed consensus protocol
    if ("TC" == protocol)
    {
        test_baseline_topocommit();
    }
    else if ("PS" == protocol)
    {
        test_baseline_pairswap();
    }
    else if ("WN" == protocol)
    {
        test_baseline_witnet();
    }
    else if ("NA" == protocol)
    {
        cout << "Invalid protocol." << endl;
        return 1;
    }

    delete xbft_node;
    delete xbft_txns;

    // Finalize the MPI environment.
    MPI_Finalize();

    return 0;
}