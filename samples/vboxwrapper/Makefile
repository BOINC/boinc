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
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifdef ANDROID
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib

  MAKEFILE_LDFLAGS =
  MAKEFILE_STDLIB  =
else
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)

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
