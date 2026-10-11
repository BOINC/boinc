#!/bin/bash

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
#
# Script to setup Macintosh header-only nlohmann-json library for
# use in building BOINC.
#

doclean=""
stdout_target="/dev/stdout"
lprefix="/tmp/installed-nlohmann-json"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean="yes"
        ;;
        -prefix|--prefix)
        lprefix="$2"
        shift
        ;;
        -q|--quiet)
        stdout_target="/dev/null"
        ;;
    esac
    shift # past argument or value
done

MAKEPATH=`xcrun -find make`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find make tool"
    return 1
fi

TOOLSPATH1=${MAKEPATH%/make}

CMAKEPATH=`xcrun -find cmake`
if [ $? -ne 0 ]; then
    echo "ERROR: can't find cmake tool"
    return 1
fi

TOOLSPATH2=${CMAKEPATH%/cmake}

export PATH="${TOOLSPATH1}":"${TOOLSPATH2}":/usr/local/bin:$PATH

SDKPATH=`xcodebuild -version -sdk macosx Path`
result=$?

## Set deployment target to oldest MacOS version supported by this Xcode version
if [ $result -eq 0 ]; then
    targetOSVers=`plutil -extract SupportedTargets.macosx.MinimumDeploymentTarget raw -n "${SDKPATH}/SDKSettings.plist"`
    result=$?
fi
if [ $result -ne 0 ]; then
    echo "Failed to set deployment target MacOS version number"
    return $result
fi

IFS='.' read -r MAJOR MINOR PATCH <<< "$targetOSVers"
MINOR=${MINOR:-0}
PATCH=${PATCH:-0}
if [ "$MAJOR" -eq 10 ]; then
    if [ "$MINOR" -lt 10 ]; then
        # Legacy macOS 10.x < 10.10 format: 10x0
        printf -v MAC_OS_VERSION "10%1d0" "$MINOR"
    else
        # Legacy macOS 10.x format: 10xx00
        # Uses printf to pad the minor version to 2 digits
        printf -v MAC_OS_VERSION "10%02d%02d" "$MINOR" "$PATCH"
    fi
else
    # macOS 11.0+ format: xx0000
    printf -v MAC_OS_VERSION "%02d%02d%02d" "$MAJOR" "$MINOR" "$PATCH"
fi

cmake --fresh -B . -S . -DCMAKE_INSTALL_PREFIX=${lprefix} -DJSON_BuildTests=OFF
if [ $? -ne 0 ]; then return 1; fi

if [ "${doclean}" = "yes" ]; then
    make clean
fi

make 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi
make install 1>$stdout_target
if [ $? -ne 0 ]; then return 1; fi

lprefix=""

return 0
