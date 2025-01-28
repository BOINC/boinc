

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)

MAKEFILE_LDFLAGS = -lpthread libstdc++.a
MAKEFILE_STDLIB  = libstdc++.a

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

all: docker_wrapper

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f docker_wrapper $(addsuffix .exe, docker_wrapper) *.o libstdc++.a

install: all

docker_wrapper: docker_wrapper.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o docker_wrapper docker_wrapper.o \
	-lboinc_api -lboinc $(MAKEFILE_LDFLAGS) \
	$(STDCPPTC)
