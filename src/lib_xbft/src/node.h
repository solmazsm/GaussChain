/*
 * Created on Sun Feb 13 2022
 *
 * node.h
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


#ifndef NODE_H
#define NODE_H

#include "globals.h"

using namespace std;

/**
 * @brief Node status during the execution
 *      Each node keeps a list of status flags, each of which corresponds to a transaction
 *          - By default, all nodes will be normal
 */
class Node
{
    public:
        Node();
        Node(string fname);
        Node(int n_faulty);
        map<int, list<int>> status; // <node_id, <status at 1st txn, 2nd txn,...>>
};

#endif
