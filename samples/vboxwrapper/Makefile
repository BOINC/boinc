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
  VBOXWRAPPER_RELEASE_SUFFIX =
else
  ifdef RELEASE_ARCH
    VBOXWRAPPER_RELEASE_SUFFIX = _$(VBOXWRAPPER_RELEASE_VERSION)_$(RELEASE_ARCH)-pc-linux-gnu
  else
    VBOXWRAPPER_RELEASE_SUFFIX = _$(VBOXWRAPPER_RELEASE_VERSION)_x86_64-pc-linux-gnu
  endif
endif

# Change the following to match your installation
BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib

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

CXXFLAGS += -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROGS = vboxwrapper$(VBOXWRAPPER_RELEASE_SUFFIX)

HEADERS = vbox_common.h vboxjob.h vbox_vboxmanage.h vboxwrapper.h

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

floppyio.o: floppyio.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c floppyio.cpp

vbox_common.o: vbox_common.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vbox_common.cpp

vbox_vboxmanage.o: vbox_vboxmanage.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vbox_vboxmanage.cpp

vboxcheckpoint.o: vboxcheckpoint.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vboxcheckpoint.cpp

vboxjob.o: vboxjob.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vboxjob.cpp

vboxlogging.o: vboxlogging.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vboxlogging.cpp

vboxwrapper.o: vboxwrapper.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c vboxwrapper.cpp

vboxwrapper$(VBOXWRAPPER_RELEASE_SUFFIX): vboxwrapper.o vbox_common.o vbox_vboxmanage.o vboxcheckpoint.o vboxjob.o vboxlogging.o floppyio.o $(MAKEFILE_STDLIB) $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o vboxwrapper$(VBOXWRAPPER_RELEASE_SUFFIX) vboxwrapper.o vbox_common.o vbox_vboxmanage.o vboxcheckpoint.o vboxjob.o vboxlogging.o floppyio.o $(MAKEFILE_LDFLAGS) -lboinc_api -lboinc -lrt $(STDCPPTC)

install: all
