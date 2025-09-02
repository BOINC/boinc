## -*- mode: makefile; tab-width: 4 -*-

## note:  -D flags taken from a zip config build under Linux

include ../Makefile.incl

if INSTALL_HEADERS
pkginclude_HEADERS = boinc_zip.h
endif

lib_LTLIBRARIES = libboinc_zip.la
pkgconfig_DATA = libboinc_zip.pc
libboinc_zip_la_SOURCES = boinc_zip.cpp
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

