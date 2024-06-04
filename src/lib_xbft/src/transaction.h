/*
 * Created on Sun Feb 13 2022
 *
 * transaction.h
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


#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <map>
#include "globals.h"

using namespace std;
 
class Transaction {
    public:
        Transaction();  // Constructor
        map<int, int> txn; // TODO: will be updated to map <int, op>
    private:

};

#endif