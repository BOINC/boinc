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

# This should work on Linux.  Modify as needed for other platforms.

ifdef ANDROID
  MULTITHREAD_RELEASE_SUFFIX =
else
  ifdef RELEASE_ARCH
    MULTITHREAD_RELEASE_SUFFIX = _$(MULTITHREAD_RELEASE_VERSION)_$(RELEASE_ARCH)-pc-linux-gnu
  else
    MULTITHREAD_RELEASE_SUFFIX = _$(MULTITHREAD_RELEASE_VERSION)_x86_64-pc-linux-gnu
  endif
endif

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib

ifdef RELEASE_ARCH
  CFLAGS += -O3 -flto
  CXXFLAGS += -O3 -flto
  LDFLAGS += -O3 -flto -s -static-libstdc++
else
  CFLAGS += -O0 -g
  CXXFLAGS += -O0 -g
  LDFLAGS += -O0 -g
endif

ifdef ANDROID
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib

  MAKEFILE_LDFLAGS =
  MAKEFILE_STDLIB  =
else
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)

  MAKEFILE_LDFLAGS = libstdc++.a -lpthread
  MAKEFILE_STDLIB  = libstdc++.a
endif

CXXFLAGS += \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROG = multi_thread$(MULTITHREAD_RELEASE_SUFFIX)

all: $(PROG)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROG) $(addsuffix .exe, $(PROG)) *.o libstdc++.a

$(PROG): multi_thread.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $(PROG) multi_thread.o $(MAKEFILE_LDFLAGS) -lboinc_api -lboinc $(STDCPPTC)

install: all
