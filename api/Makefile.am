## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

AM_CFLAGS += @GLUT_CFLAGS@
AM_LDFLAGS += @GLUT_LIBS@

api_files= \
	boinc_api.C \
    graphics2_util.C

graphics2_files = \
	gutil.C	            \
	gutil_text.C	    \
	reduce_lib.C		\
    texfont.C           \
    texture.C           \
    txf_util.C          \
    graphics2.C \
    graphics2_unix.C

if OS_DARWIN
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

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## install only headers that are meant for exporting the API !!
include_HEADERS = 	\
	boinc_api.h   	\
	graphics2.h	\
	gutil.h


.PHONY:

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C boinc_api.C ../lib/parse.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C

