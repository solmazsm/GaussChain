/*
 * Created on Sun Feb 13 2022
 *
 * topology.h
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


#ifndef TOPOLOGY_H
#define TOPOLOGY_H

using namespace std;

#include <mpi.h>
#include "globals.h"

class Simplex
{
    public:
        Simplex(list<int>);
        MPI_Comm comm_simplex;
};

int simplex_construct(string);
int simplex_bcast(string);

int split();

//TODO: will support more sophisticated topologies

#endif