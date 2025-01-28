# makefile for openclapp BOINC example appication using AMD GPU on Linux.

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifndef ANDROID
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)
else
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib
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
	ln -s `g++ -print-file-name=libstdc++.a`

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
