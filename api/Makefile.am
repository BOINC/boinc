## $Id$

include $(top_srcdir)/Makefile.incl

noinst_LIBRARIES = libboinc_api.a libboinc_graphics_api.a

libboinc_api_a_SOURCES =			\
	boinc_api.C				\
	boinc_api.h				\
	mac_app_opengl.h			\
	mac_carbon_gl.h				\
	mfile.C					\
	reduce.h				\
	x_opengl.C				\
	x_opengl.h				\
	../lib/util.C				\
	../lib/shmem.C				\
	../lib/parse.C				\
	../lib/app_ipc.C

libboinc_graphics_api_a_SOURCES = 		\
	graphics_api.C				\
	graphics_api.h				\
	graphics_data.C				\
	graphics_data.h

# gutil.C					
# gutil.h					

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.C mfile.C boinc_api.C ../lib/parse.C
api_test_SOURCES = boinc_api.C api_test.C ../lib/parse.C
