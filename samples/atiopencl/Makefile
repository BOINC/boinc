# Tuan Le
# tuanle86@berkeley.edu
# University of California, Berkeley
# Berkeley Space Sciences Lab
# Makefile for Linux
# Date 06/22/2010

BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

CXXFLAGS = -g \
    -m32 \
    -msse2 \
    -Wfloat-equal \
    -Wpointer-arith \
    -DATI_OS_LINUX \
    -g3 \
    -ffor-scope \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L /usr/X11R6/lib \
    -L.

CXXFLAGS2 = -g \
    -m32 \
    -lpthread \
    -ldl \
    -L /usr/X11R6/lib \
    -lOpenCL \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L ../../../ati-stream-sdk-v2.1-lnx32/lib/x86 \
    -L ../../../ati-stream-sdk-v2.1-lnx32/TempSDKUtil/lib/x86 \
    -L.

# the following should be freeglut; use nm to check
# you may have to change the paths for your system

ifeq ($(wildcard /usr/local/lib/libglut.a),)
	LIBGLUT = /usr/lib/libglut.a
	LIBGLU = /usr/lib/libGLU.a
	LIBJPEG = /usr/lib/libjpeg.a
else
	LIBGLUT = /usr/local/lib/libglut.a
	LIBGLU = /usr/local/lib/libGLU.a
	LIBJPEG = /usr/local/lib/libjpeg.a
endif

PROGS = atiopencl \
# uc2_graphics
# make this optional so compile doesn't break on systems without OpenGL

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

install: atiopencl

# specify library paths explicitly (rather than -l)
# because otherwise you might get a version in /usr/lib etc.

atiopencl: atiopencl.o libstdc++.a $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS2) -o atiopencl atiopencl.o libstdc++.a -pthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

atiopencl.o: atiopencl.cpp atiopencl.hpp
	$(CXX) $(CXXFLAGS) -c atiopencl.cpp \
	-I ../../../ati-stream-sdk-v2.1-lnx32/include \
	-I ../../../ati-stream-sdk-v2.1-lnx32/samples/opencl/SDKUtil
