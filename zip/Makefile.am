## -*- mode: makefile; tab-width: 4 -*-

## note:  -D flags taken from a zip config build under Linux

include ../Makefile.incl

#SUBDIRS = zip unzip

#AM_CFLAGS += -I$(top_srcdir) -I$(top_srcdir)/zip/zip -I$(top_srcdir)/zip/unzip -I$(top_srcdir)/lib -DUNIX -DDLL -DUSE_ZIPMAIN -DNO_OFF_T -DNO_CRYPT -DNO_LCHOWN -DNO_LCHMOD -DIZ_PWLEN=80 -Dinflate=inflate_boinc -Ddeflate=deflate_boinc -Dget_crc_table=get_crc_table_boinc -Dlongest_match=longest_match_boinc -Dinflate_codes=inflate_codes_boinc -Dcrc32=crc32_boinc
#AM_CXXFLAGS += -I$(top_srcdir) -I$(top_srcdir)/zip/zip -I$(top_srcdir)/zip/unzip -I$(top_srcdir)/lib -DUNIX -DDLL -DUSE_ZIPMAIN -DNO_OFF_T -DNO_CRYPT -DNO_LCHOWN -DNO_LCHMOD -DIZ_PWLEN=80 -Dinflate=inflate_boinc -Ddeflate=deflate_boinc -Dget_crc_table=get_crc_table_boinc -Dlongest_match=longest_match_boinc -Dinflate_codes=inflate_codes_boinc -Dcrc32=crc32_boinc

#if OS_WIN32
#AM_CFLAGS += -DFORCE_WIN32_OVER_UNIX -fcommon
#AM_CXXFLAGS += -DFORCE_WIN32_OVER_UNIX -fcommon
#endif

if INSTALL_HEADERS
pkginclude_HEADERS = boinc_zip.h
endif

libboinc_zip_sources = \
boinc_zip.cpp 

lib_LTLIBRARIES = libboinc_zip.la
pkgconfig_DATA = libboinc_zip.pc
libboinc_zip_la_SOURCES = $(libboinc_zip_sources)
libboinc_zip_la_LDFLAGS = -version-number $(LIBBOINC_VERSION)
libboinc_zip_la_LIBADD =

# Some OSs may not prefix libraries with lib.
# For example OS2
if OS_OS2
LIBBOINC_ZIP_STATIC=boinc_zip.${LIBEXT}
else
LIBBOINC_ZIP_STATIC=libboinc_zip.${LIBEXT}
endif


all_local = $(LIBBOINC_ZIP_STATIC)

all-local: $(all_local)

$(LIBBOINC_ZIP_STATIC): libboinc_zip.la
	rm -f $(LIBBOINC_ZIP_STATIC)
	$(LN) .libs/$(LIBBOINC_ZIP_STATIC) .

clean:
	rm -f $(LIBBOINC_ZIP_STATIC) *.a *.o *.lo *.la
