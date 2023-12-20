# This should work on Linux.  Modify as needed for other platforms.

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
BOINC_SOURCE_ZIP_DIR = $(BOINC_DIR)/zip
FREETYPE_DIR = /usr/include/freetype2

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
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
    -I$(BOINC_SOURCE_ZIP_DIR) \
    -I$(FREETYPE_DIR) \
    -L$(BOINC_API_DIR) \
    -L$(BOINC_LIB_DIR) \
    -L/usr/X11R6/lib \
    -L.

# to get the graphics app to compile you may need to install some packages
# e.g. ftgl-devel.x86_64
#
# You may have to change the paths for your system.

LIBGLUT = -lglut
LIBGLU = -lGLU
LIBGL= -lGL
LIBUI = -lX11 -lXmu
LIBFTGL = -lftgl
LIBJPEG = -ljpeg

ifdef BUILD_WITH_VCPKG
  BUILD_DIR     = $(BOINC_DIR)/3rdParty/linux
  VCPKG_DIR     ?= $(BUILD_DIR)/vcpkg/installed/x64-linux

  CXXFLAGS += \
    -I$(VCPKG_DIR)/include \
    -L$(VCPKG_DIR)/lib

  LIBUI = -lX11 -lXmu -lXrandr -lXxf86vm -lXi
  LIBFTGL = -lftgl -lfreetype -lpng -lbz2 -lbrotlidec -lbrotlienc -lbrotlicommon -lz
endif

ifdef BUILD_WITH_MINGW
  LIBGLUT = -lfreeglut_static
  LIBGLU = -lglu32
  LIBGL= -lopengl32
  LIBUI = -lgdi32 -lwinmm
  LIBFTGL = -lftgl -lfreetype -lpng -lbz2 -lbrotlidec -lbrotlienc -lbrotlicommon -lz
endif

PROGS = uc2 ucn
# make this optional so compile doesn't break on systems without OpenGL
GUI_PROGS = uc2_graphics slide_show

ifdef BUILD_WITH_GUI
  $(info building gui apps)
  PROGS += $(GUI_PROGS)
endif

$(info All PROGS=$(PROGS))

all: $(PROGS)

gui: $(GUI_PROGS)

libstdc++.a:
	ln -s `$(CXX) $(CXXFLAGS) -print-file-name=libstdc++.a`

ttfont.cpp:
	ln -s ../../api/ttfont.cpp .

clean: distclean

distclean:
	/bin/rm -f $(PROGS) $(GUI_PROGS) $(addsuffix .exe, $(PROGS)) $(addsuffix .exe, $(GUI_PROGS)) *.o libstdc++.a ttfont.cpp

install: uc2

# specify library paths explicitly (rather than -l)
# because otherwise you might get a version in /usr/lib etc.

uc2: uc2.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o uc2 uc2.o \
	-lboinc_api -lboinc $(MAKEFILE_LDFLAGS) \
	$(STDCPPTC)

ucn: ucn.o $(MAKEFILE_STDLIB) $(BOINC_API_DIR)/libboinc_api.a $(BOINC_LIB_DIR)/libboinc.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o ucn ucn.o \
	-lboinc_api -lboinc $(MAKEFILE_LDFLAGS) \
	$(STDCPPTC)

uc2_graphics: uc2_graphics.o ttfont.o $(MAKEFILE_STDLIB) $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_graphics2.a
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o uc2_graphics uc2_graphics.o ttfont.o $(MAKEFILE_LDFLAGS) \
	$(BOINC_API_DIR)/libboinc_graphics2.a \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    $(LIBFTGL) $(LIBGL) $(LIBUI) -lm

slide_show: slide_show.o $(MAKEFILE_STDLIB) $(BOINC_LIB_DIR)/libboinc.a $(BOINC_API_DIR)/libboinc_graphics2.a
	$(CXX) $(CXXFLAGS) -o slide_show slide_show.o $(MAKEFILE_LDFLAGS) \
	$(MINGW_ZIP_FIX) \
	$(BOINC_API_DIR)/libboinc_graphics2.a \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a \
	$(BOINC_ZIP_DIR)/libboinc_zip.a \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    $(LIBGL) $(LIBUI) -lm
