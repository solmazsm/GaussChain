/*
 * Created on Sun Feb 13 2022
 *
 * protocol.h
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "globals.h"

using namespace std;

/**
 * @brief The set of prepare certificates
 *
 */
class Msg_View_Change
{
public:
    int view_no_new;
    int sender;
    map<int, vector<int>> msg_prepare; // We do NOT need this any more
    list<vector<int>> cert_prepare;

    // Constructors
    Msg_View_Change(int, int, map<int, vector<int>>);
    Msg_View_Change(vector<int> vector_prepare);

    // Methods
    int convert_to_vector(vector<int> &vector_view_change);
    void print_msg();
};

/**
 * @brief The <NEW-VIEW> message sent by the new primary
 *
 */
class Msg_View_New
{
public:
    int view_new; // New view number
    vector<Msg_View_Change> vector_view_change;
    map<string, vector<int>> proposals;
    int sender;

    // Constructor
    Msg_View_New(int sender, vector<Msg_View_Change> recv_view_change);
    Msg_View_New(vector<int> vector_view_new);

    // Convert to vector (for MPI)
    int convert_to_vector(vector<int> &out_vector);
};

/**
 * @brief The coordinator receives messages from participants
 *
 * @param simplex
 * @return int
 */
int twoPC_recv(string simplex, int msg);

/**
 * @brief The coordinator broadcasts a message to all participants
 *
 * @param simplex
 * @return int
 */
int twoPC_send(string simplex, int msg);

/**
 * @brief The entire PBFT round
 *
 * @param simplex
 * @return int
 */
int pbft(string simplex);

/**
 * @brief The second phase of faulty PBFT in a simplex communicator
 *
 * @param simplex
 * @param recv_view_change
 * @return int
 */
int pbft_view_new(string simplex, vector<Msg_View_Change> recv_view_change);

/**
 * @brief The first phase of faulty PBFT in a simplex communicator
 *
 * @param simplex
 * @param msg_prepare
 * @param recv_view_change
 * @return int
 */
int pbft_view_change(string simplex, map<int, vector<int>> msg_prepare, vector<Msg_View_Change> &recv_view_change);

/**
 * @brief The first phase of civil PBFT in a simplex communicator
 *
 * @param comm_simplex
 * @return int
 */
int pbft_propose(string simplex, MPI_Comm *comm_simplex, vector<int> &msg_preprepare);

/**
 * @brief The second phase of civil PBFT in a simplex communicator
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_preprepare
 * @param msg_prepare
 * @return int
 */
int pbft_prepare(string simplex, MPI_Comm *comm_simplex, vector<int> msg_preprepare,
                 map<int, vector<int>> &msg_prepare);

/**
 * @brief The third phase of civil PBFT in a simplex communicator
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_preprepare
 * @param msg_prepare
 * @param msg_commit
 * @return int
 */
int pbft_commit(string simplex, MPI_Comm *comm_simplex, vector<int> msg_preprepare,
                map<int, vector<int>> msg_prepare, vector<int> &cert_prepare, map<int, vector<int>> &msg_commit);

/**
 * @brief The fourth phase of civil PBFT in a simplex communicator
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_prepare
 * @param msg_commit
 * @param msg_reply
 * @return int
 */
int pbft_reply(string simplex, MPI_Comm *comm_simplex, map<int, vector<int>> msg_prepare,
               map<int, vector<int>> msg_commit, vector<int> &msg_reply);

#endif