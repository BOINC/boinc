# This should work on Linux.  Modify as needed for other platforms.

ifdef ANDROID
  MULTITHREAD_RELEASE_SUFFIX =
else
  ifdef RELEASE_ARCH
    MULTITHREAD_RELEASE_SUFFIX = _$(MULTITHREAD_RELEASE_VERSION)_$(RELEASE_ARCH)-pc-linux-gnu
  else
    MULTITHREAD_RELEASE_SUFFIX = _$(MULTITHREAD_RELEASE_VERSION)_x86_64-pc-linux-gnu
  endif
endif

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
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROG = multi_thread$(MULTITHREAD_RELEASE_SUFFIX)

all: $(PROG)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROG) $(addsuffix .exe, $(PROG)) *.o libstdc++.a

$(PROG): multi_thread.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $(PROG) multi_thread.o $(MAKEFILE_LDFLAGS) -lboinc_api -lboinc $(STDCPPTC)

install: all
