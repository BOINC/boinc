## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

api_files= \
	boinc_api.C

graphics_api_files = 		\
	x_opengl.C 				\
	graphics_data.C 		\
	graphics_api.C 			\
	reduce.C				\
	gutil.C					\
	graphics_data.h 	\
	reduce.h 			\
	x_opengl.h 			\
	boinc_gl.h 

if BUILD_GRAPHICS_API
graphics_libs = libboinc_graphics_api.a
else
graphics_libs = 
endif

lib_LIBRARIES = libboinc_api.a $(graphics_libs)

EXTRA_LIBRARIES = libboinc_graphics_api.a

libboinc_api_a_SOURCES = $(api_files)

libboinc_graphics_api_a_SOURCES = $(graphics_api_files)
libboinc_graphics_api_a_CPPFLAGS = -I$(top_srcdir)/lib 

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## install only headers that are meant for exporting the API !!
include_HEADERS = 	\
	boinc_api.h   	\
	graphics_api.h	\
	gutil.h   


.PHONY: nographics
nographics:
	touch libboinc_graphics_api.a $(libboinc_graphics_api_a_OBJECTS)

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C boinc_api.C ../lib/parse.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C

