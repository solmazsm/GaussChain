#
# Created on Sun Feb 13 2022
#
# Makefile
# Copyright (C) 2022 
# 
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#

CC = mpic++
CFLAGS = -g -I/hpdic/xbft/src -I/usr/include/x86_64-linux-gnu/mpi -pedantic \
	-Wno-deprecated-declarations -std=gnu++1z
LDFLAGS = -L/usr/lib/x86_64-linux-gnu/ -lm -lssl -lcrypto -lgmp -lmpi

# Variable names...
DEPS = globals.h
DIR_NFS = $(HOME)/xbft_target
DIR_OBJ = $(DIR_NFS)/build
DIR_BIN = $(DIR_NFS)/bin
OBJ =	$(addprefix $(DIR_OBJ)/, xbft.o topology.o workload.o transaction.o util.o node.o \
								 protocol.o test.o baseline.o)

all: directories xbft

directories:
	mkdir -p $(DIR_OBJ) $(DIR_BIN)

$(DIR_OBJ)/%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

xbft: $(OBJ) $(DEPS)
	$(CC) -o $(DIR_BIN)/$@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o a.out $(DIR_BIN) $(DIR_OBJ)
