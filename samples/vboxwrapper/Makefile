# This should work on Linux.  Modify as needed for other platforms.

# Change the following to match your installation
BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

CXXFLAGS = -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROGS = vboxwrapper

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

floppyio.o: floppyio.cpp
	g++ $(CXXFLAGS) -c floppyio.cpp

vbox.o: vbox.cpp
	g++ $(CXXFLAGS) -c vbox.cpp

vboxwrapper: vboxwrapper.o vbox.o floppyio.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a
	g++ $(CXXFLAGS) -o vboxwrapper vboxwrapper.o vbox.o floppyio.o libstdc++.a -pthread -lboinc_api -lboinc
