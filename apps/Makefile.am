## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

exampledir = $(libexecdir)/boinc-apps-examples
example_PROGRAMS = uppercase concat 1sec

uppercase_SOURCES = uppercase.cpp
uppercase_CXXFLAGS = $(PTHREAD_CFLAGS)
uppercase_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
uppercase_LDADD = $(APPLIBS)

concat_SOURCES = concat.cpp
concat_CXXFLAGS = $(PTHREAD_CFLAGS)
concat_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
concat_LDADD = $(APPLIBS)

1sec_SOURCES = 1sec.cpp
1sec_CXXFLAGS = $(PTHREAD_CFLAGS)
1sec_LDFLAGS = -static-libtool-libs $(PTHREAD_CFLAGS)
1sec_LDADD = $(APPLIBS)
