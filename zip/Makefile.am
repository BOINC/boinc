## -*- mode: makefile; tab-width: 4 -*-

# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2025 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
#

## note:  -D flags taken from a zip config build under Linux

include ../Makefile.incl

if INSTALL_HEADERS
pkginclude_HEADERS = boinc_zip.h
endif

lib_LTLIBRARIES = libboinc_zip.la
pkgconfig_DATA = libboinc_zip.pc
libboinc_zip_la_SOURCES = boinc_zip.cpp
libboinc_zip_la_CFLAGS = $(AM_CFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS) $(LIBZIP_CFLAGS)
libboinc_zip_la_CXXFLAGS = $(AM_CXXFLAGS) $(PICFLAGS) $(PTHREAD_CFLAGS) $(LIBZIP_CFLAGS)
libboinc_zip_la_LDFLAGS = -version-number $(LIBBOINC_VERSION) $(LIBZIP_LIBS)
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
