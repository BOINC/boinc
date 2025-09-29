#!/bin/sh

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

set -e

if [ ! -d "mingw" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

CACHE_DIR="$PWD/3rdParty/buildCache/mingw"
BUILD_DIR="$PWD/3rdParty/mingw"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/x64-mingw-static"

mingw/update_vcpkg.sh

export CXXFLAGS="-I$VCPKG_DIR/include -L$VCPKG_DIR/lib"
export CFLAGS="$CXXFLAGS"

export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
./configure --host=x86_64-w64-mingw32 --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --enable-apps --enable-apps-mingw --enable-apps-vcpkg --enable-apps-gui --disable-server --disable-client --disable-manager
