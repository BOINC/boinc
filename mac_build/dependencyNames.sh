#!/bin/bash

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
## This file contains the names and download URLs for the 7 third-party
## libraries needed for building BOINC for Macintosh computers.
##
## This file is usually source'ed by scripts that build the dependencies.
##
## Update this file and the Xcode project build settings when upgrading
## to a newer version of one of these libraries.
##
## ** NOTE ** the scripts buildc-ares.sh, buildcurl.sh, buildopenssl.sh
## and buildWxMac.sh contain code to patch source files for their
## respective libraries, so they may need to be modified for new versions
## of those source files.
##
## To ensure that rebuilds of older versions of BOINC always use the
## same versions of the third party libraries as the original builds,
## always hard-code the path to the correct libraries in the Xcode build
## settings; please do NOT include or reference this file in the Xcode
## build settings (except in the checkDpendentVesions.sh script.)
##

opensslDirName="openssl-3.0.0"
opensslBaseName="openssl"
opensslFileName="openssl-3.0.0.tar.gz"
opensslURL="https://www.openssl.org/source/openssl-3.0.0.tar.gz"

caresDirName="c-ares-1.28.1"
caresBaseName="c-ares"
caresFileName="c-ares-1.28.1.tar.gz"
caresURL="https://github.com/c-ares/c-ares/releases/download/cares-1_28_1/c-ares-1.28.1.tar.gz"

curlDirName="curl-8.16.0"
curlBaseName="curl"
curlFileName="curl-8.16.0.tar.gz"
curlURL="https://curl.se/download/curl-8.16.0.tar.gz"

wxWidgetsDirName="wxWidgets-3.3.1"
wxWidgetsBaseName="wxWidgets"
wxWidgetsFileName="wxWidgets-3.3.1.tar.bz2"
wxWidgetsURL="https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.1/wxWidgets-3.3.1.tar.bz2"

freetypeDirName="freetype-2.11.0"
freetypeBaseName="freetype"
freetypeFileName="freetype-2.11.0.tar.gz"
freetypeURL="https://download.savannah.gnu.org/releases/freetype/freetype-2.11.0.tar.gz"

ftglDirName="ftgl-2.1.3~rc5"
ftglBaseName="ftgl"
ftglFileName="ftgl-2.1.3-rc5.tar.gz"
ftglURL="https://sourceforge.net/projects/ftgl/files/FTGL%20Source/2.1.3%7Erc5/ftgl-2.1.3-rc5.tar.gz"

zipDirName="libzip-1.11.4"
zipBaseName="libzip"
zipFileName="libzip-1.11.4.tar.gz"
zipURL="https://libzip.org/download/libzip-1.11.4.tar.gz"

# The baseNames and dirNames arrays are used by the checkDpendentVesions.sh script
baseNames=(${opensslBaseName} ${caresBaseName} ${curlBaseName} ${wxWidgetsBaseName} ${freetypeBaseName} ${ftglBaseName} ${zipBaseName})

dirNames=(${opensslDirName} ${caresDirName} ${curlDirName} ${wxWidgetsDirName} ${freetypeDirName} ${ftglDirName} ${zipDirName})

return 0
