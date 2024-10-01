BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifdef ANDROID
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib

  MAKEFILE_LDFLAGS = -lz
  MAKEFILE_STDLIB  =
else
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)

  MAKEFILE_LDFLAGS = libstdc++.a -lpthread
  MAKEFILE_STDLIB  = libstdc++.a
endif

CXXFLAGS += \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(BOINC_SOURCE_ZIP_DIR) \
    -I$(FREETYPE_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(BOINC_ZIP_DIR)


ifdef BUILD_APPS_WITH_VCPKG
  BUILD_DIR     = $(BOINC_DIR)/3rdParty/linux
  VCPKG_DIR     ?= $(BUILD_DIR)/vcpkg/installed/x64-linux

  CXXFLAGS += \
    -I$(VCPKG_DIR)/include \
    -L$(VCPKG_DIR)/lib

  CURL_EXTRA_LDFLAGS ?= -lz -ldl
endif

PROGS = boinc_gahp

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean:
	rm -f $(PROGS) $(addsuffix .exe, $(PROGS)) libstdc++.a *.o

distclean: clean

distclean-recursive: clean

boinc_gahp: boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.h $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp $(BOINC_DIR)/svn_version.h $(BOINC_DIR)/version.h $(MAKEFILE_STDLIB)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(MINGW_FLAGS) -g -O3 \
	-o boinc_gahp boinc_gahp.cpp $(BOINC_SOURCE_LIB_DIR)/remote_submit.cpp \
	-lcurl -lssl -lcrypto -lboinc $(CURL_EXTRA_LDFLAGS) $(MAKEFILE_LDFLAGS) $(STDCPPTC)

install: all
