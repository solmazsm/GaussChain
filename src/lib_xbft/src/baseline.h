/*
 * Created on Wed Feb 23 2022
 *
 * baseline.h
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

#ifndef BASELINE_H
#define BASELINE_H

#include "protocol.h"
#include "util.h"
#include "globals.h"

using namespace std;

/**
 * @brief This is the new x-blockchain protocol by dzhao@uw.edu
 * 
 * @param simplices 
 * @return int 
 */
int baseline_topocommit(vector<string> simplices);

/**
 * @brief The baseline protocol by Zackary's VLDB'20 paper
 *
 * @param simplices
 * @return int
 */
int baseline_witnet(vector<string> simplices);

/**
 * @brief A two-party transaction between the two simplices, each of which calls a PBFT
 *
 * @param first_simplex
 * @param second_simplex
 */
void txn_neighbor(string first_simplex, string second_simplex);

/**
 * @brief The baseline from Maurice Herlihy
 *
 * @param simplices
 * @return int
 */
int baseline_pairswap(vector<string> simplices);

#endif
