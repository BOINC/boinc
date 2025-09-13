# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

# This should work on Linux.  Modify as needed for other platforms.

BOINC_DIR = ../..
BOINC_SOURCE_API_DIR = $(BOINC_DIR)/api
BOINC_SOURCE_LIB_DIR = $(BOINC_DIR)/lib
FREETYPE_DIR = /usr/include/freetype2

ifdef ANDROID
  BOINC_API_DIR = $(TCINCLUDES)/lib
  BOINC_LIB_DIR = $(TCINCLUDES)/lib

  MAKEFILE_LDFLAGS =
  MAKEFILE_STDLIB  =
else
  BOINC_API_DIR = $(BOINC_SOURCE_API_DIR)
  BOINC_LIB_DIR = $(BOINC_SOURCE_LIB_DIR)

  MAKEFILE_LDFLAGS = -lpthread libstdc++.a
  MAKEFILE_STDLIB  = libstdc++.a
endif

CXXFLAGS += -g \
	-Wall -W -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common \
    -DAPP_GRAPHICS \
    -I$(BOINC_DIR) \
    -I$(BOINC_SOURCE_API_DIR) \
    -I$(BOINC_SOURCE_LIB_DIR) \
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
LIBZIP = -lzip -lz

ifdef BUILD_APPS_WITH_VCPKG
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

install: all

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
	$(BOINC_API_DIR)/libboinc_graphics2.a \
	$(BOINC_API_DIR)/libboinc_api.a \
	$(BOINC_LIB_DIR)/libboinc.a \
  $(LIBZIP) \
    $(LIBGLUT) $(LIBGLU) $(LIBJPEG) \
    $(LIBGL) $(LIBUI) -lm
