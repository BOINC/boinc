## -*- mode: make; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

noinst_LIBRARIES = libboinc_api.a

libboinc_api_a_SOURCES =                        \
	boinc_api.C                                 \
	boinc_api.h                                 \
	reduce.h                                    \
	../lib/app_ipc.C                            \
	../lib/base64.C                             \
	../lib/diagnostics.C                        \
	../lib/exception.C                          \
	../lib/parse.C                              \
	../lib/shmem.C                              \
	../lib/synch.C                              \
	../lib/util.C

# libboinc_graphics_api_a_SOURCES =               \
# 	graphics_api.C                              \
# 	graphics_api.h                              \
# 	graphics_data.C                             \
# 	graphics_data.h                             \
# 	gutil.C                                     \
# 	gutil.h                                     \
# 	mac_app_opengl.h                            \
# 	mac_carbon_gl.h                             \
# 	x_opengl.C                                  \
# 	x_opengl.h

.PHONY: nographics
nographics:
	touch libboinc_graphics_api.a $(libboinc_graphics_api_a_OBJECTS)

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C boinc_api.C ../lib/parse.C ../lib/synch.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C

all = libboinc_api.a
