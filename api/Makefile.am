## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

AM_CFLAGS += @GLUT_CFLAGS@
AM_LDFLAGS += @GLUT_LIBS@

api_files= \
	boinc_api.C

# graphics library for monolithic apps
graphics_api_files = 	\
	graphics_api.C 		\
	graphics_data.C 	\
    graphics_impl.C     \
	reduce_main.C		\
    reduce_lib.C        \
	gutil.C	            \
	x_opengl.C

# library for main-program part of split apps
graphics_lib_files =    \
    graphics_lib.C      \
    graphics_data.C     \
    reduce_main.C

# library for shared-library part of split apps
graphics_impl_files =   \
	graphics_data.C 	\
    graphics_impl.C     \
    graphics_impl_lib.C \
	gutil.C	            \
	reduce_lib.C		\
	x_opengl.C

EXTRA_DIST =            \
    boinc_api.h         \
	boinc_gl.h          \
	graphics_api.h 	    \
	graphics_data.h 	\
    graphics_impl.h     \
    graphics_lib.h      \
    gutil.h             \
	reduce.h 			\
	x_opengl.h

if BUILD_GRAPHICS_API
graphics_libs = \
    libboinc_graphics_api.a \
    libboinc_graphics_lib.a \
    libboinc_graphics_impl.a
else
graphics_libs = 
endif

lib_LIBRARIES = libboinc_api.a $(graphics_libs)

#EXTRA_LIBRARIES = libboinc_graphics_api.a

libboinc_api_a_SOURCES = $(api_files)

libboinc_graphics_api_a_SOURCES = $(graphics_api_files)
libboinc_graphics_api_a_CPPFLAGS = -I$(top_srcdir)/lib 
libboinc_graphics_lib_a_SOURCES = $(graphics_lib_files)
libboinc_graphics_lib_a_CPPFLAGS = -I$(top_srcdir)/lib 
libboinc_graphics_impl_a_SOURCES = $(graphics_impl_files)
libboinc_graphics_impl_a_CPPFLAGS = -I$(top_srcdir)/lib 

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## install only headers that are meant for exporting the API !!
include_HEADERS = 	\
	boinc_api.h   	\
	graphics_api.h	\
	gutil.h         \
	graphics_lib.h  \
    graphics_impl.h


.PHONY: nographics
nographics:
	touch libboinc_graphics_api.a $(libboinc_graphics_api_a_OBJECTS)

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C boinc_api.C ../lib/parse.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C

