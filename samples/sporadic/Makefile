
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

  MAKEFILE_LDFLAGS = -lpthread libstdc++.a
  MAKEFILE_STDLIB  = libstdc++.a
endif

CXXFLAGS += -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(BOINC_SOURCE_ZIP_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L.

ifdef BUILD_APPS_WITH_VCPKG
  BUILD_DIR     = $(BOINC_DIR)/3rdParty/linux
  VCPKG_DIR     ?= $(BUILD_DIR)/vcpkg/installed/x64-linux

  CXXFLAGS += \
    -I$(VCPKG_DIR)/include \
    -L$(VCPKG_DIR)/lib
endif

all: sporadic

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f sporadic $(addsuffix .exe, sporadic) *.o libstdc++.a

install: all

sporadic: sporadic.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o sporadic sporadic.o \
	-lboinc_api -lboinc $(MAKEFILE_LDFLAGS) \
	$(STDCPPTC)
