/*
 * Created on Sun Feb 13 2022
 *
 * transaction.cc
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


#include "transaction.h"

/**
 * @brief Construct a new Transaction:: Transaction object
 * 
 */
Transaction::Transaction()
{

    // Retrieve the total number of chains
    int n_chains = g_n_chain;

    for (int i = 0; i < n_chains; i++)
    {
        this->txn.insert(pair<int, int>(i, DEFAULT_OP));
    }

    // if (g_debug)
    // {
    //     cout << "Transaction is being created" << endl;
    // }
}