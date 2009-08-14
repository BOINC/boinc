# This should work on Linux.  Modify as needed for other platforms.

BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib

CXXFLAGS = -g \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -L /usr/X11R6/lib \
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

PROGS = uc2 \
# uc2_graphics
# make this optional so compile doesn't break on systems without OpenGL

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

clean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

distclean:
	/bin/rm -f $(PROGS) *.o libstdc++.a

install: uc2

# specify library paths explicitly (rather than -l)
# because otherwise you might get a version in /usr/lib etc.

uc2: uc2.o libstdc++.a $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) -o uc2 uc2.o libstdc++.a -pthread \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a

uc2_graphics: uc2_graphics.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_graphics2.a
	$(CXX) $(CXXFLAGS) -o uc2_graphics uc2_graphics.o libstdc++.a -pthread \
	$(BOINC_API_DIR)/libboinc_graphics2.a \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    -lGL -lX11 -lXmu -lm
