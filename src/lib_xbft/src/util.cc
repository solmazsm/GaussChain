/*
 * Created on Sun Feb 13 2022
 *
 * util.cc
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "globals.h"
#include "util.h"
#include "protocol.h"
#include "topology.h"

using namespace std;

string simplex_vec_2_string(vector<string> strs)
{
    int len = strs.size();
    int cnt = 0;
    string res;
    for (auto str : strs)
    {
        res += str;
        if (cnt < len - 1)
        {
            res += SIMPLEX_DELIM;
            cnt++;
        }
    }
    return res;
}

void update_g_witnet()
{
    int size_world;
    int rank_world;
    MPI_Comm_size(MPI_COMM_WORLD, &size_world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    vector<string> vec_primaries;
    boost::split(vec_primaries, g_witnet, boost::is_any_of(SIMPLEX_DELIM));
    int status_primary = -1;

    for (auto primary : vec_primaries)
    {
        auto iter = xbft_node->status.find(stoi(primary));
        if (iter != xbft_node->status.end())
        {
            auto elem = iter->second.begin();
            std::advance(elem, g_pbft_time_no);
            status_primary = *elem;
        }
        if (STATUS_BYZANTINE == status_primary)
        {
            int size_chain = (int) size_world / g_n_chain;
            int primary_base = (int) (stoi(primary) / size_chain) * size_chain;
            string primary_new = to_string((stoi(primary) + 1) % size_chain + primary_base);
            std::replace(vec_primaries.begin(), vec_primaries.end(), primary, primary_new);

            // Remove the old proxy simplex
            MPI_Comm *comm_proxy = g_map_simplex_comm.find(g_witnet)->second;
            if (nullptr != comm_proxy && node_in_simplex(rank_world, g_witnet))
            {
                MPI_Comm_free(comm_proxy);
            }

            // Remove the key-value pair
            auto iter = g_map_simplex_comm.find(g_witnet);
            g_map_simplex_comm.erase(iter);

            // Update the new proxy simplex
            g_witnet = simplex_vec_2_string(vec_primaries);
            simplex_construct(g_witnet);

            // if (1 == g_debug && 1 == rank_world)
            // {
            //     cout << "primary = " << primary << endl;
            // }
        }
    }
}

void init_simplex_proxy(vector<string> simplices)
{
    // Construct the initial inter-chain communicator
    for (auto simplex : simplices)
    {
        g_witnet += simplex_at(simplex, 0) + SIMPLEX_DELIM;
    }
    g_witnet = g_witnet.substr(0, g_witnet.length() - 1); // Truncate the last deliminator
    simplex_construct(g_witnet);
}

void init_chain_topology(vector<string> &simplices)
{
    // Construct the simplices
    int size_world = -1;
    int rank_world = -1;
    MPI_Comm_size(MPI_COMM_WORLD, &size_world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    int size_simplex = (int)size_world / g_n_chain;
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

    // Build up the simplex-communiators
    for (auto simplex : simplices)
    {
        simplex_construct(simplex);
    }
}

/**
 * @brief Print an vector of integers
 * 
 * @param vec 
 */
void print_vector_int(vector<int> vec)
{
    cout << "[";
    int len = vec.size();
    for (int i = 0; i < len; i++)
    {
        cout << vec.at(i);
        if (i < len - 1)
        {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

/**
 * @brief The rank of node in the simplex for the next primary
 * 
 * @param simplex 
 * @return int 
 */
int next_primary(string simplex)
{
    int rank_world = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;
    if (nullptr == comm_simplex)
    {
        cout << __FILE__ << __LINE__ << ": Cannot locate simplex communicator." << endl;
        return ERROR_PTR_NULL;
    }

    if (1 == node_in_simplex(rank_world, simplex))
    {
        int size_simplex = -1;
        MPI_Comm_size(*comm_simplex, &size_simplex);

        return (1 + g_pbft_view_no) % size_simplex;
    }

    return -1;
}

/**
 * @brief The ID of the primary within a simplex
 * 
 * @param simplex 
 * @return int 
 */
int current_primary(string simplex)
{
    int rank_world = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;
    if (nullptr == comm_simplex)
    {
        cout << __FILE__ << __LINE__ << ": Cannot locate simplex communicator." << endl;
        return ERROR_PTR_NULL;
    }

    if (1 == node_in_simplex(rank_world, simplex))
    {
        int size_simplex = -1;
        MPI_Comm_size(*comm_simplex, &size_simplex);

        return g_pbft_view_no % size_simplex;
    }
    
    return -1;
}

/**
 * @brief Print out a map nicely
 *
 * @param data
 */
void print_map_int_list_int(map<int, list<int>> data)
{
    for (auto iter : data)
    {
        cout << iter.first << " -> ";
        int is_not_first = 0;
        for (auto elem : iter.second)
        {
            if (!is_not_first)
            {
                is_not_first = 1;
            }
            else
            {
                cout << ", ";
            }
            cout << elem;
        }
        cout << endl;
    }
}

/**
 * @brief Print out a map nicely
 * 
 * @param data 
 */
void print_map_int_vector_int(map<int, vector<int>> data)
{
    for (auto iter : data)
    {
        cout << iter.first << " -> ";
        int is_not_first = 0;
        for (auto elem : iter.second)
        {
            if (!is_not_first)
            {
                is_not_first = 1;
            }
            else 
            {
                cout << ", ";
            }
            cout << elem;
        }
        cout << endl;
    }
}

/**
 * @brief Convert an index within a simplex into the index in the global world
 * 
 * @param simplex 
 * @param id_simplex 
 * @return int 
 */
int id_from_simplex_to_global(string simplex, int id_simplex)
{
    vector<string> result;
    boost::split(result, simplex, boost::is_any_of(SIMPLEX_DELIM));

    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);
    
    // if (1 == g_debug && 4 == rank_world)
    // {
    //     cout << __FILE__ << __LINE__ << ":";
    //     for (string iter : result)
    //     {
    //         cout << " " << iter;
    //     }
    //     cout << endl;

    //     cout << "result.at(id_simplex)=" << result.at(id_simplex) << endl;
    // }

    return stoi(result.at(id_simplex));
}

/**
 * @brief Return the size of the simplex
 * 
 * @param simplex 
 * @return int 
 */
int size_of_simplex(string simplex)
{
    char pbft_delim = SIMPLEX_DELIM[0];
    return std::count(simplex.begin(), simplex.end(), pbft_delim) + 1;
}

/**
 * @brief Return 1 if a node is found in the simplex; 0 otherwise
 *
 * @param node_id
 * @param simplex
 * @return int
 */
int node_in_simplex(int node_id, string simplex)
{
    vector<int> node_ids;
    simplexString_2_nodeVector(simplex, node_ids);

    auto iter = std::find(node_ids.begin(), node_ids.end(), node_id);

    if (iter == node_ids.end())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * @brief Convert a string of vertices into a vector of node IDs
 *
 * @param simplex: a string deliminated by SIMPLEX_DELIM, each number indicating an MPI rank ID
 * @return int: status
 */
int simplexString_2_nodeVector(string simplex, vector<int> &nodeIDs)
{
    int start;
    int end = 0;

    while ((start = simplex.find_first_not_of(SIMPLEX_DELIM, end)) != string::npos)
    {
        end = simplex.find(SIMPLEX_DELIM, start);
        nodeIDs.push_back(stoi(simplex.substr(start, end - start)));
    }

    return 0;
}

string simplex_at(string simplex, int idx)
{
    vector<int> nodeIDs;
    simplexString_2_nodeVector(simplex, nodeIDs);
    return to_string(nodeIDs.at(idx));
}


/**
 * @brief Variadic functions are functions (e.g. std::printf) which take a variable number of arguments.

To declare a variadic function, an ellipsis appears after the list of parameters, e.g. int printf(const char* format...);, which may be preceded by an optional comma. See Variadic arguments for additional detail on the syntax, automatic argument conversions and the alternatives.

To access the variadic arguments from the function body, the following library facilities are provided:

Defined in header <cstdarg>
va_start

enables access to variadic function arguments
(function macro)
va_arg

accesses the next variadic function argument
(function macro)
va_copy

(C++11)

makes a copy of the variadic function arguments
(function macro)
va_end

ends traversal of the variadic function arguments
(function macro)
va_list

holds the information needed by va_start, va_arg, va_end, and va_copy
(typedef)
 *
 * @param fmt
 */

void printf_debug(const char *fmt...) // C-style "const char* fmt, ..." is also valid
{

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (0 != world_rank || 0 == g_debug)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    cout << "\n\n############ DEBUG starts ###############" << endl;

    while (*fmt != '\0')
    {
        if (*fmt == 'd')
        {
            int i = va_arg(args, int);
            std::cout << i << endl;
        }
        else if (*fmt == 'c')
        {
            // note automatic conversion to integral type
            int c = va_arg(args, int);
            std::cout << static_cast<char>(c) << endl;
        }
        else if (*fmt == 'f')
        {
            double d = va_arg(args, double);
            std::cout << d << endl;
        }
        else if (*fmt == 's')
        {
            string str = va_arg(args, const char *);
            cout << str << endl;
        }
        else if (*fmt == 'v') // Variable name
        {
            string str = va_arg(args, const char *);
            cout << str << "=";
        }
        ++fmt;
    }

    cout << "############ DEBUG ends #################\n\n";

    va_end(args);
}