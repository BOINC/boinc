# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifdef RELEASE_ARCH
  CFLAGS += -O3 -flto
  CXXFLAGS += -O3 -flto
  LDFLAGS += -O3 -flto -s -static-libstdc++
else
  CFLAGS += -O0 -g
  CXXFLAGS += -O0 -g
  LDFLAGS += -O0 -g
endif

BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)

MAKEFILE_LDFLAGS = -lpthread libstdc++.a
MAKEFILE_STDLIB  = libstdc++.a

CUDA_INSTALL_PATH ?= /usr/local/cuda

ifneq (,$(filter aarch64 arm64,$(RELEASE_ARCH) $(TARGET_ARCH))$(findstring aarch64,$(CXX)))
  ifneq (,$(wildcard $(CUDA_INSTALL_PATH)/targets/sbsa-linux))
    CUDA_TARGET_PATH ?= $(CUDA_INSTALL_PATH)/targets/sbsa-linux
  else ifneq (,$(wildcard $(CUDA_INSTALL_PATH)/targets/aarch64-linux))
    CUDA_TARGET_PATH ?= $(CUDA_INSTALL_PATH)/targets/aarch64-linux
  else
    CUDA_TARGET_PATH ?= $(CUDA_INSTALL_PATH)
  endif
else
  CUDA_TARGET_PATH ?= $(CUDA_INSTALL_PATH)
endif

CUDA_INCLUDE_PATH ?= $(CUDA_TARGET_PATH)/include
ifneq (,$(wildcard $(CUDA_TARGET_PATH)/lib64))
  CUDA_LIB_PATH ?= $(CUDA_TARGET_PATH)/lib64
else
  CUDA_LIB_PATH ?= $(CUDA_TARGET_PATH)/lib
endif

CXXFLAGS += \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(BOINC_SOURCE_ZIP_DIR) \
    -I$(CUDA_INCLUDE_PATH) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(CUDA_LIB_PATH) \
    -L/usr/X11R6/lib \
    -L.

EXECUTABLE	:= example_app_nvcuda

all: $(EXECUTABLE)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(EXECUTABLE) *.o libstdc++.a

install: all

GENCODE_SM80 := -gencode=arch=compute_80,code=\"sm_80,compute_80\"
GENCODE_SM90 := -gencode=arch=compute_90,code=\"sm_90,compute_90\"
GENCODE_SM100 := -gencode=arch=compute_100,code=\"sm_100,compute_100\"

CUDA_NVCC ?= $(CUDA_INSTALL_PATH)/bin/nvcc
CUDA_CCBIN ?= $(CXX)

cuda.o: cuda.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c cuda.cpp -o cuda.o

cuda_kernel.o: cuda_kernel.cu
	$(CUDA_NVCC) -ccbin "$(CUDA_CCBIN)" $(GENCODE_SM80) $(GENCODE_SM90) $(GENCODE_SM100) -I$(BOINC_SOURCE_API_DIR) -I$(BOINC_SOURCE_LIB_DIR) -I$(BOINC_SOURCE_ZIP_DIR) -I$(CUDA_INCLUDE_PATH) -L$(BOINC_API_DIR) -L$(BOINC_LIB_DIR) -L$(CUDA_LIB_PATH) -L/usr/X11R6/lib -L. -c cuda_kernel.cu -o cuda_kernel.o

$(EXECUTABLE): cuda.o cuda_kernel.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $(EXECUTABLE) cuda.o cuda_kernel.o \
	-lboinc_api -lboinc -lcudart $(MAKEFILE_LDFLAGS) \
	$(STDCPPTC)
