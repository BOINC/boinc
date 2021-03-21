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

CXXFLAGS += -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

PROGS = multi_thread

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) -print-file-name=libstdc++.a`

clean:
	/bin/rm -f $(PROGS)

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

multi_thread: multi_thread.o $(LINUX_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o multi_thread multi_thread.o $(LINUX_LDFLAGS) -lboinc_api -lboinc $(STDCPPTC)
