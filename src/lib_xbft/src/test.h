/*
 * Created on Sun Feb 13 2022
 *
 * test.h
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


#ifndef TEST_H
#define TEST_H

#include "globals.h"

using namespace std;

void test_pbft_fault(string, map<int, vector<int>>);

void test_simplex_class();
void test_simplex_construct();
void test_simplex_bcast();
void test_pbft();

int test_baseline_pairswap();

// Initialize the chains
int test_baseline_witnet();

int test_baseline_topocommit();

#endif
