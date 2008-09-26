## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

AM_CFLAGS = @GLUT_CFLAGS@
AM_LDFLAGS = @GLUT_LIBS@

# stuff linked into both main app and graphics app
api_files= \
	boinc_api.cpp \
	reduce_main.cpp \
    graphics2_util.cpp

# stuff linked into graphics app
graphics2_files = \
	gutil.cpp	            \
	gutil_text.cpp	    \
	reduce_lib.cpp		\
    texfont.cpp           \
    texture.cpp           \
    txf_util.cpp          \
    graphics2.cpp \
    graphics2_unix.cpp

if OS_DARWIN
    graphics2_files += mac_icon.cpp
    graphics2_files += macglutfix.m
endif

EXTRA_DIST = *.h

if BUILD_GRAPHICS_API
graphics_libs = \
    libboinc_graphics2.a
else
graphics_libs = 
endif

lib_LIBRARIES = libboinc_api.a $(graphics_libs)

libboinc_api_a_SOURCES = $(api_files)

libboinc_graphics2_a_SOURCES = $(graphics2_files)
libboinc_graphics2_a_CPPFLAGS = -I$(top_srcdir)/lib 

## install only headers that are meant for exporting the API !!
pkginclude_HEADERS = 	\
	boinc_api.h   	\
	graphics2.h	\
	gutil.h


.PHONY:

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.cpp boinc_api.cpp ../lib/parse.cpp
api_test_SOURCES = boinc_api.cpp api_test.cpp ../lib/parse.cpp

