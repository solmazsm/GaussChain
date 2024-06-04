/*
 * Created on Sun Feb 13 2022
 *
 * topology.cc
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


#include <mpi.h>
#include <iostream>

#include "topology.h"
#include "util.h"

using namespace std;

/**
 * @brief Can we create a simplex using MPI communicator?
 * This is a "good" example for why we should not use MPI calls inside a class
 * 
 * TODO: DO NOT USE THIS JUST YET, NOT FULLY TESTED YET
 *
 * @return int
 */
Simplex::Simplex(list<int> nodes)
{
    int len = nodes.size();
    // Create the list of ranks that will be included in the new communicator
    int *ranks = (int *)malloc(len * sizeof(int)); // I know this is ugly, but it's for MPI...
    int i = 0;

    for (auto node : nodes)
    {
        // printf_debug("vd", "i", i);
        ranks[i] = node;
        i++;
    }
    // printf_debug("sd", "OK", __LINE__);

    MPI_Group group_world;
    MPI_Comm_group(MPI_COMM_WORLD, &group_world);

    MPI_Group group_simplex;
    MPI_Group_incl(group_world, len, ranks, &group_simplex);

    MPI_Comm_create(MPI_COMM_WORLD, group_simplex, &comm_simplex);

    cout << "Line " << __LINE__ << ": " << &comm_simplex << endl;

    free(ranks);

    /**
     * @brief Update the map<node, list<communicator>>
     *
     */
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    for (auto node : nodes)
    {
        // cout << __FILE__ << ": Line" << __LINE__ << endl;
        auto pair = g_map_node_comm.find(node);
        if (pair != g_map_node_comm.end())
        {
            std::list<MPI_Comm *>::iterator it;
            it = std::find(pair->second.begin(), pair->second.end(), &comm_simplex);
            if (it != pair->second.end())
            {
                pair->second.push_back(&comm_simplex);
            }
        }
        else // New <node, communicator> map
        {
            list<MPI_Comm *> new_comm_list{&comm_simplex};
            // new_comm_list.push_back(&comm_simplex);

            g_map_node_comm.insert(
                std::pair<int, list<MPI_Comm *>>(node, new_comm_list));
        }
    }
}

/**
 * @brief Broadcast a message from the root in a local simplex
 * 
 * @param simplex 
 * @return int 
 */
int simplex_bcast(string simplex)
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (node_in_simplex(world_rank, simplex)) // This rank should be involved in the local bcast
    {
        // Load the simplex communicator
        MPI_Comm *p_comm_simplex = g_map_simplex_comm.find(simplex)->second;

        int buffer = 999;
        MPI_Bcast(&buffer, 1, MPI_INT, 0, *p_comm_simplex);

        int simplex_rank;
        MPI_Comm_rank(*p_comm_simplex, &simplex_rank);

        cout << "world_rank=" << world_rank
            << "; simplex_rank=" << simplex_rank
            << "; buffer=" << buffer << endl;
    }

    return 0;
}

/**
 * @brief Construct a simplex and update the communicator as a global variable
 *
 * @return int, status: 0 success; others error
 */
int simplex_construct(string sigma)
{
    auto iter = g_map_simplex_comm.find(sigma);

    if (iter == g_map_simplex_comm.end()) // If the communicator does not exist
    {
        vector<int> nodeIDs;
        simplexString_2_nodeVector(sigma, nodeIDs);
        // for (auto iter : nodeIDs)
        // {
        //     printf_debug("sdd", __FILE__, __LINE__, iter);
        // }

        int len = nodeIDs.size();
        int *ranks = (int *)malloc(len * sizeof(int)); // I know this is ugly, but it's for MPI...
        std::copy(nodeIDs.begin(), nodeIDs.end(), ranks);

        MPI_Group group_world;
        MPI_Comm_group(MPI_COMM_WORLD, &group_world);

        MPI_Group group_simplex;
        MPI_Group_incl(group_world, len, ranks, &group_simplex);

        MPI_Comm *p_comm_simplex = (MPI_Comm *)malloc(sizeof(MPI_Comm));
        MPI_Comm_create(MPI_COMM_WORLD, group_simplex, p_comm_simplex);

        free(ranks);

        g_map_simplex_comm.insert(pair<string, MPI_Comm *>(sigma, p_comm_simplex));
    }

    return 0;
}

/**
 * @brief Split the global comm. into n_chain chains
 *
 * @return int
 */
int split()
{

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    //
    // THIS IS VERY IMPORTANT
    //
    int key = world_rank % g_n_chain;
    MPI_Comm_split(MPI_COMM_WORLD,
                   key,
                   world_rank,
                   &comm_local_chain);

    int buffer = key;
    MPI_Bcast(&buffer, 1, MPI_INT, 0, comm_local_chain);

    // if (g_debug)
    // {
    //     cout << "Rank " << world_rank << " receives " << buffer << " from local root." << endl;
    // }
    

    return 0;
}