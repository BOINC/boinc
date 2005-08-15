## $Id$

## note:  -D flags taken from a zip config build under Linux

SUBDIRS = zip  unzip

AM_CPPFLAGS =  -I$(top_srcdir)/zlib -I$(top_srcdir)/lib -I$(top_srcdir)/zip -I$(top_srcdir)/zip/zip -I$(top_srcdir)/zip/unzip -DUNIX -DUSE_ZLIB

##noinst_PROGRAMS = test
lib_LIBRARIES = libboinc_zip.a

## install header-files with prefix-subdir BOINC/ to avoid name-conflicts
includedir = ${prefix}/include/BOINC/
include_HEADERS = boinc_zip.h

libboinc_zip_a_SOURCES = boinc_zip.cpp \
./unzip/apihelp.c \
./unzip/crc32.c \
./unzip/crctab.c \
./unzip/envargs.c \
./unzip/explode.c \
./unzip/extract.c \
./unzip/fileio.c \
./unzip/globals.c \
./unzip/inflate.c \
./unzip/list.c \
./unzip/match.c \
./unzip/process.c \
./unzip/ttyio.c \
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
./zip/unix/z_unix.c \
./unzip/unix/unix.c 

##test_SOURCES = test.c
##test_LDADD = boinc_zip
##g++ -o test -DHAVE_DIRENT_H -I./ test.c ../lib/filesys.C ../lib/util.C -L./ -lboinc_zip
