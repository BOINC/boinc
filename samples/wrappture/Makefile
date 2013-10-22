# This should work on Linux.  Modify as needed for other platforms.

# Change the following to match your installation
BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib
RAPPTURE_DIR = /home/boincadm/rappture/rappture/src/core

CXXFLAGS = -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(RAPPTURE_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(RAPPTURE_DIR) \
    -L.

PROGS = wrappture_example fermi

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

wrappture.o: wrappture.cpp
	g++ $(CXXFLAGS) -c -o wrappture.o wrappture.cpp

wrappture_example: wrappture.o wrappture_example.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a $(RAPPTURE_DIR)/librappture.a
	g++ $(CXXFLAGS) -o wrappture_example wrappture_example.o wrappture.o $(RAPPTURE_DIR)/librappture.a libstdc++.a -pthread -lboinc_api -lboinc -lexpat -lz

fermi: fermi.cpp
	g++ $(CSSFLAGS) -o fermi fermi.cpp
