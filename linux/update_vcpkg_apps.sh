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

if [ ! -d "linux" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

TRIPLET="x64-linux"
if [ ! -z "$1" ]; then
    TRIPLET="$1"
fi

echo TRIPLET=$TRIPLET

. $PWD/3rdParty/vcpkg_ports/vcpkg_link.sh
BUILD_DIR="$PWD/3rdParty/linux"
VCPKG_PORTS="$PWD/3rdParty/vcpkg_ports"
VCPKG_ROOT="$BUILD_DIR/vcpkg"

if [ ! -d $VCPKG_ROOT ]; then
    mkdir -p $BUILD_DIR
    git -C $BUILD_DIR clone $VCPKG_LINK
fi

git -C $VCPKG_ROOT pull
$VCPKG_ROOT/bootstrap-vcpkg.sh
$VCPKG_ROOT/vcpkg install --x-manifest-root=3rdParty/vcpkg_ports/configs/apps/linux/ --x-install-root=$VCPKG_ROOT/installed/ --overlay-ports=$VCPKG_PORTS/ports --overlay-triplets=$VCPKG_PORTS/triplets/ci --triplet=$TRIPLET --clean-after-build
