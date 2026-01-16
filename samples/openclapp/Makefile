# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

# makefile for openclapp BOINC example appication using AMD GPU on Linux.

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib

ifndef ANDROID
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
else
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
endif

CXXFLAGS += -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(AMDAPPSDKROOT)/include \
    -I/usr/local/cuda/include \
    -L. \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(AMDAPPSDKROOT)/lib/x86 \
    -L/usr/local/cuda/lib \
    -L/lib/i386-linux-gnu

## The above 2 lines work around an Ubuntu LD_LIBRARY_PATH bug described in:
## <https://help.ubuntu.com/community/EnvironmentVariables#File-location_related_variables>

PROGS = openclapp

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) -print-file-name=libstdc++.a`

clean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

install: all

openclapp: openclapp.o boinc_opencl.o libstdc++.a
	$(CXX) $(CXXFLAGS) -o openclapp openclapp.o boinc_opencl.o \
        libstdc++.a -lOpenCL -lboinc_api -lboinc -lpthread

openclapp.o: openclapp.cpp openclapp.hpp
	$(CXX) $(CXXFLAGS) -c openclapp.cpp

boinc_opencl.o: $(BOINC_API_DIR)/boinc_opencl.cpp $(BOINC_API_DIR)/boinc_opencl.h
	$(CXX) $(CXXFLAGS) -c $(BOINC_API_DIR)/boinc_opencl.cpp
