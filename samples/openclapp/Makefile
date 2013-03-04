# makefile for atiopencl BOINC example appication on Linux.

BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib 

CXXFLAGS = -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -I ../../../ati-stream-sdk-v2.1-lnx32/include \
    -lpthread \
    -L.

PROGS = atiopencl \

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

install: atiopencl

atiopencl: atiopencl.o libstdc++.a
	$(CXX) $(CXXFLAGS) -o atiopencl atiopencl.o libstdc++.a \
        -lOpenCL -L../../../ati-stream-sdk-v2.1-lnx32/lib/x86 \
        -lboinc_api -L$(BOINC_API_DIR) \
        -lboinc -L$(BOINC_LIB_DIR)

atiopencl.o: atiopencl.cpp atiopencl.hpp
	$(CXX) $(CXXFLAGS) -c atiopencl.cpp
