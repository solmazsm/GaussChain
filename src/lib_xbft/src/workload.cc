/*
 * Created on Sun Feb 13 2022
 *
 * workload.cc
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


#include <iostream>
#include <iterator>
#include <list>

#include "workload.h"

using namespace std;

/**
 * @brief Construct a new Workload:: Workload object
 * 
 * @param n_txn 
 */
Workload::Workload(int n_txn)
{

    for (int i = 0; i < n_txn; i++)
    {
        Transaction txn;
        this->txns.push_back(txn);
    }

    // if (g_debug == 1) 
    // {
    //     cout << "Workload is being created" << endl;
    // }
}
