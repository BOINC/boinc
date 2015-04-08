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

vbox_common.o: vbox_common.cpp
	g++ $(CXXFLAGS) -c vbox_common.cpp

vbox_vboxmanage.o: vbox_vboxmanage.cpp
	g++ $(CXXFLAGS) -c vbox_vboxmanage.cpp

vboxcheckpoint.o: vboxcheckpoint.cpp
	g++ $(CXXFLAGS) -c vboxcheckpoint.cpp

vboxjob.o: vboxjob.cpp
	g++ $(CXXFLAGS) -c vboxjob.cpp

vboxlogging.o: vboxlogging.cpp
	g++ $(CXXFLAGS) -c vboxlogging.cpp

vboxwrapper.o: vboxwrapper.cpp
	g++ $(CXXFLAGS) -c vboxwrapper.cpp

vboxwrapper: vboxwrapper.o vbox_common.o vbox_vboxmanage.o vboxcheckpoint.o vboxjob.o vboxlogging.o floppyio.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a
	g++ $(CXXFLAGS) -o vboxwrapper vboxwrapper.o vbox_common.o vbox_vboxmanage.o vboxcheckpoint.o vboxjob.o vboxlogging.o floppyio.o libstdc++.a -pthread -lboinc_api -lboinc
