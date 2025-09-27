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
#
# Script to build the different targets in the BOINC xcode project using a
# combined install directory for all dependencies, plus some samples.
#
# Usage:
# ./mac_build/buildMacBOINC-CI.sh [--cache_dir PATH] [--debug] [--clean] [--no_shared_headers]
#
# --cache_dir is the path where the dependencies are installed by 3rdParty/buildMacDependencies.sh.
# --debug will build the debug Manager (needs debug wxWidgets library in cache_dir).
# --clean will force a full rebuild.
# --no_shared_headers will build targets individually instead of in one call of
#   BuildMacBOINC.sh. Provides additional verification & details in build output.

# check working directory because the script needs to be called like: ./mac_build/buildMacBOINC-CI.sh
if [ ! -d "mac_build" ]; then
    echo "start this script in the source root directory"
    exit 1
fi

# Delete any obsolete paths to old build products
rm -fR ./zip/build
rm -fR ./mac_build/build

cache_dir="$(pwd)/3rdParty/buildCache/mac"
style="Deployment"
config=""
doclean="-noclean"
beautifier="cat" # we need a fallback if xcpretty is not available
share_paths="yes"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -clean|--clean)
        doclean=""
        ;;
        --cache_dir)
        cache_dir="$2"
        shift
        ;;
        --debug|-dev)
        style="Development"
        config="-dev"
        ;;
        --no_shared_headers)
        share_paths="no"
        ;;
    esac
    shift # past argument or value
done

if [ ! -d "$cache_dir" ] || [ ! -d "$cache_dir/lib" ] || [ ! -d "$cache_dir/include" ]; then
    echo "${cache_dir} is not a directory or does not contain dependencies"
fi

XCPRETTYPATH=`xcrun -find xcpretty 2>/dev/null`
if [ $? -eq 0 ]; then
    beautifier="xcpretty"
fi

rootPath="${PWD}"

cd ./mac_build || exit 1
retval=0

show_version_errors() {
    if [ -f /tmp/depversions.txt ]; then
        cat /tmp/depversions.txt
        rm -f /tmp/depversions.txt
    fi
}

libSearchPathDbg=""
if [ "${style}" == "Development" ]; then
    libSearchPathDbg="./build/Development  ${cache_dir}/lib/debug"
fi

if [ ${share_paths} = "yes" ]; then
    ## all targets share the same header and library search paths
    ## Note: this does not build zip apps, upper case or VBoxWrapper projects.
    libSearchPathDbg=""
    source BuildMacBOINC.sh ${config} ${doclean} -all -setting HEADER_SEARCH_PATHS "../clientgui ../lib/** ../api/ ${cache_dir}/include ../samples/jpeglib ${cache_dir}/include/freetype2 \\\${HEADER_SEARCH_PATHS}" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "$libSearchPathDbg ${cache_dir}/lib ../lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_all.log | $beautifier; retval=${PIPESTATUS[0]}
    if [ $retval -ne 0 ]; then
        cd "${rootPath}"; show_version_errors; exit 1; fi
    return 0
fi

verify_product_archs() {
cd "${1}"
if [ $? -ne 0 ]; then
    cd "${rootPath}"
    show_version_errors
    exit 1
fi

    declare -a files=(*)
    for (( i = 0; i < ${#files[*]}; ++ i )); do
        if [[ -z "${files[i]}" ]]; then continue; fi
        if [[ "${files[i]}" = *dSYM ]]; then continue; fi
        if [[ "${files[i]}" = detect_rosetta_cpu ]]; then continue; fi
        fileToCheck="${files[i]}"
        if [[ -d "$fileToCheck" ]]; then
            fileToCheck="${files[i]}/Contents/MacOS/${files[i]%.*}"
        fi
        echo "Verifying architecture (x86_64 arm64) of ${fileToCheck} ..."
        lipo "${fileToCheck}" -verify_arch x86_64 arm64
        if [ $? -ne 0 ]; then
            echo "Verifying architecture (x86_64 arm64) of ${fileToCheck} failed"
            cd "${rootPath}"; show_version_errors; exit 1;
        fi
        echo "Verifying architecture (x86_64 arm64) of ${fileToCheck} ...done"
        echo
    done

cd "${rootPath}/mac_build"
if [ $? -ne 0 ]; then
    cd "${rootPath}"
    show_version_errors
    exit 1
fi
}

foundTargets=0
target="x"

rm -f /tmp/depversions.txt

## This is code that builds each target individually in the main BOINC Xcode
## project, plus the zip apps, upper case and VBoxWrapper projects.
for buildTarget in `xcodebuild -list -project boinc.xcodeproj`
do
    if [[ "${target}" = "Build" && "${buildTarget}" = "Configurations:" ]]; then break; fi
    if [ $foundTargets -eq 1 ]; then
        if [ "${target}" != "Build_All" ]; then
            echo "Building ${target}..."
            source BuildMacBOINC.sh ${config} ${doclean} -target ${target} -setting HEADER_SEARCH_PATHS "../clientgui ../lib/** ../api/ ${cache_dir}/include ../samples/jpeglib ${cache_dir}/include/freetype2 \\\${HEADER_SEARCH_PATHS}" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "${libSearchPathDbg} ${cache_dir}/lib  ../lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
            if [ ${retval} -eq 0 ]; then
                echo "Building ${target}...success"
                echo
            else
                echo "Building ${target}...failed"
                cd "${rootPath}"; show_version_errors; exit 1;
            fi
        fi
    fi
    if [ "${target}" = "Targets:" ]; then foundTargets=1; fi
    target="${buildTarget}"
done

## Now verify the architectures of the built products
verify_product_archs "${rootPath}/mac_build/build/${style}"

echo "Verifying architecture (x86_64 only) of detect_rosetta_cpu..."
if [[ `lipo "${rootPath}/mac_build/build/${style}/detect_rosetta_cpu" -archs` = "x86_64" ]]; then
    echo "Verifying architecture (x86_64 only) of detect_rosetta_cpu ...done"
else
    echo "Verifying architecture (x86_64 only) of detect_rosetta_cpu failed"
    cd ..; show_version_errors; exit 1;
fi

target="libs"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} ${doclean} -lib -setting HEADER_SEARCH_PATHS "../ ../api/ ../lib/** ../api/ ../zip/ ${cache_dir}/include ../samples/jpeglib ${cache_dir}/include/freetype2 \\\${HEADER_SEARCH_PATHS}" -setting USER_HEADER_SEARCH_PATHS "" -setting LIBRARY_SEARCH_PATHS "${libSearchPathDbg} ${cache_dir}/lib  ../lib \\\${LIBRARY_SEARCH_PATHS}"| tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd "${rootPath}"; show_version_errors; exit 1;
fi

verify_product_archs "${rootPath}/zip/build/${style}"

target="UpperCase2"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} ${doclean} -uc2 -setting HEADER_SEARCH_PATHS "../../ ../../api/ ../../lib/ ../../zip/ ../../clientgui/mac/ ../jpeglib/ ../samples/jpeglib/ ${cache_dir}/include ${cache_dir}/include/freetype2 \\\${HEADER_SEARCH_PATHS}"  -setting LIBRARY_SEARCH_PATHS "../../mac_build/build/Deployment ${cache_dir}/lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd "${rootPath}"; show_version_errors; exit 1;
fi

verify_product_archs "${rootPath}/samples/mac_build/build/${style}"

target="VBoxWrapper"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} ${doclean} -vboxwrapper -setting HEADER_SEARCH_PATHS "../../ ../../api/ ../../lib/ ../../clientgui/mac/ ../samples/jpeglib ${cache_dir}/include \\\${HEADER_SEARCH_PATHS}"  -setting LIBRARY_SEARCH_PATHS "../../mac_build/build/Deployment ${cache_dir}/lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd "${rootPath}"; show_version_errors; exit 1;
fi

verify_product_archs "${rootPath}/samples/vboxwrapper/build/${style}"

target="docker_wrapper"
echo "Building ${target}..."
source BuildMacBOINC.sh ${config} ${doclean} -docker_wrapper -setting HEADER_SEARCH_PATHS "../../ ../../api/ ../../lib/ ../../clientgui/mac/ ../samples/jpeglib ${cache_dir}/include \\\${HEADER_SEARCH_PATHS}"  -setting LIBRARY_SEARCH_PATHS "../../mac_build/build/Deployment ${cache_dir}/lib \\\${LIBRARY_SEARCH_PATHS}" | tee xcodebuild_${target}.log | $beautifier; retval=${PIPESTATUS[0]}
if [ ${retval} -ne 0 ]; then
    echo "Building ${target}...failed"
    cd "${rootPath}"; show_version_errors; exit 1;
fi

verify_product_archs "${rootPath}/samples/docker_wrapper/build/${style}"

cd "${rootPath}"
