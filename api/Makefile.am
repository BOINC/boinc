## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if ENABLE_LIBRARIES

# library for both main and graphics apps
api_files= \
    boinc_api.cpp \
    graphics2_util.cpp \
    reduce_main.cpp

# library for graphics apps
graphics2_files = \
    gutil.cpp \
    gutil_text.cpp \
    reduce_lib.cpp \
    graphics2.cpp \
    graphics2_unix.cpp

if OS_DARWIN
    graphics2_files += mac_icon.cpp
    graphics2_files += macglutfix.m
endif

# library for OpenCL apps
opencl_files = \
	boinc_opencl.cpp

EXTRA_DIST = *.h

if BUILD_GRAPHICS_API
AM_CXXFLAGS += @GLUT_CFLAGS@
endif

lib_LTLIBRARIES = libboinc_api.la
libboinc_api_la_SOURCES = $(api_files)
libboinc_api_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)

if BUILD_GRAPHICS_API
lib_LTLIBRARIES += libboinc_graphics2.la
libboinc_graphics2_la_SOURCES = $(graphics2_files)
libboinc_graphics2_la_CPPFLAGS = -I$(top_srcdir)/lib
libboinc_graphics2_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
endif #BUILD_GRAPHICS_API

lib_LTLIBRARIES += libboinc_opencl.la
libboinc_opencl_la_SOURCES = $(opencl_files)
libboinc_opencl_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)

if INSTALL_HEADERS
## install only headers that are meant for exporting the API !!
pkginclude_HEADERS = 	\
	boinc_api.h   	\
	boinc_opencl.h   	\
	graphics2.h	\
	gutil.h
endif ## INSTALL_HEADERS

# Some OSs may not prefix libraries with lib.
# For example OS2
if OS_OS2
LIBBOINC_API_STATIC=boinc_api.${LIBEXT}
LIBBOINC_GRAPHICS2_STATIC=boinc_graphics2.${LIBEXT}
LIBBOINC_OPENCL_STATIC=boinc_opencl.${LIBEXT}
else
LIBBOINC_API_STATIC=libboinc_api.${LIBEXT}
LIBBOINC_GRAPHICS2_STATIC=libboinc_graphics2.${LIBEXT}
LIBBOINC_OPENCL_STATIC=libboinc_opencl.${LIBEXT}
endif


if BUILD_STATIC_LIBS
all_local = $(LIBBOINC_API_STATIC)
if BUILD_GRAPHICS_API
all_local += $(LIBBOINC_GRAPHICS2_STATIC)
endif
all_local += $(LIBBOINC_OPENCL_STATIC)
endif

all-local: $(all_local)

$(LIBBOINC_API_STATIC): libboinc_api.la
	rm -f $(LIBBOINC_API_STATIC)
	$(LN) .libs/$(LIBBOINC_API_STATIC) .

$(LIBBOINC_GRAPHICS2_STATIC): libboinc_graphics2.la
	rm -f $(LIBBOINC_GRAPHICS2_STATIC)
	$(LN) .libs/$(LIBBOINC_GRAPHICS2_STATIC) .

$(LIBBOINC_OPENCL_STATIC): libboinc_opencl.la
	rm -f $(LIBBOINC_OPENCL_STATIC)
	$(LN) .libs/$(LIBBOINC_OPENCL_STATIC) .

endif ## ENABLE_LIBRARIES

.PHONY:
