/*
 * Created on Sun Feb 13 2022
 *
 * protocol.cc
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

#include "protocol.h"
#include "util.h"
#include "topology.h"
#include "globals.h"

using namespace std;

/**
 * @brief Convert a NEW-VIEW into a vector
 *
 * @param out_vector
 * @return int
 */
int Msg_View_New::convert_to_vector(vector<int> &out_vector)
{
    out_vector.push_back(this->view_new);
    out_vector.push_back(this->sender);

    vector<int> vec_view_change;
    for (auto view_change : this->vector_view_change)
    {
        int cnt_cert = view_change.cert_prepare.size();
        vec_view_change.push_back(2 + cnt_cert * PBFT_MSG_LEN_MAX);

        vec_view_change.push_back(view_change.view_no_new);
        vec_view_change.push_back(view_change.sender);

        for (auto cert : view_change.cert_prepare)
        {
            for (auto elem : cert)
            {
                vec_view_change.push_back(elem);
            }
        }
    }

    out_vector.push_back(vec_view_change.size());
    out_vector.insert(out_vector.end(), vec_view_change.begin(), vec_view_change.end());

    vector<int> vec_proposals;
    for (auto iter : this->proposals)
    {
        out_vector.insert(out_vector.end(), iter.second.begin(), iter.second.end());
    }

    return 0;
}

/**
 * @brief Construct a new Msg_View_New::Msg_View_New object
 *
 * @param vec_view_new
 */
Msg_View_New::Msg_View_New(vector<int> vec_view_new)
{
    this->view_new = vec_view_new.at(0);
    this->sender = vec_view_new.at(1);

    // Reconstruct the VIEW-CHANGE objects
    int len_view_change = vec_view_new.at(2);
    for (auto pos_start = vec_view_new.begin() + 3;
         pos_start < vec_view_new.begin() + 3 + len_view_change;
         pos_start++)
    {
        vector<int> vec_view_change(pos_start + 1, pos_start + 1 + (*pos_start));
        auto msg_view_change = Msg_View_Change(vec_view_change);
        this->vector_view_change.push_back(msg_view_change);
        pos_start += *pos_start; // Haha, I probably won't under this code in a few days :)
    }

    // Reconstruct the proposals
    for (auto pos_start = vec_view_new.begin() + 3 + len_view_change;
         pos_start < vec_view_new.end();
         pos_start += PBFT_MSG_LEN_MAX)
    {
        string sr = to_string(*(pos_start + 1)) + SIMPLEX_DELIM + to_string(*(pos_start + 2));
        vector<int> proposal(pos_start, pos_start + PBFT_MSG_LEN_MAX);
        this->proposals.insert(pair<string, vector<int>>(sr, proposal));
    }
}

/**
 * @brief Construct a new Msg_View_New::Msg_View_New object
 *
 * @param in_view_change
 */
Msg_View_New::Msg_View_New(int in_sender, vector<Msg_View_Change> in_view_change)
    : sender(in_sender), vector_view_change(in_view_change) // I still don't get why the fuck C++ must use this init list for object parameters

{
    this->view_new = g_pbft_view_no + 1;

    // Construct the O (proposals) from this->view_change
    set<int> times;
    for (auto view_change : this->vector_view_change)
    {
        for (auto cert : view_change.cert_prepare)
        {
            times.insert(cert.at(2));

            string sr = to_string(cert.at(1)) + SIMPLEX_DELIM + to_string(cert.at(2));
            if (this->proposals.find(sr) == proposals.end())
            {
                vector<int> o{this->view_new, cert.at(1), cert.at(2), this->sender};
                this->proposals.insert(pair<string, vector<int>>(sr, o));
            }
        }
    }
    int t_max = *times.rbegin();
    for (int t = 0; t < t_max; t++)
    {
        if (times.end() == times.find(t))
        {
            string sr = to_string(t) + SIMPLEX_DELIM + to_string(PBFT_OP_NULL);
            vector<int> o{this->view_new, t, PBFT_OP_NULL, this->sender};
            this->proposals.insert(pair<string, vector<int>>(sr, o));
        }
    }
}

/**
 * @brief Print the object to stdout
 *
 */
void Msg_View_Change::print_msg()
{
    cout << "view_no_new=" << this->view_no_new << "; ";
    cout << "sender=" << this->sender << "; ";

    for (auto vec : this->cert_prepare)
    {
        print_vector_int(vec);
        cout << " ";
    }

    // for (auto iter : this->msg_prepare)
    // {
    //     cout << iter.first << "->(";
    //     for (auto elem : iter.second)
    //     {
    //         cout << elem << ", ";
    //     }
    //     cout << "), ";
    // }
    cout << endl;
}

/**
 * @brief Construct a new Msg_View_Change::Msg_View_Change object
 *
 * This contructor is invoked by explicitly providing a list of values as the prepare certificate
 *
 * @param vector_prepare
 */
Msg_View_Change::Msg_View_Change(vector<int> vector_prepare)
{
    this->view_no_new = vector_prepare.at(0);
    this->sender = vector_prepare.at(1);

    int msg_len = 0;
    vector<int> msg;
    for (int idx = 2; idx < vector_prepare.size(); idx++)
    {
        msg.push_back(vector_prepare.at(idx));
        msg_len++;
        if (PBFT_MSG_LEN_MAX == msg_len)
        {
            vector<int> elem = msg;
            this->msg_prepare.insert(pair<int, vector<int>>(elem.at(PBFT_MSG_LEN_MAX - 1), elem));
            this->cert_prepare.push_back(elem);

            // Reset the buffer
            msg.clear();
            msg_len = 0;
        }
    }
}

/**
 * @brief Construct a new Msg_View_Change::Msg_View_Change object
 *
 * This constructor is invoked when we try to load the prepare certificates from g_cert_prepare.
 *
 * @param view_no_new
 * @param sender
 * @param msg_prepare
 */
Msg_View_Change::Msg_View_Change(int view_no_new, int sender, map<int, vector<int>> msg_prepare)
{
    this->view_no_new = view_no_new;
    this->sender = sender;
    this->msg_prepare = msg_prepare; // This is not used any more.
    this->cert_prepare = g_cert_prepare;
}

/**
 * @brief Convert the message into a vector of integers for MPI
 *
 * @param vector_view_change Output. 0th: new view number; 1st: sender ID; every quadruple <v, s, r, n>
 * @return int Output: Status of this method
 */
int Msg_View_Change::convert_to_vector(vector<int> &vector_view_change)
{
    vector_view_change.push_back(this->view_no_new);
    vector_view_change.push_back(this->sender);
    for (auto iter : this->cert_prepare)
    {
        for (auto elem : iter)
        {
            vector_view_change.push_back(elem);
        }
    }

    return 0;
}

/**
 * @brief The coordinator receives messages from participants
 * 
 * @param simplex 
 * @return int 
 */
int twoPC_recv(string simplex, int msg)
{
    vector<string> vec_proxy;
    boost::split(vec_proxy, simplex, boost::is_any_of(SIMPLEX_DELIM));

    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (node_in_simplex(rank_world, simplex))
    {
        int *vote = (int*) malloc(sizeof(int) * g_n_chain);
        MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;

        MPI_Gather(&msg, 1, MPI_INT, vote, 1, MPI_INT, 0, *comm_simplex);
        free(vote);
    }

    return 0;
}

/**
 * @brief The coordinator broadcasts a message to all participants
 * 
 * @param simplex 
 * @return int 
 */
int twoPC_send(string simplex, int msg)
{
    vector<string> vec_proxy;
    boost::split(vec_proxy, simplex, boost::is_any_of(SIMPLEX_DELIM));

    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (node_in_simplex(rank_world, simplex))
    {
        MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;

        // cout << __FILE__ << __LINE__ << ": simplex=" << simplex << endl;

        MPI_Bcast(&msg, 1, MPI_INT, 0, *comm_simplex);
    }

    return 0;
}

/**
 * @brief The entire PBFT round
 *
 * @param simplex
 * @return int
 */
int pbft(string simplex)
{
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    /**
     *  @brief brief description the status of the primary
     * 
     */
    int id_primary = current_primary(simplex);
    int status_primary = -1;
    int id_primary_global = id_from_simplex_to_global(simplex, id_primary);
    auto iter = xbft_node->status.find(id_primary_global);
    if (iter != xbft_node->status.end())
    {
        auto elem = iter->second.begin();
        // cout << "g_pbft_time_no=" << g_pbft_time_no << endl;
        std::advance(elem, g_pbft_time_no);
        status_primary = *elem;
        // cout << status_primary << endl;
    }

    // If the primary is faulty, then change the view
    if (STATUS_BYZANTINE == status_primary)
    {
        if (g_debug && rank_world == id_primary_global)
        {
            cout << __FILE__ << __LINE__ << ": rank_world=" << rank_world 
                 << ". Found a faulty node." << endl;
        }

        vector<Msg_View_Change> recv_view_change;
        map<int, vector<int>> msg_prepare; // Not really used
        pbft_view_change(simplex, msg_prepare, recv_view_change);
        pbft_view_new(simplex, recv_view_change);
        g_pbft_view_no++;
    }

    /**
     * @brief Phase 1
     *
     */
    vector<int> msg_preprepare;
    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;
    pbft_propose(simplex, comm_simplex, msg_preprepare);

    /**
     * @brief Phase 2
     *
     */
    map<int, vector<int>> msg_prepare;
    pbft_prepare(simplex, comm_simplex, msg_preprepare, msg_prepare);

    /**
     * @brief Phase 3
     *
     */
    map<int, vector<int>> msg_commit;
    vector<int> cert_prepare;
    pbft_commit(simplex, comm_simplex, msg_preprepare, msg_prepare, cert_prepare, msg_commit);
    g_cert_prepare.push_back(cert_prepare);

    /**
     * @brief Phase 4
     *
     */
    vector<int> msg_reply;
    pbft_reply(simplex, comm_simplex, msg_prepare, msg_commit, msg_reply);

    return 0;
}

/**
 * @brief Second phase of PBFT recovery
 *
 * @param simplex
 * @param recv_view_change
 * @return int
 */
int pbft_view_new(string simplex, vector<Msg_View_Change> recv_view_change)
{
    // printf_debug("sds", __FILE__, __LINE__, "Starting new views.");

    // Retrieve the communicator
    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;
    if (nullptr == comm_simplex)
    {
        cout << __FILE__ << __LINE__ << ": Cannot locate simplex communicator." << endl;
        return ERROR_PTR_NULL;
    }

    // Calculate the ID of the old and new primaries
    int old_primary = current_primary(simplex);
    int new_primary = next_primary(simplex);

    // MPI ranks
    int rank_world = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (1 == node_in_simplex(rank_world, simplex))
    {
        // Local ranks within the simplex
        int rank_simplex = -1;
        int size_simplex = -1;
        MPI_Comm_rank(*comm_simplex, &rank_simplex);
        MPI_Comm_size(*comm_simplex, &size_simplex);

        // Try to collect 2f+1 VIEW-CHANGE messages and broadcast <v+1, V, O, p>
        int votes = 0;
        if (rank_simplex == new_primary) // What the new primary will do
        {
            // cout << __FILE__ << __LINE__ << ": current_primary=" << current_primary(simplex) << endl;
            // cout << __FILE__ << __LINE__ << ": new_primary=" << new_primary << endl;

            for (auto iter : recv_view_change)
            {
                if (g_pbft_view_no + 1 == iter.view_no_new)
                {
                    votes++;
                }
                if (votes >= 2 * PBFT_F + 1)
                {
                    break;
                }
            }
        }

        MPI_Bcast(&votes, 1, MPI_INT, new_primary, *comm_simplex);

        if (votes >= 2 * PBFT_F + 1)
        {
            int *array_view_new;
            int len;

            if (new_primary == rank_simplex)
            {
                // Construct the view_new message
                Msg_View_New view_new = Msg_View_New(new_primary, recv_view_change);
                vector<int> vec_view_new;
                view_new.convert_to_vector(vec_view_new);

                // if (new_primary == rank_simplex)
                // {
                //     print_vector_int(vec_view_new);
                // }

                // Broadcast the recv_view_change
                len = vec_view_new.size();

                array_view_new = (int *)malloc(sizeof(int) * len);
                std::copy(vec_view_new.begin(), vec_view_new.end(), array_view_new);
            }

            MPI_Bcast(&len, 1, MPI_INT, new_primary, *comm_simplex);
            if (rank_simplex != new_primary)
            {
                array_view_new = (int *)malloc(sizeof(int) * len);
            }

            // if (1 == g_debug && 2 == rank_simplex)
            // {
            //     for (int i = 0; i < len; i++)
            //     {
            //         cout << array_view_new[i] << ", ";
            //     }
            //     cout << endl;
            // }

            MPI_Bcast(array_view_new, len, MPI_INT, new_primary, *comm_simplex);

            // if (1 == g_debug && 2 == rank_simplex)
            // {
            //     for (int i = 0; i < len; i++)
            //     {
            //         cout << array_view_new[i] << ", ";
            //     }
            //     cout << endl;
            // }

            // Verify the NEW-VIEW at new replicas
            if (rank_simplex != new_primary)
            {
                // Reconstruct the NEW-VIEW object
                vector<int> vector_view_new(array_view_new, array_view_new + len);
                auto msg_view_new = Msg_View_New(vector_view_new);

                // Check each proposal
                int valid;
                for (auto proposal : msg_view_new.proposals)
                {
                    valid = 0;
                    int view = proposal.second.at(0);
                    int time = proposal.second.at(1);
                    int req = proposal.second.at(2);

                    if (req == PBFT_OP_NULL)
                    {
                        for (auto view_change : msg_view_new.vector_view_change)
                        {
                            if (view_change.view_no_new == view)
                            {
                                for (auto cert : view_change.cert_prepare)
                                {
                                    if (time == cert.at(1))
                                    {
                                        goto decision;
                                    }
                                }
                            }
                        }
                        valid = 1;
                    }
                    else
                    {
                        for (auto view_change : msg_view_new.vector_view_change)
                        {
                            if (view_change.view_no_new == view)
                            {
                                for (auto cert : view_change.cert_prepare)
                                {
                                    if (time == cert.at(1) && req == cert.at(2))
                                    {
                                        valid = 1;
                                        goto decision;
                                    }
                                }
                            }
                        }
                    }

                // Found an unqualified proposal
                decision:
                    if (0 == valid)
                    {
                        cout << "Something went wrong at rank_simplex " << rank_simplex << endl;
                        return 1;
                    }
                }

                // if ((new_primary + 1) % size_simplex == rank_simplex)
                // {
                //     cout << "All look good from rank_simplex " << rank_simplex << endl;
                // }
            }

            free(array_view_new);
        }
    }

    return 0;
}

/**
 * @brief First phase of PBFT recovery
 *
 * @param simplex
 * @return int
 */
int pbft_view_change(string simplex, map<int, vector<int>> msg_prepare, vector<Msg_View_Change> &recv_view_change)
{
    // printf_debug("sds", __FILE__, __LINE__, "Starting to change views.");

    // Retrieve the communicator
    MPI_Comm *comm_simplex = g_map_simplex_comm.find(simplex)->second;
    if (nullptr == comm_simplex)
    {
        cout << __FILE__ << __LINE__ << ": Cannot locate simplex communicator." << endl;
        return ERROR_PTR_NULL;
    }

    // MPI ranks
    int rank_world = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (1 == node_in_simplex(rank_world, simplex))
    {
        // Local ranks within the simplex
        int rank_simplex = -1;
        MPI_Comm_rank(*comm_simplex, &rank_simplex);

        // Increment the view index
        int view_no_new = g_pbft_view_no + 1;

        // Construct the view-change message
        auto msg_view_change = Msg_View_Change(view_no_new, rank_simplex, msg_prepare);

        // Retrieve the primary ID
        int primary = current_primary(simplex);

        // if (primary == rank_simplex)
        // {
        //     cout << __FILE__ << __LINE__ << ": view_no_new=" << view_no_new << endl;
        //     print_map_int_vector_int(msg_prepare);
        // }

        // Convert a Msg_View_Change object into an array of integers for MPI
        vector<int> vector_view_change;
        msg_view_change.convert_to_vector(vector_view_change);
        int len = vector_view_change.size();
        
        // if (2 == rank_simplex) // Check a random replica
        // {
        //     cout << __FILE__ << __LINE__ << ": len=" << len << endl;
        //     for (auto iter : vector_view_change)
        //     {
        //         cout << iter << " ";
        //     }
        //     cout << endl;
        // }

        // Msg_View_Change another_msg = Msg_View_Change(vector_view_change); // Test the constructor
        // if (next_primary(simplex) == rank_simplex)
        // {
        //     another_msg.print_msg();
        // }

        // MPI Broadcast the view-change message
        int rank_primary = current_primary(simplex);
        int size_simplex = size_of_simplex(simplex);
        for (int backup_id = 0; backup_id < size_simplex; backup_id++)
        {
            if (backup_id == rank_primary) // Skip the primary
            {
                continue;
            }
            int *array_view_change = (int *)malloc(len * sizeof(int));
            std::copy(vector_view_change.begin(), vector_view_change.end(), array_view_change);

            if (node_in_simplex(rank_world, simplex))
            {
                MPI_Bcast(array_view_change, len, MPI_INT, backup_id, *comm_simplex);

                vector<int> vector_view_change;
                vector_view_change.insert(vector_view_change.begin(),
                                          array_view_change, array_view_change + len);
                // print_vector_int(vector_view_change);
                recv_view_change.push_back(Msg_View_Change(vector_view_change));
            }

            free(array_view_change);
        }
    }

    return 0;
}

/**
 * @brief The fourth (last) phase of PBFT in civil cases
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_prepare
 * @param msg_commit
 * @param msg_reply Output
 * @return int
 */
int pbft_reply(string simplex, MPI_Comm *comm_simplex, map<int, vector<int>> msg_prepare,
               map<int, vector<int>> msg_commit, vector<int> &msg_reply)
{
    // Retrieve the ranks
    int rank_world = -1, rank_simplex = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (node_in_simplex(rank_world, simplex))
    {
        MPI_Comm_rank(*comm_simplex, &rank_simplex);

        int votes = 0;
        int proposal_view = msg_prepare.find(1)->second[0]; // "find(1)": we select the request from the first replica
        int proposal_time = msg_prepare.find(1)->second[1];

        for (auto iter : msg_commit)
        {
            if (iter.second[0] == proposal_view && iter.second[1] == proposal_time)
            {
                votes++;
            }
        }

        msg_reply.push_back(rank_simplex);
        if (votes >= 2 * PBFT_F + 1)
        {
            msg_reply.push_back(msg_prepare.find(1)->second[2]);
        }
        else
        {
            msg_reply.push_back(-1);
        }
    }

    return 0;
}

/**
 * @brief 3rd Phase of PBFT
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_preprepare
 * @param msg_prepare
 * @param msg_commit Output
 * @return int
 */
int pbft_commit(string simplex, MPI_Comm *comm_simplex, vector<int> msg_preprepare,
                map<int, vector<int>> msg_prepare, vector<int> &cert_prepare, map<int, vector<int>> &msg_commit)
{
    // Retrieve the ranks
    int rank_world = -1, rank_simplex = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    if (node_in_simplex(rank_world, simplex))
    {
        MPI_Comm_rank(*comm_simplex, &rank_simplex);

        // Collect the available votes
        int votes = 0;
        int proposal_view = msg_preprepare[0];
        int proposal_time = msg_preprepare[1];
        int proposal_req = msg_preprepare[2];

        char pbft_delim = SIMPLEX_DELIM[0];
        int size_simplex = std::count(simplex.begin(), simplex.end(), pbft_delim) + 1;

        for (auto iter : msg_prepare)
        {
            if (proposal_req == iter.second[2] && proposal_view == iter.second[0] && proposal_time == iter.second[1])
            {
                votes++;
                if (votes >= 2 * PBFT_F) // No need to continue
                {
                    // Construct the prepare certificate
                    cert_prepare.push_back(proposal_view);
                    cert_prepare.push_back(proposal_time);
                    cert_prepare.push_back(proposal_req);
                    cert_prepare.push_back(rank_simplex);

                    break;
                }
            }
        }

        // Enough votes collected, i.e., <PREPARE CERTIFICATE> obtained. Ready to commit
        if (votes >= 2 * PBFT_F)
        {
            int array_commit[PBFT_MSG_LEN_MAX] = {proposal_view, proposal_time, rank_simplex, -1};

            // cout << __FILE__ << __LINE__ << ": rank_siplex=" << rank_simplex << " (";
            // for (int i = 0; i < PBFT_MSG_LEN_MAX; i++)
            // {
            //     cout << array_commit[i];
            //     if (i != PBFT_MSG_LEN_MAX - 1)
            //     {
            //         cout << ", ";
            //     }
            // }
            // cout << ")" << endl;

            int *array_commit_all = (int *)malloc(sizeof(int) * PBFT_MSG_LEN_MAX * size_simplex);
            MPI_Allgather(array_commit, PBFT_MSG_LEN_MAX, MPI_INT,
                          array_commit_all, PBFT_MSG_LEN_MAX, MPI_INT, *comm_simplex);

            // if (1 == g_debug && 0 == rank_simplex)
            // {
            //     cout << __FILE__ << __LINE__ << ": array_commit_all = (";
            //     for (int i = 0; i < size_simplex; i++)
            //     {
            //         for (int j = 0; j < PBFT_MSG_LEN_MAX; j++)
            //         {
            //             int idx = i * size_simplex + j;
            //             cout << array_commit_all[idx];
            //             if (idx < PBFT_MSG_LEN_MAX * size_simplex - 1)
            //             {
            //                 cout << ", ";
            //             }
            //         }
            //     }
            //     cout << ")" << endl;
            // }

            // Add array_commit_all to msg_commit
            for (int i = 0; i < size_simplex; i++)
            {
                vector<int> vector_commit;
                int pos_start = i * PBFT_MSG_LEN_MAX;
                int pos_end = (i + 1) * PBFT_MSG_LEN_MAX;
                vector_commit.insert(vector_commit.begin(),
                                     array_commit_all + pos_start, array_commit_all + pos_end);
                msg_commit.insert(pair<int, vector<int>>(vector_commit[2], vector_commit));
            }
            // vector_prepare.insert(vector_prepare.begin(),
            //                       std::begin(tmp_prepare), std::end(tmp_prepare));

            free(array_commit_all);
        }
    }

    return 0;
}

/**
 * @brief 2nd phase of PBFT
 *
 * @param simplex
 * @param comm_simplex
 * @param msg_preprepare
 * @param msg_prepare Output
 * @return int
 */
int pbft_prepare(string simplex, MPI_Comm *comm_simplex, vector<int> msg_preprepare,
                 map<int, vector<int>> &msg_prepare)
{
    // Make sure irrelevant ranks are not involved
    int rank_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_world);

    // Retrieve the local rank in the simplex
    int rank_simplex;
    if (node_in_simplex(rank_world, simplex)) // IMPORTANT: need to check the qualified ranks
    {
        MPI_Comm_rank(*comm_simplex, &rank_simplex);
        // cout << __FILE__ << ": " << __LINE__
        //      << ": rank_world=" << rank_world << "; rank_simplex=" << rank_simplex
        //      << ", simplex = " << simplex << endl;
    }

    // Construct the PREPARE message
    // Option 1: A sequence of MPI_Bcast (slower?)
    // OPtion 2: MPI_Allgather and then ignore the root-messages?
    int array_prepare[PBFT_MSG_LEN_MAX];
    std::copy(msg_preprepare.begin(), msg_preprepare.end(), array_prepare);

    array_prepare[PBFT_MSG_LEN_MAX - 1] = rank_simplex; // Update the sender ID

    char pbft_delim = SIMPLEX_DELIM[0];
    int size_simplex = std::count(simplex.begin(), simplex.end(), pbft_delim) + 1;

    // This is a dangerous way to return the simplex size, because the size will look
    // different for ranks inside and outside the simplex.
    // if (node_in_simplex(rank_world, simplex))
    // {
    //     MPI_Comm_size(*comm_simplex, &size_simplex);
    // }

    // printf_debug("sdvd", __FILE__, __LINE__, "size_simplex", size_simplex);

    int rank_primary = current_primary(simplex);

    for (int backup_id = 0; backup_id < size_simplex; backup_id++) // Skip 0, the primary
    {
        if (backup_id == rank_primary)
        {
            continue;
        }

        // This is important to keep a temporay variable for the broadcast buffer
        int tmp_prepare[PBFT_MSG_LEN_MAX];
        std::copy(std::begin(array_prepare), std::end(array_prepare), std::begin(tmp_prepare));

        if (node_in_simplex(rank_world, simplex))
        {
            MPI_Bcast(tmp_prepare, PBFT_MSG_LEN_MAX, MPI_INT, backup_id, *comm_simplex);

            vector<int> vector_prepare;
            vector_prepare.insert(vector_prepare.begin(),
                                  std::begin(tmp_prepare), std::end(tmp_prepare));
            msg_prepare.insert(pair<int, vector<int>>(backup_id, vector_prepare));
        }
    }

    return 0;
}

/**
 * @brief The first phase of PBFT
 *
 * @param simplex Input: The simplex represented by a communicator/string
 * @param comm_simplex Input: Pointer to the simplex communicator
 * @param msg_preprepare Output: PRE-PREPARE message in the first phase
 * @return int Output: Status code
 */
int pbft_propose(string simplex, MPI_Comm *comm_simplex, vector<int> &msg_preprepare)
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (0 == node_in_simplex(world_rank, simplex)) // This rank is not in the simplex
    {
        return 0;
    }

    int rank_simplex = -1;
    MPI_Comm_rank(*comm_simplex, &rank_simplex);

    int primary = current_primary(simplex);

    if (primary == rank_simplex)
    {
        msg_preprepare.push_back(g_pbft_view_no); // v
        msg_preprepare.push_back(g_pbft_time_no); // t
        msg_preprepare.push_back(g_pbft_request); // r
        msg_preprepare.push_back(0);              // c_0
    }

    // int tmp = 777;
    // MPI_Bcast(&tmp, 1, MPI_INT, 0, *comm_simplex);

    // IMPORTANT: Don't use vectors in MPI; we need to convert vectors into arrays
    int array_preprepare[PBFT_MSG_LEN_MAX];
    std::copy(msg_preprepare.begin(), msg_preprepare.end(), array_preprepare);
    MPI_Bcast(&array_preprepare, PBFT_MSG_LEN_MAX, MPI_INT, primary, *comm_simplex);

    // Convert the array back to vector
    if (primary != rank_simplex)
    {
        msg_preprepare.insert(msg_preprepare.begin(),
                              std::begin(array_preprepare), std::end(array_preprepare));
    }

    return 0;
}
