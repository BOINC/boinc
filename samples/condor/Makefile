BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifndef ANDROID
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)
  
  LINUX_LDFLAGS  = -lpthread
  BOINC_LIB_DIR  = $(BOINC_SOURCE_LIB_DIR)
else
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib

  LINUX_LDFLAGS  = -lz
endif

PROGS = boinc_gahp

all: $(PROGS)

clean:
	rm -f $(PROGS)

distclean: clean

distclean-recursive: clean

boinc_gahp: boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.h $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp $(BOINC_DIR)/svn_version.h $(BOINC_DIR)/version.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -g -O2 -I$(BOINC_DIR) -I$(BOINC_SOURCE_LIB_DIR) \
	-I$(VCPKG_DIR)/include -o boinc_gahp boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp \
	-L$(BOINC_LIB_DIR) -L$(VCPKG_DIR)/lib -lcurl -lssl -lcrypto -lboinc $(LINUX_LDFLAGS) $(STDCPPTC)
