## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = upper_case concat 1sec
# bin_PROGRAMS = upper_case upper_case_x11 concat 1sec

# when we are ready to build graphics apps on unix:
# LDADD = -lm -lGL -lGLU -lpthread -L. -lapp

LDADD = -L../lib -L../api -lboinc -lboinc_api

# upper_case_x11_SOURCES = upper_case_x11.C ../api/x_opengl.C ../api/boinc_api.C ../api/graphics_api.C
# upper_case_x11_CPPFLAGS = -DBOINC_APP_GRAPHICS

upper_case_SOURCES       = upper_case.C
concat_SOURCES           = concat.C
1sec_SOURCES             = 1sec.C

