/*
 * Created on Sun Feb 13 2022
 *
 * util.h
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

#ifndef UTIL_H
#define UTIL_H

#include "globals.h"
#include "protocol.h"

using namespace std;

void printf_debug(const char *fmt...);

int simplexString_2_nodeVector(string simplex, vector<int> &nodeIDs);
int node_in_simplex(int node_id, string simplex);

int size_of_simplex(string simplex);

int id_from_simplex_to_global(string simplex, int id_simplex);

void print_map_int_vector_int(map<int, vector<int>>);
void print_map_int_list_int(map<int, list<int>> data);


int current_primary(string simplex);
int next_primary(string simplex);

void print_vector_int(vector<int> vec);
void convert_view_new_to_vector(Msg_View_New msg_view_new, vector<int> &out_vec);

/**
 * @brief Initialize the simplex for proxies
 *
 * @param simplices Input
 */
void init_simplex_proxy(vector<string> simplices);

/**
 * @brief Initialize the topology of multiple chains
 *
 * @param simplices Output
 */
void init_chain_topology(vector<string> &simplices);

/**
 * @brief Return the idx-th rank of a simplex
 *
 * @param simplex
 * @param idx
 * @return string
 */
string simplex_at(string simplex, int idx);

/**
 * @brief Update the proxy simplex
 *
 */
void update_g_witnet();

/**
 * @brief Convert a vector of strings into a single string with SIMPLEX_DELIM
 *
 * @param strs
 * @return string
 */
string simplex_vec_2_string(vector<string> strs);

#endif
