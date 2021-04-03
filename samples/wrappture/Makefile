# This should work on Linux.  Modify as needed for other platforms.

# Change the following to match your installation
BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip

ifndef ANDROID
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)
  BOINC_ZIP_DIR = $(BOINC_SOURCE_ZIP_DIR)
  BUILD_DIR     = $(BOINC_DIR)/3rdParty/linux
  VCPKG_DIR     = $(BUILD_DIR)/vcpkg/installed/x64-linux
  LINUX_LDFLAGS = libstdc++.a -lpthread
  LINUX_STDLIB  = libstdc++.a
else
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib
  BOINC_ZIP_DIR = $(TCINCLUDES)/lib
  LINUX_LDFLAGS = 
  LINUX_STDLIB  = 
endif

RAPPTURE_DIR ?= $(VCPKG_DIR)/include/rappture

CXXFLAGS += -g \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(VCPKG_DIR)/include \
    -I$(RAPPTURE_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L$(VCPKG_DIR)/lib \
    -L.

PROGS = wrappture_example fermi

all: $(PROGS)

libstdc++.a:
	ln -s `$(CXX) -print-file-name=libstdc++.a`

clean: distclean

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

wrappture.o: wrappture.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o wrappture.o wrappture.cpp

wrappture_example: wrappture.o wrappture_example.o $(LINUX_STDLIB) $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_api.a $(VCPKG_DIR)/lib/librappture.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o wrappture_example wrappture_example.o wrappture.o $(LINUX_LDFLAGS) -lboinc_api -lboinc -lrappture -lexpat -lz

fermi: fermi.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o fermi fermi.cpp