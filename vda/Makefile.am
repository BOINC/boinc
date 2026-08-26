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

vdadir=$(libexecdir)/boinc-server-maker/vda
vda_PROGRAMS = vda vdad ssim

AM_CXXFLAGS += $(MYSQL_CFLAGS)

vda_SOURCES = vda.cpp vda_lib.cpp vda_lib2.cpp vda_policy.cpp stats.cpp
vda_LDADD = $(SERVERLIBS)

vdad_SOURCES = vdad.cpp vda_lib.cpp vda_lib2.cpp vda_policy.cpp stats.cpp
vdad_LDADD = $(SERVERLIBS)

ssim_SOURCES = ssim.cpp vda_lib.cpp vda_policy.cpp stats.cpp des.h
ssim_LDADD = $(SERVERLIBS)

clean: clean-am
	rm -f *.gcno
