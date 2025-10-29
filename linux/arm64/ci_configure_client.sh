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

set -e

exec_prefix=""

while [ $# -gt 0 ]; do
    key="$1"
    case $key in
        --exec_prefix)
        exec_prefix="--exec_prefix=$2"
        shift
        ;;
        *)
        echo "unrecognized option $key"
        ;;
    esac
    shift # past argument or value
done

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

TRIPLET="arm64-linux-release"

BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_ROOT="$BUILD_DIR/vcpkg"
export VCPKG_DIR="$VCPKG_ROOT/installed/$TRIPLET"

linux/update_vcpkg_client.sh $TRIPLET

export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export LD=aarch64-linux-gnu-ld
export CFLAGS="-march=armv8-a -O3"
export CXXFLAGS="-march=armv8-a -O3 -std=c++11"
export CPPFLAGS="-I$VCPKG_DIR/include"
export LDFLAGS="-march=armv8-a -static-libstdc++ -s"
export _libcurl_pc="$VCPKG_DIR/lib/pkgconfig/libcurl.pc"
export PKG_CONFIG_PATH=$VCPKG_DIR/lib/pkgconfig/

./configure --host=aarch64-linux-gnu --with-boinc-platform="aarch64-unknown-linux-gnu" --with-boinc-alt-platform="arm-unknown-linux-gnueabihf" --with-libcurl=$VCPKG_DIR --with-ssl=$VCPKG_DIR --disable-server --enable-client --disable-manager $exec_prefix
