## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

AM_LDFLAGS += -lpthread
if ENABLE_CLIENT_RELEASE
  AM_LDFLAGS += -static-libtool-libs
## for an entirely statically linked library, you may want to try
## -all-static instead.  There's a good chance it won't work properly,
## so we'll use the safer "-static-libtool-libs" by default.
else
if DYNAMIC_CLIENT
## if libtool starts to need flags for dynamic linking, add them here
else
  AM_LDFLAGS += -static
endif
endif ## ENABLE_CLIENT_RELEASE

bin_PROGRAMS = boincscr

boincscr_SOURCES = screensaver_x11.cpp
boincscr_DEPENDENCIES = $(LIBBOINC)
boincscr_CPPFLAGS = $(AM_CPPFLAGS) $(XCB_CFLAGS) $(XCBATOM_CFLAGS)
boincscr_LDFLAGS = $(AM_LDFLAGS) -L../lib $(XCB_LIBS) $(XCBATOM_LIBS)
boincscr_LDADD = $(LIBBOINC) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS)
