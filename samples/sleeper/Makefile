# This should work on Linux.  Modify as needed for other platforms.

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifndef ANDROID
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)

  LINUX_LDFLAGS = libstdc++.a -lpthread
  LINUX_STDLIB  = libstdc++.a
else
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib
  
  LINUX_LDFLAGS = 
  LINUX_STDLIB  = 
endif

CXX ?= g++
CXXFLAGS += -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROGS = sleeper

all: $(PROGS)

install: all

libstdc++.a:
	ln -s `$(CXX) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	rm -f $(PROGS) *.o libstdc++.a

sleeper: sleeper.o $(LINUX_STDLIB) $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o sleeper sleeper.o -lboinc_api -lboinc $(LINUX_LDFLAGS) \
    $(STDCPPTC)

.PHONY: all install clean distclean
