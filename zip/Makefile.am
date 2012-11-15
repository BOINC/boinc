## -*- mode: makefile; tab-width: 4 -*-

## note:  -D flags taken from a zip config build under Linux

include ../Makefile.incl

SUBDIRS = zip  unzip

AM_CPPFLAGS += -I$(top_srcdir)/zip -I$(top_srcdir) -I$(top_srcdir)/zip/zip -I$(top_srcdir)/zip/unzip -I$(top_srcdir)/lib -DUNIX -DDLL -DUSE_ZIPMAIN -DNO_OFF_T -DNO_CRYPT -DIZ_PWLEN=80

if INSTALL_HEADERS
pkginclude_HEADERS = boinc_zip.h
endif

libboinc_zip_sources = \
boinc_zip.cpp \
./unzip/api.c	\
./unzip/apihelp.c \
./unzip/crc32.c \
./unzip/explode.c \
./unzip/extract.c \
./unzip/fileio.c \
./unzip/globals.c \
./unzip/inflate.c \
./unzip/list.c \
./unzip/match.c \
./unzip/process.c \
./unzip/ttyio.c \
./unzip/unix/unix.c 
./unzip/unreduce.c \
./unzip/unshrink.c \
./unzip/unzip.c \
./unzip/zipinfo.c \
./zip/deflate.c \
./zip/trees.c \
./zip/util.c \
./zip/z_fileio.c \
./zip/z_globals.c \
./zip/zip.c \
./zip/zipfile.c \
./zip/zipup.c \
./zip/unix/unix.c \

##noinst_PROGRAMS = test
if ENABLE_LIBRARIES

lib_LTLIBRARIES = libboinc_zip.la
libboinc_zip_la_SOURCES = $(libboinc_zip_sources)
libboinc_zip_la_LDFLAGS = -L$(libdir) -rpath $(libdir) -version-number $(LIBBOINC_VERSION)
libboinc_zip_la_LIBADD =

# Some OSs may not prefix libraries with lib. 
# For example OS2
if OS_OS2
LIBBOINC_ZIP_STATIC=boinc_zip.${LIBEXT}
else
LIBBOINC_ZIP_STATIC=libboinc_zip.${LIBEXT}
endif


if BUILD_STATIC_LIBS
all_local = $(LIBBOINC_ZIP_STATIC)

endif

all-local: $(all_local)

$(LIBBOINC_ZIP_STATIC): libboinc_zip.la
	rm -f $(LIBBOINC_ZIP_STATIC)
	$(LN) .libs/$(LIBBOINC_ZIP_STATIC) .

endif
