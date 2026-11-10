# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2026 University of California
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

clean: clean-am
	rm -f *.gcno
