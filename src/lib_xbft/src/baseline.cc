/*
 * Created on Wed Feb 23 2022
 *
 * baseline.cc
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "globals.h"
#include "util.h"
#include "topology.h"

using namespace std;

int baseline_topocommit(vector<string> simplices)
{
    int rank_world, size_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    MPI_Comm_size(MPI_COMM_WORLD, &size_world);

    // Update the proxy simplex, if needed
    update_g_witnet();

    // Phase 1
    int msg;
    if (0 == rank_world)
    {
        msg = MSG_TCP_REQ;
    }
    MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Phase 2
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // Phase 3
    msg = MSG_TCP_RDY;
    int *res;
    if (0 == rank_world)
    {
        res = (int *)malloc(sizeof(int) * size_world);
    }
    MPI_Gather(&msg, 1, MPI_INT, res, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Phase 4
    if (0 == rank_world)
    {
        msg = MSG_TCP_REQ;
    }
    MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Phase 5
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // Add latency for local operations (default 10 ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(LATENCY_LOCAL_OP));

    // Increment the time stamp
    g_pbft_time_no++;

    return 0;
}

int baseline_witnet(vector<string> simplices)
{
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    // Update the proxy simplex
    // g_pbft_time_no = 1;
    update_g_witnet();
    // printf_debug("vs", "g_witnet", g_witnet.c_str());

    /**
     * @brief 2PC + WitnessNetwork + PBFT
     *
     */
    // Make decision from the witnessNetwork; we assume the first ledger is the witness network
    string witnet = simplices.at(0);
    if (node_in_simplex(rank_world, witnet))
    {
        pbft(witnet);
    }

    // The witnessNetwork now broadcasts the <PREPARE> message
    twoPC_send(g_witnet, MSG_2PC_PREPARE);

    // Each ledgers verifies the received message from the witness network
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // Each ledgers determins the reply to the witness network
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // The (proxy of) witness network recieves the reply from participants
    twoPC_recv(g_witnet, MSG_2PC_READY);

    // The witness network needs to verify the message (using PBFT)
    if (node_in_simplex(rank_world, witnet))
    {
        pbft(witnet);
    }

    // The witness network now (collectively) decides the decision
    if (node_in_simplex(rank_world, witnet))
    {
        pbft(witnet);
    }

    // The witness network sends out the decision
    twoPC_send(g_witnet, MSG_2PC_COMMIT);
    
    // The participants need to verify the decision from the witness network
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // Again, the participants need to agree on the replied value to the witness network
    for (auto simplex : simplices)
    {
        if (node_in_simplex(rank_world, simplex))
        {
            pbft(simplex);
        }
    }

    // Add latency for local operations (default 10 ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(LATENCY_LOCAL_OP));

    // The witness network now receives (multiple) replies from participants
    twoPC_recv(g_witnet, MSG_2PC_ACK);

    // The witness network now (collectively) decides the final decision for the user
    if (node_in_simplex(rank_world, witnet))
    {
        pbft(witnet);
    }

    // Increment the time stamp
    g_pbft_time_no++;

    return 0;
}

void txn_neighbor(string first_simplex, string second_simplex)
{
    // Retrieve the communicator of the simplex
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    MPI_Comm *comm_simplex;
    if (node_in_simplex(rank_world, first_simplex))
    {
        comm_simplex = g_map_simplex_comm.find(first_simplex)->second;
        pbft(first_simplex);
    }
    else if (node_in_simplex(rank_world, second_simplex))
    {
        comm_simplex = g_map_simplex_comm.find(second_simplex)->second;
        pbft(second_simplex);
    }

    // Add latency for local operations (default 10 ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(LATENCY_LOCAL_OP));
}

int baseline_pairswap(vector<string> simplices)
{
    int len = simplices.size();

    // Set the timer
    for (int i = 0; i < len; i++)
    {
        int j = (i+1) % len;
        txn_neighbor(simplices.at(i), simplices.at(j));
        
        // printf_debug("sd", "Round ", i+1);
    }

    // Commit the two-party transactions
    for (int i = len - 1; i >= 0; i--)
    {
        int j = (i+1) % len;
        txn_neighbor(simplices.at(j), simplices.at(i));
    }

    // Increment the time stamp
    g_pbft_time_no++;

    return 0;
}
