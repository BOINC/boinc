## -*- mode: makefile; tab-width: 4 -*-
## $Id$

include $(top_srcdir)/Makefile.incl

if ENABLE_LIBRARIES

# stuff linked into both main app and graphics app
api_files= \
    boinc_api.cpp \
    reduce_main.cpp \
    graphics2_util.cpp

# stuff linked into graphics app
graphics2_files = \
    gutil.cpp \
    gutil_text.cpp \
    reduce_lib.cpp \
    texfont.cpp \
    texture.cpp \
    txf_util.cpp \
    graphics2.cpp \
    graphics2_unix.cpp

if OS_DARWIN
    graphics2_files += mac_icon.cpp
    graphics2_files += macglutfix.m
endif

EXTRA_DIST = *.h

if BUILD_GRAPHICS_API
AM_CFLAGS += @GLUT_CFLAGS@
endif

lib_LTLIBRARIES = libboinc_api.la
libboinc_api_la_SOURCES = $(api_files)
libboinc_api_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
if BUILD_GRAPHICS_API
lib_LTLIBRARIES += libboinc_graphics2.la
libboinc_graphics2_la_SOURCES = $(graphics2_files)
libboinc_graphics2_la_CPPFLAGS = -I$(top_srcdir)/lib 
libboinc_graphics2_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
endif #BUILD_GRAPHICS_API

if INSTALL_HEADERS
## install only headers that are meant for exporting the API !!
pkginclude_HEADERS = 	\
	boinc_api.h   	\
	graphics2.h	\
	gutil.h
endif ## INSTALL_HEADERS

if BUILD_STATIC_LIBS
all_local = libboinc_api.a
if BUILD_GRAPHICS_API
all_local += libboinc_graphics2.a
endif
endif

all-local: $(all_local)

libboinc_api.a: libboinc_api.la
	rm -f libboinc_api.a
	ln .libs/libboinc_api.a

libboinc_graphics2.a: libboinc_graphics2.la
	rm -f libboinc_graphics2.a
	ln .libs/libboinc_graphics2.a


endif ## ENABLE_LIBRARIES

.PHONY:

EXTRA_PROGRAMS = api_app api_test

api_app_SOURCES = api_app.cpp ../lib/parse.cpp
api_app_LDADD = $(LIBAPI)
api_test_SOURCES = api_test.cpp ../lib/parse.cpp
api_test_LDADD = $(LIBAPI)

