## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

api_files= \
	boinc_api.C                                 \
	../lib/app_ipc.C                            \
	../lib/base64.C                             \
	../lib/diagnostics.C                        \
	../lib/exception.C                          \
	../lib/parse.C                              \
	../lib/shmem.C                              \
	../lib/synch.C                              \
	../lib/util.C

graphics_api_files = \
	x_opengl.C 		\
	graphics_data.C \
	graphics_api.C 	\
	reduce.C		\
	gutil.C

lib_LIBRARIES = libboinc_api.a libboinc_graphics_api.a

libboinc_api_a_SOURCES = $(api_files)

libboinc_graphics_api_a_SOURCES = $(graphics_api_files)
libboinc_graphics_api_a_CPPFLAGS = -I$(top_srcdir)/boincglut/include -I$(top_srcdir)/lib 

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/

## FIXME: currently we install ALL header-files: please remove those not needed!
include_HEADERS = \
	boinc_api.h   \
	boinc_gl.h   \
	graphics_api.h   \
	graphics_data.h   \
	gutil.h   \
	mac_app_opengl.h   \
	mac_carbon_gl.h   \
	reduce.h   \
	x_opengl.h  

.PHONY: nographics
nographics:
	touch libboinc_graphics_api.a $(libboinc_graphics_api_a_OBJECTS)

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C boinc_api.C ../lib/parse.C ../lib/synch.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C

