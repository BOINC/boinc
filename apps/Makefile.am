## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = upper_case concat 1sec
# bin_PROGRAMS = upper_case upper_case_x11 concat 1sec

noinst_LIBRARIES = libapp.a

libapp_a_SOURCES = ../api/mfile.C ../lib/parse.C ../lib/filesys.C ../lib/shmem.C ../lib/util.C ../lib/app_ipc.C

# when we are ready to build graphics apps on unix:
# LDADD = -lm -lGL -lGLU -lpthread -L. -lapp

LDADD = -lm -lpthread -L. -lapp

# upper_case_x11_SOURCES = upper_case_x11.C ../api/x_opengl.C ../api/boinc_api.C ../api/graphics_api.C
# upper_case_x11_CPPFLAGS = -DBOINC_APP_GRAPHICS

upper_case_SOURCES       = upper_case.C ../api/boinc_api.C
concat_SOURCES           = concat.C ../api/boinc_api.C
1sec_SOURCES             = 1sec.C ../api/boinc_api.C

#  ../api/graphics_api.C
