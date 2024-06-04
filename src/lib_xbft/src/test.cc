/*
 * Created on Sun Feb 13 2022
 *
 * test.cc
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
#include "test.h"
#include "topology.h"
#include "util.h"
#include "protocol.h"
#include "baseline.h"

using namespace std;

/**
 * @brief 
 * 
 * @return int 
 */
int test_baseline_topocommit()
{
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    // Initialize the chains
    vector<string> simplices;
    init_chain_topology(simplices);

    // Initialize the proxy-chain
    init_simplex_proxy(simplices);

    // Process transactions
    // baseline_witnet(simplices);
    int n_txn = xbft_txns->txns.size();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < n_txn; i++)
    {
        baseline_topocommit(simplices);

        if (g_debug && rank_world == 0)
        {
            cout << "Transaction #" << i << " completed: View #" << g_pbft_view_no
                 << ", Time #" << (g_pbft_time_no - 1) << endl;
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (0 == rank_world)
    {
        cout << chrono::duration_cast<chrono::milliseconds>(end - begin).count();
    }

    return 0;
}

/**
 * @brief Test witNet by Zackary
 * 
 * @return int 
 */
int test_baseline_witnet()
{
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    // Initialize the chains
    vector<string> simplices;
    init_chain_topology(simplices);

    // Initialize the proxy-chain
    init_simplex_proxy(simplices);

    // Process transactions
    // baseline_witnet(simplices);
    int n_txn = xbft_txns->txns.size();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < n_txn; i++)
    {
        baseline_witnet(simplices);

        if (g_debug && 0 == rank_world)
        {
            cout << "Transaction #" << i << " completed: View #" << g_pbft_view_no
                 << ", Time #" << (g_pbft_time_no - 1) << endl;
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (0 == rank_world)
    {
        cout << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << endl;
    }

    return 0;
}

/**
 * @brief Test pairSwap by Maurice Herlihy
 * 
 * @return int 
 */
int test_baseline_pairswap()
{
    // Construct the simplices
    vector<string> simplices;
    int size_world = -1;
    int rank_world = -1;
    MPI_Comm_size(MPI_COMM_WORLD, &size_world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    int size_simplex = (int) size_world / g_n_chain;
    // cout << __FILE__ << __LINE__ << size_simplex;
    for (int simplex_id = 0; simplex_id < g_n_chain; simplex_id++)
    {
        string simplex;
        for (int rank_simplex = 0; rank_simplex < size_simplex; rank_simplex++)
        {
            simplex += to_string(simplex_id * size_simplex + rank_simplex);
            if (rank_simplex < size_simplex - 1)
            {
                simplex += SIMPLEX_DELIM;
            }
        }
        simplices.push_back(simplex);
    }

    // if (1 == g_debug && 0 == rank_world)
    // {
    //     cout << __FILE__ << __LINE__ << ": simplices = [";
    //     for (auto str : simplices)
    //     {
    //         cout << str << ", ";
    //     }
    //     cout << "]" << endl;
    // }

    // Build up the simplex-communiators
    for (auto simplex : simplices)
    {
        simplex_construct(simplex);
    }

    // Process the transactions
    if (xbft_txns != nullptr)
    {
        int n_txn = xbft_txns->txns.size();
        // cout << "n_txn = " << n_txn << endl;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (int i = 0; i < n_txn; i++)
        {
            baseline_pairswap(simplices);

            if (g_debug && 0 == rank_world)
            {
                cout << "Transaction #" << i << " completed: View #" << g_pbft_view_no 
                     << ", Time #" << (g_pbft_time_no - 1) << endl;
            }
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        if (0 == rank_world)
        {
            cout << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << endl;
        }
    }
    else
    {
        if (rank_world == 0 && 1 == g_debug)
        {
            cout << __FILE__ << __LINE__ << ": ERROR. " << endl;
            return ERROR_PTR_NULL;
        }
    }

    return 0;
}

/**
 * @brief Test the fault tolerance of PBFT
 * 
 */
void test_pbft_fault(string simplex, MPI_Comm *comm_simplex, map<int, vector<int>> msg_prepare)
{
    // MPI Stuff
    int rank_world = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    int rank_simplex = -1;
    if (1 == node_in_simplex(rank_world, simplex))
    {
        MPI_Comm_rank(*comm_simplex, &rank_simplex);
    }

    // Check the node status (from the global variable)
    if (0 == rank_world && 1 == g_debug)
    {
        cout << "xbft_node == nullptr ? " << (xbft_node == nullptr) << endl;

        for (auto iter : xbft_node->status)
        {
            cout << iter.first << ": ";
            for (auto elem : iter.second)
            {
                cout << elem << ", ";
            }
            cout << endl;
        }   
    } 
    
    // Construct an arbitrary simplex
    // string simplex = "1;2;3;4";
    // simplex_construct(simplex);

    // Update the timestamp
    g_pbft_time_no = 1;

    // Load the status of the primary
    int size_simplex = size_of_simplex(simplex);
    int id_primary = g_pbft_view_no % size_simplex;
    int status_primary = -1;
    int id_primary_global = id_from_simplex_to_global(simplex, id_primary);
    printf_debug("sdvd", __FILE__, __LINE__, "id_primary_global", id_primary_global);
    auto iter = xbft_node->status.find(id_primary_global);
    if (iter != xbft_node->status.end())
    {
        auto elem = iter->second.begin();
        std::advance(elem, g_pbft_time_no);
        status_primary = *elem;
    }
    printf_debug("sdvd", __FILE__, __LINE__, "status_primary", status_primary);

    // If the primary is faulty
    if (STATUS_BYZANTINE == status_primary)
    {
        /**
         * @brief PBFT <VIEW-CHANGE>
         * 
         */
        vector<Msg_View_Change> recv_view_change;
        pbft_view_change(simplex, msg_prepare, recv_view_change);

        if (rank_simplex == (1 + g_pbft_view_no) % size_simplex) // The "new" primary
        {
            cout << "For the new primary node rank_simplex=" << rank_simplex << endl;
            cout << __FILE__ << __LINE__ << ": g_pbft_view_no=" << g_pbft_view_no << endl;
            for (auto iter : recv_view_change)
            {
                iter.print_msg();
            }
        }

        // Test another node
        // if (rank_simplex == 3) 
        // {
        //     for (auto iter : recv_view_change)
        //     {
        //         cout << __FILE__ << __LINE__ << ": g_pbft_view_no=" << g_pbft_view_no << endl;
        //         iter.print_msg();
        //         for (auto elem : g_cert_prepare){
        //             print_vector_int(elem);
        //         }
        //     }
        // }

        /**
         * @brief PBFT <VIEW-NEW>
         * 
         */
        pbft_view_new(simplex, recv_view_change);
    }
}

/**
 * @brief Test PBFT on a specfic simplex
 *
 */
void test_pbft()
{
    vector<int> msg_preprepare;

    // Construct an arbitrary simplex
    string simplex = "1;2;3;4";
    simplex_construct(simplex);

    // Set an arbitrary message
    // g_pbft_view_no = 111;
    // g_pbft_time_no = 222;
    // g_pbft_request = 333;

    // Retrieve g_map_simplex_comm
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    for (auto iter : g_map_simplex_comm)
    {
        cout << "Rank " << world_rank << " Simplex " << iter.first << ": " << iter.second << endl;
    }

    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;

    if (1 == g_debug)
    {
        cout << __FILE__ << ": " << __LINE__ << ": " << comm_simplex << endl;
    }

    /**
     * @brief PBFT 1st phase
     * 
     */
    pbft_propose(simplex, comm_simplex, msg_preprepare);

    // Check the pre-prepare message
    cout << __FILE__ << __LINE__ << ": world_rank=" << world_rank;
    for (auto iter : msg_preprepare)
    {
        cout << ", " << iter;
    }
    cout << endl;

    /**
     * @brief PBFT 2nd phase
     * 
     */
    map<int, vector<int>> msg_prepare;
    pbft_prepare(simplex, comm_simplex, msg_preprepare, msg_prepare);

    // Check the prepare message
    cout << "world_rank=" << world_rank << "; ";
    for (auto iter : msg_prepare)
    {
        cout << "sender_rank=" << iter.first << " ";

        cout << "(";
        for (auto msg : iter.second)
        {
            cout << msg << ", ";
        }
        cout << "); ";
    }
    cout << endl;

    /**
     * @brief PBFT 3rd phase
     * 
     */
    map<int, vector<int>> msg_commit;
    vector<int> cert_prepare;
    pbft_commit(simplex, comm_simplex, msg_preprepare, msg_prepare, cert_prepare, msg_commit);
    g_cert_prepare.push_back(cert_prepare);
    
    if (3 == world_rank)
    {
        cout << __FILE__ << __LINE__ << ": ";
        print_vector_int(cert_prepare);
    }
    
    // Check commit messages
    cout << __FILE__ << __LINE__ << ": world_rank=" << world_rank << "; ";
    for (auto iter : msg_commit)
    {
        cout << "sender_rank=" << iter.first << " ";

        cout << "(";
        for (auto msg : iter.second)
        {
            cout << msg << ", ";
        }
        cout << "); ";
    }
    cout << endl << endl;

    /**
     * @brief PBFT 4th phase
     * 
     */
    vector<int> msg_reply;
    pbft_reply(simplex, comm_simplex, msg_prepare, msg_commit, msg_reply);

    MPI_Barrier(MPI_COMM_WORLD);
    if (1 == g_debug && !msg_reply.empty())
    {
        cout << __FILE__ << __LINE__ << ": rank_world=" << world_rank
             << ", msg_reply=(" << msg_reply[0] << ", " << msg_reply[1] << ")" << endl;
        fflush(stdout);
    }

    /**
     * @brief PBFT fault tolerance
     * 
     */
    test_pbft_fault(simplex, comm_simplex, msg_prepare);
}

/**
 * @brief Let's say we want to broadcast a message in a simplex
 *
 * @param simplex
 */
void test_simplex_bcast()
{
    simplex_bcast("2;3");
}

/**
 * @brief Construct a new communicator for a user-defined simplex
 *
 */
void test_simplex_construct()
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    simplex_construct("2;3");
    simplex_construct("0;1;2");

    // Retrieve g_map_simplex_comm
    for (auto iter : g_map_simplex_comm)
    {
        cout << "Rank " << world_rank << " Simplex " << iter.first << ": " << iter.second << endl;
    }
}

void test_simplex_class()
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    // printf_debug("vdsd", "world_rank", world_rank, __FILE__, __LINE__);

    std::list<int> nodes = {2, 3};
    int len = nodes.size();
    int *ranks = (int *)malloc(len * sizeof(int)); // I know this is ugly, but it's for MPI...
    int i = 0;
    for (auto node : nodes)
    {
        // printf_debug("vd", "i", i);
        ranks[i] = node;
        i++;
    }
    MPI_Group group_world;
    MPI_Comm_group(MPI_COMM_WORLD, &group_world);
    MPI_Group group_simplex;
    MPI_Group_incl(group_world, len, ranks, &group_simplex);
    MPI_Comm comm_simplex;
    MPI_Comm_create(MPI_COMM_WORLD, group_simplex, &comm_simplex);

    cout << "Line " << __LINE__ << ": " << &comm_simplex << endl;

    // Simplex sigma = Simplex(nodes);

    // Check the global map<node, list<comm>>
    for (auto entry : g_map_node_comm)
    {
        // cout << "Line " << __LINE__ << ": Node " << entry.first << endl;
        for (auto elem : entry.second)
        {
            // cout << "\tComm address: " << elem << endl;
        }
    }

    // delete sigma;
}