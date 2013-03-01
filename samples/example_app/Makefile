# This should work on Linux.  Modify as needed for other platforms.

BOINC_DIR = ../..
BOINC_API_DIR = $(BOINC_DIR)/api
BOINC_LIB_DIR = $(BOINC_DIR)/lib
FREETYPE_DIR = /usr/include/freetype2

CXXFLAGS = -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_LIB_DIR) \
    -I$(BOINC_API_DIR) \
    -I$(FREETYPE_DIR) \
    -L /usr/X11R6/lib \
    -L.

# to get the graphics app to compile you may need to install some packages
# e.g. ftgl-devel.x86_64
#
# You may have to change the paths for your system.

ifeq ($(wildcard /usr/local/lib/libglut.a),)
	LIBGLUT = /usr/local/lib/libglut.a
	LIBGLU = /usr/local/lib/libGLU.a
	LIBJPEG = /usr/local/lib/libjpeg.a
else
# NOTE: to make your app portable you'll need the static (.a) versions
# of libglut, libGLU, and libjpeg.
# You may have to build these from source.
	#LIBGLUT = /usr/lib64/libglut.a
	#LIBGLU = /usr/lib64/libGLU.a
	#LIBJPEG = /usr/lib64/libjpeg.a
	LIBGLUT = -lglut
	LIBGLU = -lGLU
	LIBJPEG = -ljpeg
endif

PROGS = uc2 \
# uc2_graphics
# make this optional so compile doesn't break on systems without OpenGL

all: $(PROGS)

libstdc++.a:
	ln -s `g++ -print-file-name=libstdc++.a`

ttfont.cpp:
	ln -s ../../api/ttfont.cpp .

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

uc2_graphics: uc2_graphics.o ttfont.o libstdc++.a $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_graphics2.a
	$(CXX) $(CXXFLAGS) -o uc2_graphics uc2_graphics.o ttfont.o libstdc++.a -pthread \
	$(BOINC_API_DIR)/libboinc_graphics2.a \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    -lftgl -lGL -lX11 -lXmu -lm
