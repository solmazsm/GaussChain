/*
 * Created on Sun Feb 13 2022
 *
 * workload.h
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


#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <list>

#include "globals.h"
#include "transaction.h"
 
using namespace std;


class Transaction;
class Workload {
    public:
        Workload(int n_txn);  // Constructor with given number of txns
        list<Transaction> txns;

    private:
        
};

#endif