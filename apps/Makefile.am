## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = upper_case concat 1sec

LDADD = $(top_builddir)/api/libboinc_api.a $(LIBBOINC) $(PTHREAD_LIBS)
AM_CXXFLAGS = $(MYSQL_CFLAGS)

upper_case_SOURCES       = upper_case.cpp
concat_SOURCES           = concat.cpp
1sec_SOURCES             = 1sec.cpp
