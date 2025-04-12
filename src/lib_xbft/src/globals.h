/*
 * Created on Sun Feb 13 2022
 *
 * globals.h
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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <mpi.h> 
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <cstdarg>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <ctime>
#include <boost/algorithm/string.hpp>

#include "workload.h"
#include "node.h"

using namespace std;

#define DEFAULT_OP 666
#define STATUS_NORMAL 0
#define STATUS_CRASH 1
#define STATUS_BYZANTINE 2
#define SIMPLEX_DELIM ";"
#define PBFT_MSG_LEN_MAX 4
#define PBFT_F 1 // "f" in PBFT
#define PBFT_OP_NULL 999 // In PBFT new-view messages, some O messages have null operations
#define ERROR_PTR_NULL 404 // Some pointers are unexpectedly empty
#define LATENCY_LOCAL_OP 10 // Default latency for local operation

// Message types for 2PC
// We don't include other "failure" messages because they should have been handled by PBFT
#define MSG_2PC_PREPARE 0
#define MSG_2PC_READY 1
#define MSG_2PC_COMMIT 2
#define MSG_2PC_ACK 3

// (Inter-ledger) Message types for topological commit protocols
#define MSG_TCP_REQ 0
#define MSG_TCP_RDY 1
#define MSG_TCP_CMT 2

/**
 * @brief Keep in mind that these global variables are visible to the same rank only.
 * That is, these global variables are shared among the functions/classes of the same process;
 * They are not supposed to be shared across different ranks.
 * 
 */

inline int g_n_chain = 4;
inline int g_debug = 1;
inline int g_n_txn = 1;

inline MPI_Comm comm_local_chain;

class Workload;
inline Workload *xbft_txns;

class Node;
inline Node *xbft_node; // Rollback smart pointer back to raw pointer

inline map<int, list<MPI_Comm*>> g_map_node_comm;

// A simplex is expressed as a string, e.g., "2;3" means a 1-simplex of rank-2 and rank-3
inline map<string, MPI_Comm*> g_map_simplex_comm;

// PBFT global variables
inline int g_pbft_view_no = 0;
inline int g_pbft_time_no = 0;
inline int g_pbft_request = 0;
 
inline list<vector<int>> g_cert_prepare;

// Witeness network
inline string g_witnet;

#endif
