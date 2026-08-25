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

clean: clean-am
	rm -f *.gcno
