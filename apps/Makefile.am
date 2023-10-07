## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

exampledir = $(libexecdir)/boinc-apps-examples
example_PROGRAMS = upper_case concat 1sec

upper_case_SOURCES = upper_case.cpp
upper_case_CXXFLAGS = $(PTHREAD_CFLAGS)
upper_case_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
upper_case_LDADD = $(APPLIBS)

concat_SOURCES = concat.cpp
concat_CXXFLAGS = $(PTHREAD_CFLAGS)
concat_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
concat_LDADD = $(APPLIBS)

1sec_SOURCES = 1sec.cpp
1sec_CXXFLAGS = $(PTHREAD_CFLAGS)
1sec_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
1sec_LDADD = $(APPLIBS)
