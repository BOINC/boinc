# This should work on Linux.  Modify as needed for other platforms.

# Do this first:
# ln -s `g++ -print-file-name=libstdc++.a`
# This creates a symbolic link to the C++ library,
# which is linked statically

BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

CXXFLAGS = -g \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L /usr/X11R6/lib \
    -L.

# the following should be freeglut; use nm to check
# you may have to change the paths for your system
LIBGLUT = /usr/local/lib/libglut.a
LIBGLU = /usr/local/lib/libGLU.a
LIBJPEG = /usr/local/lib/libjpeg.a

PROGS = uc2 uc2_graphics

all: $(PROGS)

clean:
	/bin/rm -f $(PROGS)

uc2: uc2.o $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) -o uc2 uc2.o libstdc++.a -pthread -lboinc_api -lboinc

uc2_graphics: uc2_graphics.o $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_graphics2.a
	$(CXX) $(CXXFLAGS) -o uc2_graphics uc2_graphics.o libstdc++.a -pthread \
    -lboinc_graphics2 -lboinc_api -lboinc \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    -lGL -lX11 -lXmu -lm
