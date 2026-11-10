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

  MAKEFILE_LDFLAGS = -lz
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
    -I$(FREETYPE_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \


ifdef BUILD_APPS_WITH_VCPKG
  BUILD_DIR     = $(BOINC_DIR)/3rdParty/linux
  VCPKG_DIR     ?= $(BUILD_DIR)/vcpkg/installed/x64-linux

  CXXFLAGS += \
    -I$(VCPKG_DIR)/include \
    -L$(VCPKG_DIR)/lib

  CURL_EXTRA_LDFLAGS ?= -lz -ldl
endif

ifdef BUILD_WITH_MINGW
  CURL_EXTRA_LDFLAGS += -liphlpapi
endif

PROGS = boinc_gahp

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) libstdc++.a *.o

distclean: clean

distclean-recursive: clean

boinc_gahp: boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.h $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp $(BOINC_DIR)/svn_version.h $(BOINC_DIR)/version.h $(MAKEFILE_STDLIB)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(MINGW_FLAGS) \
	-o boinc_gahp boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp \
	-lcurl -lssl -lcrypto -lboinc $(CURL_EXTRA_LDFLAGS) $(MAKEFILE_LDFLAGS) $(STDCPPTC)

install: all
